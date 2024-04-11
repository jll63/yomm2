#ifndef YOREL_YOMM2_CORE_INCLUDED
#define YOREL_YOMM2_CORE_INCLUDED

#include <functional>
#include <memory>

#include <yorel/yomm2/policy.hpp>

#pragma push_macro("max")
#undef max

namespace yorel {
namespace yomm2 {

#ifdef YOMM2_DEFAULT_POLICY
using default_policy = YOMM2_DEFAULT_POLICY;
#else
#if defined(YOMM2_SHARED)
#ifdef NDEBUG
using default_policy = policy::release_shared;
#else
using default_policy = policy::debug_shared;
#endif
#else
using default_policy = policy::default_static;
#endif
#endif

namespace detail {

template<typename... Classes>
using get_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<types<Classes...>>>,
    boost::mp11::mp_back<types<Classes...>>, default_policy>;

template<typename... Classes>
using remove_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<types<Classes...>>>,
    boost::mp11::mp_pop_back<types<Classes...>>, types<Classes...>>;

template<class... Ts>
using virtual_ptr_policy = std::conditional_t<
    sizeof...(Ts) == 2, boost::mp11::mp_first<detail::types<Ts...>>,
    default_policy>;
} // namespace detail

// -----------------------------------------------------------------------------
// Method

template<typename Key, typename Signature, class Policy = default_policy>
struct method;

template<typename Key, typename R, class Policy, typename... A>
struct method<Key, R(A...), Policy> : detail::method_info {
    using self_type = method;
    using policy_type = Policy;
    using declared_argument_types = detail::types<A...>;
    using call_argument_types = boost::mp11::mp_transform<
        detail::remove_virtual, declared_argument_types>;
    using virtual_argument_types =
        typename detail::polymorphic_types<declared_argument_types>;
    using signature_type = R(A...);
    using return_type = R;
    using function_pointer_type = R (*)(detail::remove_virtual<A>...);
    using next_type = function_pointer_type;

    static constexpr auto arity = boost::mp11::mp_count_if<
        declared_argument_types, detail::is_virtual>::value;
    static_assert(arity > 0, "method must have at least one virtual argument");

    size_t slots_strides[arity == 1 ? 1 : 2 * arity - 1];
    // For 1-method: the offset of the method in the method table, which
    // contains a pointer to a function.
    // For multi-methods: the offset of the first virtual argument in the
    // method table, which contains a pointer to the corresponding cell in
    // the dispatch table, followed by the offset of the second argument and
    // the stride in the second dimension, etc.

    static method fn;
    static function_pointer_type fake_definition;

    method();

    method(const method&) = delete;
    method(method&&) = delete;

    ~method();

    template<typename ArgType>
    const std::uintptr_t* vptr(const ArgType& arg) const;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    std::uintptr_t
    resolve_uni(const ArgType& arg, const MoreArgTypes&... more_args) const;

    template<
        size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    std::uintptr_t resolve_multi_first(
        const ArgType& arg, const MoreArgTypes&... more_args) const;

    template<
        size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    std::uintptr_t resolve_multi_next(
        const std::uintptr_t* dispatch, const ArgType& arg,
        const MoreArgTypes&... more_args) const;

    template<typename... ArgType>
    function_pointer_type resolve(const ArgType&... args) const;

    return_type operator()(detail::remove_virtual<A>... args) const;

    static return_type
    not_implemented_handler(detail::remove_virtual<A>... args);
    static return_type ambiguous_handler(detail::remove_virtual<A>... args);

    template<typename Container>
    using next = detail::next_aux<method, Container>;

    template<auto Function>
    struct add_function {
        explicit add_function(next_type* next = nullptr) {

            static detail::definition_info info;

            if (info.method) {
                assert(info.method == &fn);
                return;
            }

            info.method = &fn;
            info.type = Policy::template static_type<decltype(Function)>();
            info.next = reinterpret_cast<void**>(next);
            using parameter_types =
                detail::parameter_type_list_t<decltype(Function)>;
            info.pf = (void*)detail::wrapper<
                Policy, signature_type, Function, parameter_types>::fn;
            using spec_type_ids = detail::type_id_list<
                Policy,
                detail::spec_polymorphic_types<
                    Policy, declared_argument_types, parameter_types>>;
            info.vp_begin = spec_type_ids::begin;
            info.vp_end = spec_type_ids::end;
            fn.specs.push_front(info);
        }
    };

    template<auto... Function>
    struct add_functions : std::tuple<add_function<Function>...> {};

