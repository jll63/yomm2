#ifndef YOREL_YOMM2_CORE_HPP
#define YOREL_YOMM2_CORE_HPP

#include <functional>
#include <memory>

#include <boost/assert.hpp>

#include <yorel/yomm2/policy.hpp>

#ifndef YOMM2_DEFAULT_POLICY
#define YOMM2_DEFAULT_POLICY ::yorel::yomm2::default_policy
#endif

#include <yorel/yomm2/cast.hpp>
#include <yorel/yomm2/detail/types.hpp>
#include <yorel/yomm2/detail.hpp>

namespace yorel {
namespace yomm2 {

// =============================================================================
// virtual_traits

template<class Policy, typename T>
struct virtual_traits;

template<class Policy, typename T>
struct virtual_traits<Policy, virtual_<T>> : virtual_traits<Policy, T> {};

template<class Policy, typename T>
struct virtual_traits<Policy, T&> {
    using polymorphic_type = std::remove_cv_t<T>;

    static const T& rarg(const T& arg) {
        return arg;
    }

    template<typename D>
    static D& cast(T& obj) {
        return optimal_cast<Policy, D&>(obj);
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, T&&> {
    using polymorphic_type = std::remove_cv_t<T>;

    static const T& rarg(const T& arg) {
        return arg;
    }

    template<typename D>
    static D&& cast(T&& obj) {
        return optimal_cast<Policy, D&&>(obj);
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, T*> {
    using polymorphic_type = std::remove_cv_t<T>;

    static const T& rarg(const T* arg) {
        return *arg;
    }

    template<typename D>
    static D cast(T* obj) {
        return &optimal_cast<Policy, std::remove_pointer_t<D>&>(*obj);
    }
};

namespace detail {

template<class Policy, typename T>
using polymorphic_type = typename virtual_traits<Policy, T>::polymorphic_type;

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux {
    using type = void;
};

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux<Policy, virtual_<P>, Q> {
    using type = polymorphic_type<Policy, Q>;
};

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux<
    Policy, virtual_ptr<P, Policy>, virtual_ptr<Q, Policy>> {
    using type = typename virtual_traits<
        Policy, virtual_ptr<Q, Policy>>::polymorphic_type;
};

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux<
    Policy, const virtual_ptr<P, Policy>&, const virtual_ptr<Q, Policy>&> {
    using type = typename virtual_traits<
        Policy, const virtual_ptr<Q, Policy>&>::polymorphic_type;
};

template<class Policy, typename P, typename Q>
using select_spec_polymorphic_type =
    typename select_spec_polymorphic_type_aux<Policy, P, Q>::type;

template<class Policy, typename MethodArgList, typename SpecArgList>
using spec_polymorphic_types = boost::mp11::mp_remove<
    boost::mp11::mp_transform_q<
        boost::mp11::mp_bind_front<select_spec_polymorphic_type, Policy>,
        MethodArgList, SpecArgList>,
    void>;

} // namespace detail

// =============================================================================
// Method

namespace detail {

template<typename T>
struct is_virtual : std::false_type {};

template<typename T>
struct is_virtual<virtual_<T>> : std::true_type {};

template<typename... Ts>
constexpr auto arity =
    boost::mp11::mp_count_if<types<Ts...>, is_virtual>::value;

template<typename T>
struct remove_virtual_ {
    using type = T;
};

template<typename T>
struct remove_virtual_<virtual_<T>> {
    using type = T;
};

template<typename T>
using remove_virtual = typename remove_virtual_<T>::type;

template<typename>
struct parameter_type_list;

template<typename ReturnType, typename... ParameterTypes>
struct parameter_type_list<ReturnType(ParameterTypes...)> {
    using type = types<ParameterTypes...>;
};

template<typename ReturnType, typename... ParameterTypes>
struct parameter_type_list<ReturnType (*)(ParameterTypes...)> {
    using type = types<ParameterTypes...>;
};

template<typename MethodArgList>
using polymorphic_types = boost::mp11::mp_transform<
    remove_virtual, boost::mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<class Policy, typename ArgType, typename T>
inline uintptr_t get_tip(const T& arg) {
    if constexpr (is_virtual<ArgType>::value) {
        return Policy::dynamic_type(virtual_traits<Policy, ArgType>::rarg(arg));
    } else {
        return Policy::dynamic_type(arg);
    }
}

template<class Policy, typename T>
struct argument_traits {
    static const T& rarg(const T& arg) {
        return arg;
    }

    template<typename>
    static T cast(T obj) {
        return std::forward<T>(obj);
    }
};

template<class Policy, typename T>
struct argument_traits<Policy, virtual_<T>> : virtual_traits<Policy, T> {};

template<class Policy, class Class>
struct argument_traits<Policy, virtual_ptr<Class, Policy>>
    : virtual_traits<Policy, virtual_ptr<Class, Policy>> {};

template<class Policy, class Class>
struct argument_traits<Policy, const virtual_ptr<Class, Policy>&>
    : virtual_traits<Policy, const virtual_ptr<Class, Policy>&> {};

// -----------------------------------------------------------------------------
// thunk

template<class Policy, typename, auto, typename>
struct thunk;

template<
    class Policy, typename BASE_RETURN, typename... BASE_PARAM, auto SPEC,
    typename... SPEC_PARAM>
struct thunk<Policy, BASE_RETURN(BASE_PARAM...), SPEC, types<SPEC_PARAM...>> {
    static BASE_RETURN fn(remove_virtual<BASE_PARAM>... arg) {
        using base_type = boost::mp11::mp_first<types<BASE_PARAM...>>;
        using spec_type = boost::mp11::mp_first<types<SPEC_PARAM...>>;
        return SPEC(
            argument_traits<Policy, BASE_PARAM>::template cast<SPEC_PARAM>(
                remove_virtual<BASE_PARAM>(arg))...);
    }
};

void type_next(...);

template<typename Container>
auto type_next(Container t) -> decltype(t.next);

template<typename Container>
using type_next_t = decltype(type_next(std::declval<Container>()));

template<typename Container, typename Next>
constexpr bool has_next_v = std::is_same_v<type_next_t<Container>, Next>;

template<typename Method, typename Container>
struct next_aux {
    static typename Method::next_type next;
};

template<typename Method, typename Container>
typename Method::next_type next_aux<Method, Container>::next;

template<auto F, typename T>
struct member_function_thunk;

template<auto F, class ReturnType, class C, typename... Args>
struct member_function_thunk<F, ReturnType (C::*)(Args...)> {
    static ReturnType fn(C* this_, Args&&... args) {
        return (this_->*F)(args...);
    }
};

} // namespace detail

template<typename Name, typename Signature, class Policy = YOMM2_DEFAULT_POLICY>
struct method;

template<typename Name, typename ReturnType, class Policy, typename... Args>
struct method<Name, ReturnType(Args...), Policy> : detail::method_info {
    using self_type = method;
    using policy_type = Policy;
    using declared_argument_types = detail::types<Args...>;
    using call_argument_types = boost::mp11::mp_transform<
        detail::remove_virtual, declared_argument_types>;
    using virtual_argument_types =
        typename detail::polymorphic_types<declared_argument_types>;
    using signature_type = ReturnType(Args...);
    using return_type = ReturnType;
    using function_pointer_type =
        ReturnType (*)(detail::remove_virtual<Args>...);
    using next_type = function_pointer_type;

    static constexpr auto arity = detail::arity<Args...>;
    static_assert(arity > 0, "method must have at least one virtual argument");

    static std::size_t slots_strides[2 * arity - 1];
    // Slots followed by strides. No stride for first virtual argument.
    // For 1-method: the offset of the method in the method table, which
    // contains a pointer to a function.
    // For multi-methods: the offset of the first virtual argument in the
    // method table, which contains a pointer to the corresponding cell in
    // the dispatch table, followed by the offset of the second argument and
    // the stride in the second dimension, etc.

    static method fn;

    method();

    method(const method&) = delete;
    method(method&&) = delete;

    ~method();

    template<typename ArgType>
    auto vptr(const ArgType& arg) const -> const std::uintptr_t*;

    template<class Error>
    void check_static_offset(std::size_t actual, std::size_t expected) const;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    auto resolve_uni(const ArgType& arg, const MoreArgTypes&... more_args) const
        -> std::uintptr_t;

    template<
        std::size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    auto resolve_multi_first(
        const ArgType& arg, const MoreArgTypes&... more_args) const
        -> std::uintptr_t;

    template<
        std::size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    auto resolve_multi_next(
        const std::uintptr_t* dispatch, const ArgType& arg,
        const MoreArgTypes&... more_args) const -> std::uintptr_t;

    template<typename... ArgType>
    function_pointer_type resolve(const ArgType&... args) const;

    auto operator()(detail::remove_virtual<Args>... args) const -> return_type;

    static BOOST_NORETURN auto
    not_implemented_handler(detail::remove_virtual<Args>... args)
        -> return_type;
    static BOOST_NORETURN auto
    ambiguous_handler(detail::remove_virtual<Args>... args) -> return_type;

    template<typename Container>
    using next = detail::next_aux<method, Container>;

    template<auto Function>
    struct add_function {
        explicit add_function(next_type* next = nullptr) {

            static detail::definition_info info;

            if (info.method) {
                BOOST_ASSERT(info.method == &fn);
                return;
            }

            info.method = &fn;
            info.type = Policy::template static_type<decltype(Function)>();
            info.next = reinterpret_cast<void**>(next);
            using parameter_types =
                typename detail::parameter_type_list<decltype(Function)>::type;
            info.pf = (void*)detail::thunk<
                Policy, signature_type, Function, parameter_types>::fn;
            using spec_type_ids = detail::type_id_list<
                Policy,
                detail::spec_polymorphic_types<
                    Policy, declared_argument_types, parameter_types>>;
            info.vp_begin = spec_type_ids::begin;
            info.vp_end = spec_type_ids::end;
            fn.specs.push_back(info);
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
        : add_function<detail::member_function_thunk<F, decltype(F)>::fn> {};

    template<auto... F>
    struct add_member_functions {
        std::tuple<add_member_function<F>...> add;
    };

    template<typename Container>
    struct use_next {
        static next_type next;
    };
};

template<typename Name, typename ReturnType, class Policy, typename... Args>
method<Name, ReturnType(Args...), Policy>
    method<Name, ReturnType(Args...), Policy>::fn;

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<typename Container>
typename method<Name, ReturnType(Args...), Policy>::next_type
    method<Name, ReturnType(Args...), Policy>::use_next<Container>::next;

template<typename T>
constexpr bool is_method = std::is_base_of_v<detail::method_info, T>;

// -----------------------------------------------------------------------------
// class_declaration

namespace detail {

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, types<Class, Bases...>> : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Policy, Class>();
        this->first_base = type_id_list<Policy, types<Bases...>>::begin;
        this->last_base = type_id_list<Policy, types<Bases...>>::end;
        Policy::classes.push_back(*this);
        this->is_abstract = std::is_abstract_v<Class>;
        this->static_vptr = &Policy::template static_vptr<Class>;
    }

    ~class_declaration_aux() {
        Policy::classes.remove(*this);
    }
};

// Collect the base classes of a list of classes. The result is a mp11 map that
// associates each class to a list starting with the class itself, followed by
// all its bases, as per std::is_base_of. Thus the list includes the class
// itself at least twice: at the front, and down the list, as its own improper
// base. The direct and its direct and indirect proper bases are included. The
// runtime will extract the direct proper bases. See unit tests for an example.
template<typename... Cs>
using inheritance_map = types<boost::mp11::mp_push_front<
    boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_back<std::is_base_of, Cs>, types<Cs...>>,
    Cs>...>;

template<class Policy, class... Classes>
struct use_classes_aux;

template<class Policy, class... Classes>
struct use_classes_aux<Policy, types<Classes...>> {
    using type = boost::mp11::mp_apply<
        std::tuple,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<class_declaration_aux, Policy>,
            boost::mp11::mp_apply<inheritance_map, types<Classes...>>>>;
};

template<class Policy, class... Classes, class... MoreClassLists>
struct use_classes_aux<Policy, types<types<Classes...>, MoreClassLists...>>
    : use_classes_aux<
          Policy, boost::mp11::mp_append<types<Classes...>, MoreClassLists...>>

{};

template<typename... Ts>
using second_last = boost::mp11::mp_at_c<
    types<Ts...>, boost::mp11::mp_size<types<Ts...>>::value - 2>;

template<class... Classes>
using use_classes_macro = typename std::conditional_t<
    is_policy<second_last<Classes...>>,
    use_classes_aux<
        second_last<Classes...>,
        boost::mp11::mp_pop_back<boost::mp11::mp_pop_back<types<Classes...>>>>,
    use_classes_aux<
        boost::mp11::mp_back<types<Classes...>>,
        boost::mp11::mp_pop_back<types<Classes...>>>>::type;

} // namespace detail

template<class... Classes>
struct class_declaration
    : detail::class_declaration_aux<
          detail::get_policy<Classes...>, detail::remove_policy<Classes...>> {};

template<class... Classes>
struct class_declaration<detail::types<Classes...>>
    : detail::class_declaration_aux<
          detail::get_policy<Classes...>, detail::remove_policy<Classes...>> {};

template<class... Classes>
using use_classes = typename detail::use_classes_aux<
    detail::get_policy<Classes...>, detail::remove_policy<Classes...>>::type;

// -----------------------------------------------------------------------------
// virtual_ptr

namespace detail {

template<class Class, class Policy>
struct is_virtual<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual<const virtual_ptr<Class, Policy>&> : std::true_type {};

template<typename>
struct is_virtual_ptr_aux : std::false_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<const virtual_ptr<Class, Policy>&> : std::true_type {
};

} // namespace detail

template<class Class, class Policy>
struct virtual_ptr_traits {
    static bool constexpr is_smart_ptr = false;
    using polymorphic_type = Class;
};

template<class Policy, class Class>
struct virtual_traits<Policy, virtual_ptr<Class, Policy>> {
    using ptr_traits = virtual_ptr_traits<Class, Policy>;
    using polymorphic_type = typename ptr_traits::polymorphic_type;

    static const virtual_ptr<Class, Policy>&
    rarg(const virtual_ptr<Class, Policy>& ptr) {
        return ptr;
    }

    template<typename Derived>
    static decltype(auto) cast(const virtual_ptr<Class, Policy>& ptr) {
        return ptr.template cast<Derived>();
    }
};

template<class Policy, class Class>
struct virtual_traits<Policy, const virtual_ptr<Class, Policy>&>
    : virtual_traits<Policy, virtual_ptr<Class, Policy>> {};

template<typename T>
constexpr bool is_virtual_ptr = detail::is_virtual_ptr_aux<T>::value;

template<class Class, class Policy = YOMM2_DEFAULT_POLICY>
class virtual_ptr {
    template<class, class>
    friend class virtual_ptr;

    template<class, typename>
    friend struct virtual_traits;

  protected:
    constexpr static bool IsSmartPtr =
        virtual_ptr_traits<Class, Policy>::is_smart_ptr;
    using Box = std::conditional_t<IsSmartPtr, Class, Class*>;
    static constexpr bool is_indirect =
        Policy::template has_facet<policies::indirect_vptr>;

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

    auto unbox() const -> auto& {
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

        using namespace policies;
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
                    typename virtual_traits<
                        Policy, Other&>::polymorphic_type>;
            } else {
                vptr = Policy::template static_vptr<
                    typename virtual_traits<Policy, Other&>::polymorphic_type>;
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

    auto operator*() const noexcept -> decltype(auto) {
        return *get();
    }

    template<class Other>
    static auto final(Other&& obj) {
        using namespace detail;
        using namespace policies;

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
virtual_ptr(Class&) -> virtual_ptr<Class, YOMM2_DEFAULT_POLICY>;

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

template<typename Name, typename ReturnType, class Policy, typename... Args>
method<Name, ReturnType(Args...), Policy>::method() {
    this->slots_strides_ptr = slots_strides;

#ifndef BOOST_NO_RTTI
    this->name = typeid(method).name();
#else
    this->name = "method";
#endif

    using virtual_type_ids = detail::type_id_list<
        Policy,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<detail::polymorphic_type, Policy>,
            virtual_argument_types>>;
    this->vp_begin = virtual_type_ids::begin;
    this->vp_end = virtual_type_ids::end;
    this->not_implemented = (void*)not_implemented_handler;
    this->ambiguous = (void*)ambiguous_handler;
    this->method_type = Policy::template static_type<method>();
    Policy::methods.push_back(*this);
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
std::size_t
    method<Name, ReturnType(Args...), Policy>::slots_strides[2 * arity - 1];

template<typename Name, typename ReturnType, class Policy, typename... Args>
method<Name, ReturnType(Args...), Policy>::~method() {
    Policy::methods.remove(*this);
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
auto inline method<Name, ReturnType(Args...), Policy>::operator()(
    detail::remove_virtual<Args>... args) const ->
    typename method<Name, ReturnType(Args...), Policy>::return_type {
    using namespace detail;
    auto pf = resolve(argument_traits<Policy, Args>::rarg(args)...);
    return pf(std::forward<remove_virtual<Args>>(args)...);
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<typename... ArgType>
inline typename method<Name, ReturnType(Args...), Policy>::function_pointer_type
method<Name, ReturnType(Args...), Policy>::resolve(
    const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (arity == 1) {
        pf = resolve_uni<types<Args...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<0, types<Args...>, ArgType...>(args...);
    }

    return reinterpret_cast<function_pointer_type>(pf);
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<typename ArgType>
inline auto
method<Name, ReturnType(Args...), Policy>::vptr(const ArgType& arg) const
    -> const std::uintptr_t* {
    if constexpr (is_virtual_ptr<ArgType>) {
        return arg._vptr();
        // No need to check the method pointer: this was done when the
        // virtual_ptr was created.
    } else {
        return Policy::dynamic_vptr(arg);
    }
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<class Error>
inline void method<Name, ReturnType(Args...), Policy>::check_static_offset(
    std::size_t actual, std::size_t expected) const {
    using namespace detail;

    if (actual != expected) {
        if (Policy::template has_facet<policies::error_handler>) {
            Error error;
            error.method = Policy::template static_type<method>();
            error.expected = this->slots_strides[0];
            error.actual = actual;
            Policy::error(error_type(std::move(error)));

            abort();
        }
    }
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
inline auto method<Name, ReturnType(Args...), Policy>::resolve_uni(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        if constexpr (has_static_offsets<method>::value) {
            if constexpr (Policy::template has_facet<policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    static_offsets<method>::slots[0], this->slots_strides[0]);
            }
            return vtbl[static_offsets<method>::slots[0]];
        } else {
            return vtbl[this->slots_strides[0]];
        }
    } else {
        return resolve_uni<mp_rest<MethodArgList>>(more_args...);
    }
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline auto method<Name, ReturnType(Args...), Policy>::resolve_multi_first(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        std::size_t slot;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[0];
            if constexpr (Policy::template has_facet<policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    static_offsets<method>::slots[0], this->slots_strides[0]);
            }
        } else {
            slot = this->slots_strides[0];
        }

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

template<typename Name, typename ReturnType, class Policy, typename... Args>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline auto method<Name, ReturnType(Args...), Policy>::resolve_multi_next(
    const std::uintptr_t* dispatch, const ArgType& arg,
    const MoreArgTypes&... more_args) const -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        const std::uintptr_t* vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg._vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        std::size_t slot, stride;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[VirtualArg];
            stride = static_offsets<method>::strides[VirtualArg - 1];
            if constexpr (Policy::template has_facet<policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    this->slots_strides[VirtualArg], slot);
                check_static_offset<static_stride_error>(
                    this->slots_strides[2 * VirtualArg], stride);
            }
        } else {
            slot = this->slots_strides[VirtualArg];
            stride = this->slots_strides[arity + VirtualArg - 1];
        }

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

template<typename Name, typename ReturnType, class Policy, typename... Args>
BOOST_NORETURN auto
method<Name, ReturnType(Args...), Policy>::not_implemented_handler(
    detail::remove_virtual<Args>... args) ->
    typename method<Name, ReturnType(Args...), Policy>::return_type {

    if constexpr (Policy::template has_facet<policies::error_handler>) {
        resolution_error error;
        error.status = resolution_error::no_definition;
        error.method_name = fn.name;
        error.arity = arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (..., (*ti_iter++ = detail::get_tip<Policy, Args>(args)));
        std::copy_n(
            types, (std::min)(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error_type(std::move(error)));
    }

    abort(); // in case user handler "forgets" to abort
}

template<typename Name, typename ReturnType, class Policy, typename... Args>
BOOST_NORETURN auto
method<Name, ReturnType(Args...), Policy>::ambiguous_handler(
    detail::remove_virtual<Args>... args) ->
    typename method<Name, ReturnType(Args...), Policy>::return_type {
    if constexpr (Policy::template has_facet<policies::error_handler>) {
        resolution_error error;
        error.status = resolution_error::ambiguous;
        error.method_name = fn.name;
        error.arity = arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (..., (*ti_iter++ = detail::get_tip<Policy, Args>(args)));
        std::copy_n(
            types, (std::min)(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error_type(std::move(error)));
    }

    abort(); // in case user handler "forgets" to abort
}

#ifndef BOOST_NO_RTTI

inline auto set_error_handler(error_handler_type handler)
    -> error_handler_type {
    auto p = &default_policy::error;
    auto prev = default_policy::error;
    default_policy::error = handler;
    return prev;
}

#endif

} // namespace yomm2
} // namespace yorel

#endif
