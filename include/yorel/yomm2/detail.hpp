#ifndef YOREL_YOMM2_DETAIL_HPP
#define YOREL_YOMM2_DETAIL_HPP

#include <boost/assert.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <yorel/yomm2/detail/list.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

template<class Policy, class Class>
type_id collect_static_type_id() {
    if constexpr (std::is_base_of_v<policy::deferred_static_rtti, Policy>) {
        return reinterpret_cast<type_id>(Policy::template static_type<Class>);
    } else {
        return Policy::template static_type<Class>();
    }
}

template<class Policy, class TypeList>
struct type_id_list;

template<class Policy, typename... T>
struct type_id_list<Policy, boost::mp11::mp_list<T...>> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr std::size_t values =
        sizeof...(T) + std::is_base_of_v<policy::deferred_static_rtti, Policy>;
    static type_id value[values];
    static type_id* begin;
    static type_id* end;
};

template<class Policy, typename... T>
type_id type_id_list<Policy, boost::mp11::mp_list<T...>>::value[values] = {
    collect_static_type_id<Policy, T>()...};

template<class Policy, typename... T>
type_id* type_id_list<Policy, boost::mp11::mp_list<T...>>::begin = value;

template<class Policy, typename... T>
type_id* type_id_list<Policy, boost::mp11::mp_list<T...>>::end =
    value + sizeof...(T);

template<class Policy>
struct type_id_list<Policy, boost::mp11::mp_list<>> {
    static constexpr type_id* const begin = nullptr;
    static constexpr auto end = begin;
};

struct yomm2_end_of_dump {};

template<typename T>
struct dump_type {
    static_assert(std::is_same_v<T, yomm2_end_of_dump>);
};

template<class Policy>
struct runtime;

namespace mp11 = boost::mp11;

template<typename>
struct parameter_type_list;

template<typename ReturnType, typename... ParameterTypes>
struct parameter_type_list<ReturnType(ParameterTypes...)> {
    using type = boost::mp11::mp_list<ParameterTypes...>;
};

template<typename ReturnType, typename... ParameterTypes>
struct parameter_type_list<ReturnType (*)(ParameterTypes...)> {
    using type = boost::mp11::mp_list<ParameterTypes...>;
};

template<typename T>
using parameter_type_list_t = typename parameter_type_list<T>::type;

template<typename T>
struct is_virtual : std::false_type {};

template<typename T>
struct is_virtual<virtual_<T>> : std::true_type {};

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

void type_of_name(...);

template<typename Container>
auto type_of_name(Container t) -> decltype(t.name());

template<typename Container>
using type_of_name_t = decltype(type_of_name(std::declval<Container>()));

template<typename Container>
constexpr bool has_name_v =
    std::is_same_v<type_of_name_t<Container>, std::string_view>;

// ----------
// has_next_v

void type_next(...);

template<typename Container>
auto type_next(Container t) -> decltype(t.next);

template<typename Container>
using type_next_t = decltype(type_next(std::declval<Container>()));

template<typename Container, typename Next>
constexpr bool has_next_v = std::is_same_v<type_next_t<Container>, Next>;

template<typename T>
const char* default_method_name() {
#ifndef BOOST_NO_RTTI
    return typeid(T).name();
#else
    return "method";
#endif
}

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, boost::mp11::mp_list<Class, Bases...>>
    : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Policy, Class>();
        this->first_base =
            type_id_list<Policy, boost::mp11::mp_list<Bases...>>::begin;
        this->last_base =
            type_id_list<Policy, boost::mp11::mp_list<Bases...>>::end;
        Policy::classes.push_back(*this);
        this->is_abstract = std::is_abstract_v<Class>;
        this->static_vptr = &Policy::template static_vptr<Class>;
    }

    ~class_declaration_aux() {
        Policy::classes.remove(*this);
    }
};

// -----------
// method info

struct method_info;

struct definition_info : static_list<definition_info>::static_link {
    ~definition_info();
    method_info* method; // for the destructor, to remove definition
    type_id type;        // of the function, for trace
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

template<typename... Ts>
constexpr auto arity =
    boost::mp11::mp_count_if<boost::mp11::mp_list<Ts...>, is_virtual>::value;

inline definition_info::~definition_info() {
    if (method) {
        method->specs.remove(*this);
    }
}

template<typename T>
struct is_policy_aux : std::is_base_of<policy::abstract_policy, T> {};

template<typename... T>
struct is_policy_aux<boost::mp11::mp_list<T...>> : std::false_type {};

template<typename T>
constexpr bool is_policy = is_policy_aux<T>::value;

template<typename T>
constexpr bool is_not_policy = !is_policy<T>;

template<typename T>
struct is_method_aux : std::false_type {};

template<typename... T>
struct is_method_aux<method<T...>> : std::true_type {};

template<typename T>
constexpr bool is_method = is_method_aux<T>::value;

template<typename Signature>
struct next_ptr_t;

template<typename R, typename... T>
struct next_ptr_t<R(T...)> {
    using type = R (*)(T...);
};

template<typename Method, typename Signature>
inline typename next_ptr_t<Signature>::type next;

template<typename B, typename D, typename = void>
struct requires_dynamic_cast_ref_aux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_ref_aux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};