    template<typename Container, bool has_next>
    struct add_definition_;

    template<typename Container>
    struct add_definition_<Container, false> {
        add_function<Container::fn> override_{nullptr};
    };

    template<typename Container>
    struct add_definition_<Container, true> {
        add_function<Container::fn> add{&Container::next};
    };

    template<typename Container>
    struct add_definition
        : add_definition_<Container, detail::has_next_v<Container, next_type>> {
        using type = add_definition; // make it a meta-function
    };

    template<auto F>
    struct add_member_function
        : add_function<detail::member_function_wrapper<F, decltype(F)>::fn> {};

    template<auto... F>
    struct add_member_functions {
        std::tuple<add_member_function<F>...> add;
    };

    template<typename Container>
    struct use_next {
        static next_type next;
    };
};

template<typename Key, typename R, class Policy, typename... A>
method<Key, R(A...), Policy> method<Key, R(A...), Policy>::fn;

template<typename Key, typename R, class Policy, typename... A>
typename method<Key, R(A...), Policy>::function_pointer_type
    method<Key, R(A...), Policy>::fake_definition;

template<typename Key, typename R, class Policy, typename... A>
template<typename Container>
typename method<Key, R(A...), Policy>::next_type
    method<Key, R(A...), Policy>::use_next<Container>::next;

// -----------------------------------------------------------------------------
// class_declaration

template<class... Classes>
struct class_declaration
    : detail::class_declaration_aux<
          detail::get_policy<Classes...>, detail::remove_policy<Classes...>> {};

template<class... Classes>
struct class_declaration<detail::types<Classes...>>
    : detail::class_declaration_aux<
          detail::get_policy<Classes...>, detail::remove_policy<Classes...>> {};

template<typename First, class... Rest>
using use_classes = typename detail::use_classes_aux<
    detail::get_policy<First, Rest...>,
    detail::remove_policy<First, Rest...>>::type;

// -----------------------------------------------------------------------------
// virtual_ptr

template<class Class, class Policy = default_policy>
class virtual_ptr {
    template<class, class>
    friend class virtual_ptr;

    template<class, typename>
    friend struct detail::virtual_traits;
    template<class, typename>
    friend struct detail::virtual_ptr_traits;

  protected:
    constexpr static bool IsSmartPtr =
        detail::virtual_ptr_traits<Class, Policy>::is_smart_ptr;
    using Box = std::conditional_t<IsSmartPtr, Class, Class*>;
    static constexpr bool is_indirect =
        Policy::template has_facet<policy::indirect_vptr>;

    using vptr_type = std::conditional_t<
        is_indirect, std::uintptr_t const* const*, std::uintptr_t const*>;

    Box obj;
    vptr_type vptr;

    template<typename Other>
    void box(Other&& value) {
        if constexpr (IsSmartPtr) {
            if constexpr (std::is_rvalue_reference_v<Other>) {
                obj = std::move(value);
            } else {
                obj = value;
            }
        } else {
            static_assert(std::is_lvalue_reference_v<Other>);
            obj = &value;
        }
    }

    auto& unbox() const {
        if constexpr (IsSmartPtr) {
            return obj;
        } else {
            return *obj;
        }
    }

  public:
    using element_type = Class;
    using box_type = Box;

    template<class Other>
    virtual_ptr(Other&& other) {
        box(other);

        using namespace policy;
        using namespace detail;

        static_assert(
            std::is_polymorphic_v<polymorphic_type<
                Policy, const std::remove_reference_t<Other>&>>,
            "use 'final' if intended");

        auto dynamic_id =
            Policy::dynamic_type(virtual_traits<Policy, Other&>::rarg(other));
        auto static_id = Policy::template static_type<
            typename virtual_traits<Policy, Other&>::polymorphic_type>();

        if (dynamic_id == static_id) {
            if constexpr (has_facet<Policy, indirect_vptr>) {
                vptr = &Policy::template static_vptr<
                    typename detail::virtual_traits<
                        Policy, Other&>::polymorphic_type>;
            } else {
                vptr = Policy::template static_vptr<
                    typename detail::virtual_traits<
                        Policy, Other&>::polymorphic_type>;
            }
        } else {
            auto index = dynamic_id;

            if constexpr (has_facet<Policy, type_hash>) {
                index = Policy::hash_type_id(index);
            }

            if constexpr (has_facet<Policy, indirect_vptr>) {
                vptr = Policy::indirect_vptrs[index];
            } else {
                vptr = Policy::vptrs[index];
            }
        }
    }

