#ifndef YOREL_YOMM2_CORE_HPP
#define YOREL_YOMM2_CORE_HPP

#include <functional>
#include <memory>

#include <boost/assert.hpp>

#include <yorel/yomm2/policy.hpp>

#ifndef YOMM2_DEFAULT_POLICY
#define YOMM2_DEFAULT_POLICY ::yorel::yomm2::default_policy
#endif

#include <yorel/yomm2/detail/types.hpp>

namespace yorel {
namespace yomm2 {

// =============================================================================
// Registering classes

namespace detail {

template<class Policy, class Class>
auto collect_static_type_id() -> type_id {
    if constexpr (std::is_base_of_v<policies::deferred_static_rtti, Policy>) {
        return reinterpret_cast<type_id>(Policy::template static_type<Class>);
    } else {
        return Policy::template static_type<Class>();
    }
}

template<class Policy, class TypeList>
struct type_id_list;

template<class Policy, typename... T>
struct type_id_list<Policy, types<T...>> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr std::size_t values = sizeof...(T) +
        std::is_base_of_v<policies::deferred_static_rtti, Policy>;
    static type_id value[values];
    static type_id* begin;
    static type_id* end;
};

template<class Policy, typename... T>
type_id type_id_list<Policy, types<T...>>::value[values] = {
    collect_static_type_id<Policy, T>()...};

template<class Policy, typename... T>
type_id* type_id_list<Policy, types<T...>>::begin = value;

template<class Policy, typename... T>
type_id* type_id_list<Policy, types<T...>>::end = value + sizeof...(T);

template<class Policy>
struct type_id_list<Policy, types<>> {
    static constexpr type_id* const begin = nullptr;
    static constexpr auto end = begin;
};

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
// base. The direct and indirect bases are all included. The runtime will
// extract the direct proper bases.
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

template<typename T>
struct is_policy_fn : std::is_base_of<policies::abstract_policy, T> {};

template<typename... T>
struct is_policy_fn<types<T...>> : std::false_type {};

} // namespace detail

template<class... Classes>
using use_classes = typename detail::use_classes_aux<
    boost::mp11::mp_at<
        detail::types<Classes..., YOMM2_DEFAULT_POLICY>,
        boost::mp11::mp_find_if<
            detail::types<Classes..., YOMM2_DEFAULT_POLICY>,
            detail::is_policy_fn>>,
    boost::mp11::mp_remove_if<
        detail::types<Classes...>, detail::is_policy_fn>>::type;

// =============================================================================
// optimal_cast

namespace detail {

template<typename B, typename D, typename = void>
struct requires_dynamic_cast_ref_aux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_ref_aux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};

template<class B, class D>
constexpr bool requires_dynamic_cast =
    detail::requires_dynamic_cast_ref_aux<B, D>::value;

template<class Policy, class D, class B>
auto optimal_cast(B&& obj) -> decltype(auto) {
    if constexpr (requires_dynamic_cast<B, D>) {
        return Policy::template dynamic_cast_ref<D>(obj);
    } else {
        return static_cast<D>(obj);
    }
}

} // namespace detail

// =============================================================================
// virtual_traits

template<class Policy, typename T>
struct virtual_traits;

template<class Policy, typename T>
struct virtual_traits<Policy, T&> {
    using polymorphic_type = std::remove_cv_t<T>;