template<class B, class D>
constexpr bool requires_dynamic_cast =
    requires_dynamic_cast_ref_aux<B, D>::value;

template<class Policy, class D, class B>
decltype(auto) optimal_cast(B&& obj) {
    if constexpr (requires_dynamic_cast<B, D>) {
        return Policy::template dynamic_cast_ref<D>(obj);
    } else {
        return static_cast<D>(obj);
    }
}

template<class Policy, typename T>
struct virtual_traits;

template<class Policy, typename T>
struct virtual_traits<Policy, virtual_<T>> : virtual_traits<Policy, T> {};

template<class Policy, typename T>
using polymorphic_type = typename virtual_traits<Policy, T>::polymorphic_type;

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

// -----------------------------------------------------------------------------
// virtual_ptr

template<class Class, class Policy>
struct is_virtual<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual<const virtual_ptr<Class, Policy>&> : std::true_type {};

template<class Class, class Policy>
struct virtual_ptr_traits {
    static bool constexpr is_smart_ptr = false;
    using polymorphic_type = Class;
};

template<class Class, class Policy>
struct virtual_ptr_traits<std::shared_ptr<Class>, Policy> {
    static bool constexpr is_smart_ptr = true;
    using polymorphic_type = Class;

    template<typename OtherPtrRef>
    static decltype(auto) cast(const std::shared_ptr<Class>& ptr) {
        using OtherPtr = typename std::remove_reference_t<OtherPtrRef>;
        using OtherClass = typename OtherPtr::box_type::element_type;

        if constexpr (requires_dynamic_cast<Class&, OtherClass&>) {
            return std::dynamic_pointer_cast<OtherClass>(ptr);
        } else {
            return std::static_pointer_cast<OtherClass>(ptr);
        }
    }
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

template<typename>
struct is_virtual_ptr_aux : std::false_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<const virtual_ptr<Class, Policy>&> : std::true_type {
};

template<typename T>
constexpr bool is_virtual_ptr = is_virtual_ptr_aux<T>::value;

template<class... Ts>
using virtual_ptr_class = std::conditional_t<
    sizeof...(Ts) == 2,
    boost::mp11::mp_second<boost::mp11::mp_list<Ts..., void>>,
    boost::mp11::mp_first<boost::mp11::mp_list<Ts...>>>;

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

template<typename T>
struct shared_ptr_traits {
    static const bool is_shared_ptr = false;
};

template<typename T>
struct shared_ptr_traits<std::shared_ptr<T>> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = false;
    using polymorphic_type = T;
};

template<typename T>
struct shared_ptr_traits<const std::shared_ptr<T>&> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = true;
    using polymorphic_type = T;
};

template<class Policy, typename T>
struct virtual_traits<Policy, const std::shared_ptr<T>&> {
    using polymorphic_type = std::remove_cv_t<T>;

    static const T& rarg(const std::shared_ptr<T>& arg) {
        return *arg;
    }

    template<class DERIVED>
    static void check_cast() {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            shared_ptr_traits<DERIVED>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(std::is_class_v<
                      typename shared_ptr_traits<DERIVED>::polymorphic_type>);
    }