    template<class Other>
    virtual_ptr(virtual_ptr<Other, Policy>& other)
        : obj(other.obj), vptr(other.vptr) {
    }

    template<class Other>
    virtual_ptr(const virtual_ptr<Other, Policy>& other)
        : obj(other.obj), vptr(other.vptr) {
    }

    template<class Other>
    virtual_ptr(virtual_ptr<Other, Policy>&& other)
        : obj(std::move(other.obj)), vptr(other.vptr) {
    }

    auto get() const noexcept {
        return obj;
    }

    auto operator->() const noexcept {
        return get();
    }

    decltype(auto) operator*() const noexcept {
        return *get();
    }

    template<class Other>
    static auto final(Other&& obj) {
        using namespace detail;
        using namespace policy;

        using other_virtual_traits = virtual_traits<Policy, Other>;
        using polymorphic_type =
            typename other_virtual_traits::polymorphic_type;

        vptr_type vptr;

        if constexpr (has_facet<Policy, indirect_vptr>) {
            vptr = &Policy::template static_vptr<polymorphic_type>;
        } else {
            vptr = Policy::template static_vptr<polymorphic_type>;
        }

        if constexpr (has_facet<Policy, runtime_checks>) {
            // check that dynamic type == static type
            auto dynamic_type =
                Policy::dynamic_type(other_virtual_traits::rarg(obj));
            auto static_type = Policy::template static_type<polymorphic_type>();

            if (dynamic_type != static_type) {
                method_table_error error;
                error.type = dynamic_type;
                Policy::error(error);
                abort();
            }
        }

        virtual_ptr result;
        result.box(obj);
        result.vptr = vptr;

        return result;
    }

    template<typename Other>
    auto cast() const {
        using namespace detail;
        std::remove_cv_t<std::remove_reference_t<Other>> result;
        result.vptr = vptr;

        if constexpr (IsSmartPtr) {
            result.obj =
                virtual_ptr_traits<Class, Policy>::template cast<Other>(obj);
        } else {
            result.obj =
                &optimal_cast<Policy, typename Other::element_type&>(*obj);
        }

        return result;
    }

    // consider as private, public for tests only
    auto _vptr() const noexcept {
        if constexpr (is_indirect) {
            return *vptr;
        } else {
            return vptr;
        }
    }

  protected:
    virtual_ptr() = default;
};

template<class Class>
virtual_ptr(Class&) -> virtual_ptr<Class, default_policy>;

template<class Class, class Policy = default_policy>
using virtual_shared_ptr = virtual_ptr<std::shared_ptr<Class>, Policy>;

template<class Class, class Policy = default_policy>
inline auto make_virtual_shared() {
    return virtual_shared_ptr<Class, Policy>::final(
        std::make_shared<detail::virtual_ptr_class<Class>>());
}

template<class Policy, class Class>
inline auto final_virtual_ptr(Class& obj) {
    return virtual_ptr<Class, Policy>::final(obj);
}

template<class Class>
inline auto final_virtual_ptr(Class& obj) {
    return virtual_ptr<Class>::final(obj);
}

// -----------------------------------------------------------------------------
// definitions

template<typename Key, typename R, class Policy, typename... A>
method<Key, R(A...), Policy>::method() {
    this->name = detail::default_method_name<method>();
    this->slots_strides_p = slots_strides;
    using virtual_type_ids = detail::type_id_list<
        Policy,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<detail::polymorphic_type, Policy>,
            virtual_argument_types>>;
    this->vp_begin = virtual_type_ids::begin;
    this->vp_end = virtual_type_ids::end;
    this->not_implemented = (void*)not_implemented_handler;
    this->ambiguous = (void*)ambiguous_handler;
    Policy::catalog.methods.push_front(*this);
}

template<typename Key, typename R, class Policy, typename... A>
method<Key, R(A...), Policy>::~method() {
    Policy::catalog.methods.remove(*this);
}

template<typename Key, typename R, class Policy, typename... A>
typename method<Key, R(A...), Policy>::return_type inline method<
    Key, R(A...), Policy>::operator()(detail::remove_virtual<A>... args) const {
    using namespace detail;
    return resolve(argument_traits<Policy, A>::rarg(args)...)(
        std::forward<remove_virtual<A>>(args)...);
}

template<typename Key, typename R, class Policy, typename... A>
template<typename... ArgType>
inline typename method<Key, R(A...), Policy>::function_pointer_type
method<Key, R(A...), Policy>::resolve(const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (arity == 1) {
        pf = resolve_uni<types<A...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<0, types<A...>, ArgType...>(args...);
    }

    return reinterpret_cast<function_pointer_type>(pf);
}

template<typename Key, typename R, class Policy, typename... A>
template<typename ArgType>
inline const std::uintptr_t*
method<Key, R(A...), Policy>::vptr(const ArgType& arg) const {
    if constexpr (detail::is_virtual_ptr<ArgType>) {
        return arg._vptr();
        // No need to check the method pointer: this was done when the
        // virtual_ptr was created.
    } else {
        return Policy::dynamic_vptr(arg);
    }
}

template<typename Key, typename R, class Policy, typename... A>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
inline std::uintptr_t method<Key, R(A...), Policy>::resolve_uni(
    const ArgType& arg, const MoreArgTypes&... more_args) const {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        return vtbl[this->slots_strides[0]];
    } else {
        return resolve_uni<mp_rest<MethodArgList>>(more_args...);
    }
}

template<typename Key, typename R, class Policy, typename... A>
template<
    size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline std::uintptr_t method<Key, R(A...), Policy>::resolve_multi_first(
    const ArgType& arg, const MoreArgTypes&... more_args) const {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        auto slot = slots_strides[0];

        // The first virtual parameter is special.  Since its stride is
        // 1, there is no need to store it. Also, the method table
        // contains a pointer into the multi-dimensional dispatch table,
        // already resolved to the appropriate group.
        auto dispatch = reinterpret_cast<const std::uintptr_t*>(vtbl[slot]);
        return resolve_multi_next<1, mp_rest<MethodArgList>, MoreArgTypes...>(
            dispatch, more_args...);
    } else {
        return resolve_multi_first<mp_rest<MethodArgList>, MoreArgTypes...>(
            more_args...);
    }
}