    static auto rarg(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T& obj) -> D& {
        return detail::optimal_cast<Policy, D&>(obj);
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, T&&> {
    using polymorphic_type = std::remove_cv_t<T>;

    static auto rarg(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T&& obj) -> D&& {
        return detail::optimal_cast<Policy, D&&>(obj);
    }
};

// =============================================================================
// Common details

namespace detail {

template<typename T>
struct is_virtual : std::false_type {};

template<typename T>
struct is_virtual<virtual_<T>> : std::true_type {};

template<typename T>
struct remove_virtual_aux {
    using type = T;
};

template<typename T>
struct remove_virtual_aux<virtual_<T>> {
    using type = T;
};

template<typename T>
using remove_virtual = typename remove_virtual_aux<T>::type;

template<class Policy, typename T>
using polymorphic_type = typename virtual_traits<Policy, T>::polymorphic_type;

template<typename MethodArgList>
using polymorphic_types = boost::mp11::mp_transform<
    remove_virtual, boost::mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<class Policy, typename T>
struct argument_traits {
    static auto rarg(const T& arg) -> const T& {
        return arg;
    }

    template<typename>
    static auto cast(T value) -> T {
        return std::forward<T>(value);
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

} // namespace detail

// =============================================================================
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

    static auto rarg(const virtual_ptr<Class, Policy>& ptr)
        -> const virtual_ptr<Class, Policy>& {
        return ptr;
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Policy>& ptr) -> decltype(auto) {
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
                    typename virtual_traits<Policy, Other&>::polymorphic_type>;
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

    auto get() const {
        return obj;
    }

    auto operator->() const {
        return get();
    }

    auto operator*() const -> decltype(auto) {
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
                &detail::optimal_cast<Policy, typename Other::element_type&>(*obj);
        }

        return result;
    }

    // consider as private, public for tests only
    auto _vptr() const {
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

// =============================================================================
// Method

namespace detail {

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

template<class Policy, typename MethodParameters, typename OverriderParameters>
using spec_polymorphic_types = boost::mp11::mp_remove<
    boost::mp11::mp_transform_q<
        boost::mp11::mp_bind_front<select_spec_polymorphic_type, Policy>,
        MethodParameters, OverriderParameters>,
    void>;

template<class Method>
struct static_offsets;

template<class Method, typename = void>
struct has_static_offsets : std::false_type {};

template<class Method>
struct has_static_offsets<
    Method, std::void_t<decltype(static_offsets<Method>::slots)>>
    : std::true_type {};

} // namespace detail

template<typename Method, typename Return, class Policy = YOMM2_DEFAULT_POLICY>
struct method;

template<typename Name, typename Return, typename... Parameters, class Policy>
class method<Name(Parameters...), Return, Policy> : public detail::method_info {
    // Aliases used in implementation only. Everything extracted from template
    // arguments is capitalized like the arguments themselves.
    using DeclaredParameters = detail::types<Parameters...>;
    using CallParameters =
        boost::mp11::mp_transform<detail::remove_virtual, DeclaredParameters>;
    using VirtualParameters =
        typename detail::polymorphic_types<DeclaredParameters>;
    using Signature = auto(Parameters...) -> Return;
    using FunctionPointer = auto (*)(detail::remove_virtual<Parameters>...)
        -> Return;
    static constexpr auto Arity = boost::mp11::mp_count_if<
        detail::types<Parameters...>, detail::is_virtual>::value;
    static_assert(Arity > 0, "method must have at least one virtual argument");

    static std::size_t slots_strides[2 * Arity - 1];
    // Slots followed by strides. No stride for first virtual argument.
    // For 1-method: the offset of the method in the method table, which
    // contains a pointer to a function.
    // For multi-methods: the offset of the first virtual argument in the
    // method table, which contains a pointer to the corresponding cell in
    // the dispatch table, followed by the offset of the second argument and
    // the stride in the second dimension, etc.

    template<typename ArgType>
    auto vptr(const ArgType& arg) const -> const std::uintptr_t*;

    template<class Error>
    auto check_static_offset(std::size_t actual, std::size_t expected) const
        -> void;

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
    FunctionPointer resolve(const ArgType&... args) const;

    static BOOST_NORETURN auto
    not_implemented_handler(detail::remove_virtual<Parameters>... args)
        -> Return;
    static BOOST_NORETURN auto
    ambiguous_handler(detail::remove_virtual<Parameters>... args) -> Return;

    template<auto, typename>
    struct thunk;

    friend class generator;

  public:
    // Public aliases.
    using name_type = Name;
    using return_type = Return;
    using next_type = Return (*)(detail::remove_virtual<Parameters>...);

    static method fn;

    method();
    method(const method&) = delete;
    method(method&&) = delete;
    ~method();

    auto operator()(detail::remove_virtual<Parameters>... args) const -> Return;

    template<auto>
    static FunctionPointer next;

  private:
    template<
        auto Overrider, typename OverriderReturn,
        typename... OverriderParameters>
    struct thunk<Overrider, OverriderReturn (*)(OverriderParameters...)> {
        static auto fn(detail::remove_virtual<Parameters>... arg) -> Return;
        using OverriderParameterTypeIds = detail::type_id_list<
            Policy,
            detail::spec_polymorphic_types<
                Policy, DeclaredParameters,
                detail::types<OverriderParameters...>>>;
    };

    template<auto Function>
    struct override_impl {
        explicit override_impl(FunctionPointer* next = nullptr);
    };

    template<auto Function, typename FunctionType>
    struct override_aux;

    template<auto Function, typename FnReturnType, typename... FnParameters>
    struct override_aux<Function, FnReturnType (*)(FnParameters...)>
        : override_impl<Function> {
    };

    template<
        auto Function, class FnClass, typename FnReturnType,
        typename... FnParameters>
    struct override_aux<
        Function, FnReturnType (FnClass::*)(FnParameters...)> {
        static auto fn(FnClass* this_, FnParameters&&... args) -> FnReturnType {
            return (this_->*Function)(std::forward<FnParameters>(args)...);
        }

        override_impl<fn> impl{&next<Function>};
    };

  public:
    template<auto... Function>
    struct override {
        std::tuple<override_aux<Function, decltype(Function)>...> impl;
    };
};

template<typename Name, typename Return, typename... Parameters, class Policy>
method<Name(Parameters...), Return, Policy>
    method<Name(Parameters...), Return, Policy>::fn;

template<typename Name, typename Return, typename... Parameters, class Policy>
template<auto>
typename method<Name(Parameters...), Return, Policy>::FunctionPointer
    method<Name(Parameters...), Return, Policy>::next;

template<typename T>
constexpr bool is_method = std::is_base_of_v<detail::method_info, T>;

template<typename Name, typename Return, typename... Parameters, class Policy>
method<Name(Parameters...), Return, Policy>::method() {
    this->slots_strides_ptr = slots_strides;

    using virtual_type_ids = detail::type_id_list<
        Policy,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<detail::polymorphic_type, Policy>,
            VirtualParameters>>;
    this->vp_begin = virtual_type_ids::begin;
    this->vp_end = virtual_type_ids::end;
    this->not_implemented = (void*)not_implemented_handler;
    this->ambiguous = (void*)ambiguous_handler;
    this->method_type = Policy::template static_type<method>();
    Policy::methods.push_back(*this);
}

template<typename Name, typename Return, typename... Parameters, class Policy>
std::size_t
    method<Name(Parameters...), Return, Policy>::slots_strides[2 * Arity - 1];

template<typename Name, typename Return, typename... Parameters, class Policy>
method<Name(Parameters...), Return, Policy>::~method() {
    Policy::methods.remove(*this);
}

template<typename Name, typename Return, typename... Parameters, class Policy>
template<class Error>
auto method<Name(Parameters...), Return, Policy>::check_static_offset(
    std::size_t actual, std::size_t expected) const -> void {
    using namespace detail;

    if (actual != expected) {
        if (Policy::template has_facet<policies::error_handler>) {
            Error error;
            error.method = Policy::template static_type<method>();
            error.expected = this->slots_strides[0];
            error.actual = actual;
            Policy::error(error);

            abort();
        }
    }
}

// -----------------------------------------------------------------------------
// method dispatch

template<typename Name, typename Return, typename... Parameters, class Policy>
BOOST_FORCEINLINE auto method<Name(Parameters...), Return, Policy>::operator()(
    detail::remove_virtual<Parameters>... args) const -> Return {
    using namespace detail;
    auto pf = resolve(argument_traits<Policy, Parameters>::rarg(args)...);

    return pf(std::forward<remove_virtual<Parameters>>(args)...);
}

template<typename Name, typename Return, typename... Parameters, class Policy>
template<typename... ArgType>
BOOST_FORCEINLINE
    typename method<Name(Parameters...), Return, Policy>::FunctionPointer
    method<Name(Parameters...), Return, Policy>::resolve(
        const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (Arity == 1) {
        pf = resolve_uni<types<Parameters...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<0, types<Parameters...>, ArgType...>(args...);
    }

    return reinterpret_cast<FunctionPointer>(pf);
}

template<typename Name, typename Return, typename... Parameters, class Policy>
template<typename ArgType>
BOOST_FORCEINLINE auto
method<Name(Parameters...), Return, Policy>::vptr(const ArgType& arg) const
    -> const std::uintptr_t* {
    if constexpr (is_virtual_ptr<ArgType>) {
        return arg._vptr();
        // No need to check the method pointer: this was done when the
        // virtual_ptr was created.
    } else {
        return Policy::dynamic_vptr(arg);
    }
}

template<typename Name, typename Return, typename... Parameters, class Policy>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
BOOST_FORCEINLINE auto method<Name(Parameters...), Return, Policy>::resolve_uni(
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
            if constexpr (Policy::template has_facet<
                              policies::runtime_checks>) {
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

template<typename Name, typename Return, typename... Parameters, class Policy>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name(Parameters...), Return, Policy>::resolve_multi_first(
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
            if constexpr (Policy::template has_facet<
                              policies::runtime_checks>) {
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

template<typename Name, typename Return, typename... Parameters, class Policy>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name(Parameters...), Return, Policy>::resolve_multi_next(
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
            if constexpr (Policy::template has_facet<
                              policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    this->slots_strides[VirtualArg], slot);
                check_static_offset<static_stride_error>(
                    this->slots_strides[2 * VirtualArg], stride);
            }
        } else {
            slot = this->slots_strides[VirtualArg];
            stride = this->slots_strides[Arity + VirtualArg - 1];
        }

        dispatch = dispatch + vtbl[slot] * stride;
    }

    if constexpr (VirtualArg + 1 == Arity) {
        return *dispatch;
    } else {
        return resolve_multi_next<
            VirtualArg + 1, mp_rest<MethodArgList>, MoreArgTypes...>(
            dispatch, more_args...);
    }
}

// -----------------------------------------------------------------------------
// Error handling

template<typename Name, typename Return, typename... Parameters, class Policy>
BOOST_NORETURN auto
method<Name(Parameters...), Return, Policy>::not_implemented_handler(
    detail::remove_virtual<Parameters>... args) -> Return {
    if constexpr (Policy::template has_facet<policies::error_handler>) {
        resolution_error error;
        error.status = resolution_error::no_definition;
        error.method = Policy::template static_type<method>();
        error.arity = Arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (...,
         (*ti_iter++ = Policy::dynamic_type(
              detail::argument_traits<Policy, Parameters>::rarg(args))));
        std::copy_n(
            types, (std::min)(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error);
    }

    abort(); // in case user handler "forgets" to abort
}

template<typename Name, typename Return, typename... Parameters, class Policy>
BOOST_NORETURN auto
method<Name(Parameters...), Return, Policy>::ambiguous_handler(
    detail::remove_virtual<Parameters>... args) -> Return {
    if constexpr (Policy::template has_facet<policies::error_handler>) {
        resolution_error error;
        error.status = resolution_error::ambiguous;
        error.method = Policy::template static_type<method>();
        error.arity = Arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (...,
         (*ti_iter++ = Policy::dynamic_type(
              detail::argument_traits<Policy, Parameters>::rarg(args))));
        std::copy_n(
            types, (std::min)(sizeof...(args), resolution_error::max_types),
            &error.types[0]);
        Policy::error(error);
    }

    abort(); // in case user handler "forgets" to abort
}

// -----------------------------------------------------------------------------
// thunk

template<typename Name, typename Return, typename... Parameters, class Policy>
template<
    auto Overrider, typename OverriderReturn, typename... OverriderParameters>
auto method<Name(Parameters...), Return, Policy>::
    thunk<Overrider, OverriderReturn (*)(OverriderParameters...)>::fn(
        detail::remove_virtual<Parameters>... arg) -> Return {
    return Overrider(
        detail::argument_traits<Policy, Parameters>::template cast<
            OverriderParameters>(detail::remove_virtual<Parameters>(arg))...);
}

// -----------------------------------------------------------------------------
// overriders

template<typename Name, typename Return, typename... Parameters, class Policy>
template<auto Function>
method<Name(Parameters...), Return, Policy>::override_impl<
    Function>::override_impl(FunctionPointer* p_next) {
    // Work around MSVC bug: using &next<Function> as a default value
    // for 'next' confuses it about Parameters not being expanded.
    if (!p_next) {
        p_next = &next<Function>;
    }

    static detail::overrider_info info;

    if (info.method) {
        BOOST_ASSERT(info.method == &fn);
        return;
    }

    info.method = &fn;
    info.type = Policy::template static_type<decltype(Function)>();
    info.next = reinterpret_cast<void**>(p_next);
    using Thunk = thunk<Function, decltype(Function)>;
    info.pf = (void*)Thunk::fn;
    info.vp_begin = Thunk::OverriderParameterTypeIds::begin;
    info.vp_end = Thunk::OverriderParameterTypeIds::end;
    fn.specs.push_back(info);
}

} // namespace yomm2
} // namespace yorel

#endif