    template<class DERIVED>
    static auto cast(const std::shared_ptr<T>& obj) {
        check_cast<DERIVED>();

        if constexpr (requires_dynamic_cast<T*, DERIVED>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
        }
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, std::shared_ptr<T>> {
    using polymorphic_type = std::remove_cv_t<T>;

    static const T& rarg(const std::shared_ptr<T>& arg) {
        return *arg;
    }

    template<class DERIVED>
    static void check_cast() {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            !shared_ptr_traits<DERIVED>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(std::is_class_v<
                      typename shared_ptr_traits<DERIVED>::polymorphic_type>);
    }
    template<class DERIVED>
    static auto cast(const std::shared_ptr<T>& obj) {
        check_cast<DERIVED>();

        if constexpr (requires_dynamic_cast<T*, DERIVED>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
        }
    }
};

template<typename MethodArgList>
using polymorphic_types = boost::mp11::mp_transform<
    remove_virtual, boost::mp11::mp_filter<detail::is_virtual, MethodArgList>>;

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

template<class Policy, typename ArgType, typename T>
inline uintptr_t get_tip(const T& arg) {
    if constexpr (is_virtual<ArgType>::value) {
        return Policy::dynamic_type(virtual_traits<Policy, ArgType>::rarg(arg));
    } else {
        return Policy::dynamic_type(arg);
    }
}

template<typename... Classes>
using get_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<boost::mp11::mp_list<Classes...>>>,
    boost::mp11::mp_back<boost::mp11::mp_list<Classes...>>,
    YOMM2_DEFAULT_POLICY>;

template<typename... Classes>
using remove_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<boost::mp11::mp_list<Classes...>>>,
    boost::mp11::mp_pop_back<boost::mp11::mp_list<Classes...>>,
    boost::mp11::mp_list<Classes...>>;

template<class... Ts>
using virtual_ptr_policy = std::conditional_t<
    sizeof...(Ts) == 2, boost::mp11::mp_first<boost::mp11::mp_list<Ts...>>,
    YOMM2_DEFAULT_POLICY>;

// -----------------------------------------------------------------------------
// thunk

template<class Policy, typename, auto, typename>
struct thunk;

template<
    class Policy, typename BASE_RETURN, typename... BASE_PARAM, auto SPEC,
    typename... SPEC_PARAM>
struct thunk<
    Policy, BASE_RETURN(BASE_PARAM...), SPEC,
    boost::mp11::mp_list<SPEC_PARAM...>> {
    static BASE_RETURN fn(remove_virtual<BASE_PARAM>... arg) {
        using base_type =
            boost::mp11::mp_first<boost::mp11::mp_list<BASE_PARAM...>>;
        using spec_type =
            boost::mp11::mp_first<boost::mp11::mp_list<SPEC_PARAM...>>;
        return SPEC(
            argument_traits<Policy, BASE_PARAM>::template cast<SPEC_PARAM>(
                remove_virtual<BASE_PARAM>(arg))...);
    }
};

template<typename Method, typename Container>
struct next_aux {
    static typename Method::next_type next;
};

template<typename Method, typename Container>
typename Method::next_type next_aux<Method, Container>::next;

template<auto F, typename T>
struct member_function_thunk;

template<auto F, class R, class C, typename... Args>
struct member_function_thunk<F, R (C::*)(Args...)> {
    static R fn(C* this_, Args&&... args) {
        return (this_->*F)(args...);
    }
};

// Collect the base classes of a list of classes. The result is a mp11 map that
// associates each class to a list starting with the class itself, followed by
// all its bases, as per std::is_base_of. Thus the list includes the class
// itself at least twice: at the front, and down the list, as its own improper
// base. The direct and its direct and indirect proper bases are included. The
// runtime will extract the direct proper bases. See unit tests for an example.
template<typename... Cs>
using inheritance_map = boost::mp11::mp_list<boost::mp11::mp_push_front<
    boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_back<std::is_base_of, Cs>,
        boost::mp11::mp_list<Cs...>>,
    Cs>...>;

template<class Policy, class... Classes>
struct use_classes_aux;

template<class Policy, class... Classes>
struct use_classes_aux<Policy, boost::mp11::mp_list<Classes...>> {
    using type = boost::mp11::mp_apply<
        std::tuple,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<class_declaration_aux, Policy>,
            boost::mp11::mp_apply<
                inheritance_map, boost::mp11::mp_list<Classes...>>>>;
};

template<class Policy, class... Classes, class... MoreClassLists>
struct use_classes_aux<
    Policy,
    boost::mp11::mp_list<boost::mp11::mp_list<Classes...>, MoreClassLists...>>
    : use_classes_aux<
          Policy,
          boost::mp11::mp_append<
              boost::mp11::mp_list<Classes...>, MoreClassLists...>>

{};

template<typename... Ts>
using second_last = boost::mp11::mp_at_c<
    boost::mp11::mp_list<Ts...>,
    boost::mp11::mp_size<boost::mp11::mp_list<Ts...>>::value - 2>;

template<class... Classes>
using use_classes_macro = typename std::conditional_t<
    is_policy<second_last<Classes...>>,
    use_classes_aux<
        second_last<Classes...>,
        boost::mp11::mp_pop_back<
            boost::mp11::mp_pop_back<boost::mp11::mp_list<Classes...>>>>,
    use_classes_aux<
        boost::mp11::mp_back<boost::mp11::mp_list<Classes...>>,
        boost::mp11::mp_pop_back<boost::mp11::mp_list<Classes...>>>>::type;

struct empty_base {};

// -----------------------------------------------------------------------------
// static_slots

template<class Method>
struct static_offsets;

template<class Method, typename = void>
struct has_static_offsets : std::false_type {};

template<class Method>
struct has_static_offsets<
    Method, std::void_t<decltype(static_offsets<Method>::slots)>>
    : std::true_type {};

// -----------------------------------------------------------------------------
// report

struct update_method_report {
    std::size_t cells = 0;
    std::size_t concrete_cells = 0;
    std::size_t not_implemented = 0;
    std::size_t concrete_not_implemented = 0;
    std::size_t ambiguous = 0;
    std::size_t concrete_ambiguous = 0;
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