template<typename Key, typename R, class Policy, typename... A>
template<
    size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline std::uintptr_t method<Key, R(A...), Policy>::resolve_multi_next(
    const std::uintptr_t* dispatch, const ArgType& arg,
    const MoreArgTypes&... more_args) const {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        auto slot = this->slots_strides[2 * VirtualArg - 1];
        auto stride = this->slots_strides[2 * VirtualArg];
        dispatch = dispatch + vtbl[slot] * stride;
    }

    if constexpr (VirtualArg + 1 == arity) {
        return *dispatch;
    } else {
        return resolve_multi_next<
            VirtualArg + 1, mp_rest<MethodArgList>, MoreArgTypes...>(
            dispatch, more_args...);
    }
}

template<typename Key, typename R, class Policy, typename... A>
typename method<Key, R(A...), Policy>::return_type
method<Key, R(A...), Policy>::not_implemented_handler(
    detail::remove_virtual<A>... args) {

    if constexpr (Policy::template has_facet<policy::error_handler>) {
        resolution_error error;
        error.status = resolution_error::no_definition;
        error.method_name = fn.name;
        error.arity = arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (..., (*ti_iter++ = detail::get_tip<Policy, A>(args)));
        std::copy_n(
            types, std::max(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error_type(std::move(error)));
    }

    abort(); // in case user handler "forgets" to abort
}

template<typename Key, typename R, class Policy, typename... A>
typename method<Key, R(A...), Policy>::return_type
method<Key, R(A...), Policy>::ambiguous_handler(
    detail::remove_virtual<A>... args) {
    if constexpr (Policy::template has_facet<policy::error_handler>) {
        resolution_error error;
        error.status = resolution_error::ambiguous;
        error.method_name = fn.name;
        error.arity = arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (..., (*ti_iter++ = detail::get_tip<Policy, A>(args)));
        std::copy_n(
            types, std::max(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error_type(std::move(error)));
    }

    abort(); // in case user handler "forgets" to abort
}

template<class Policy>
void update();

#ifdef YOMM2_SHARED

yOMM2_API void update();
yOMM2_API error_handler_type set_error_handler(error_handler_type handler);
yOMM2_API method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler);

#else

#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)

inline void update() {
    update<default_policy>();
}

inline error_handler_type set_error_handler(error_handler_type handler) {
    auto p = &default_policy::error;
    auto prev = default_policy::error;
    default_policy::error = handler;
    return prev;
}

inline method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler) {
    auto prev = default_policy::call_error;
    default_policy::call_error = handler;
    return prev;
}

#endif

#endif

#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)

[[deprecated("use update() instead")]] yOMM2_API inline void update_methods() {
    update();
}

#endif

} // namespace yomm2
} // namespace yorel

#include <yorel/yomm2/runtime.hpp>

#pragma pop_macro("max")

#endif
