#ifndef YOREL_YOMM2_DETAIL_INCLUDED
#define YOREL_YOMM2_DETAIL_INCLUDED

#include "yorel/yomm2/detail/chain.hpp"

#include <iomanip> // for operator<<, setw

namespace yorel {
namespace yomm2 {
namespace detail {

template<typename... Types>
struct types;

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
struct type_id_list<Policy, types<T...>> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr size_t values =
        sizeof...(T) + std::is_base_of_v<policy::deferred_static_rtti, Policy>;
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

template<typename Iterator>
struct type_range {
    Iterator first, last;
    Iterator begin() const {
        return first;
    }
    Iterator end() const {
        return last;
    }
};

template<typename Iterator>
type_range(Iterator b, Iterator e) -> type_range<Iterator>;

struct yomm2_end_of_dump {};

template<typename T>
struct dump_type {
    static_assert(std::is_same_v<T, yomm2_end_of_dump>);
};

template<class Policy>
struct runtime;

namespace mp11 = boost::mp11;

enum { TRACE_RUNTIME = 1, TRACE_CALLS = 2 };

#if defined(YOMM2_SHARED)
extern yOMM2_API std::ostream* logs;
extern yOMM2_API unsigned trace_flags;
#else
inline std::ostream* logs;
inline unsigned trace_flags;
#endif

inline word make_word(size_t i) {
    word w;
    w.i = i;
    return w;
}

inline word make_word(void* pf) {
    word w;
    w.pf = pf;
    return w;
}

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

template<typename T>
using parameter_type_list_t = typename parameter_type_list<T>::type;

template<typename T>
struct is_virtual : std::false_type {};

template<typename T>
struct is_virtual<virtual_<T>> : std::true_type {};

template<typename T, class Policy, bool IsSmartPtr>
struct is_virtual<virtual_ptr<T, Policy, IsSmartPtr>> : std::true_type {};

template<typename T, class Policy, bool IsSmartPtr>
struct is_virtual<const virtual_ptr<T, Policy, IsSmartPtr>&> : std::true_type {
};

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

// ----------
// has_trace

template<typename Container>
auto type_trace(Container t) -> decltype(t.trace);

auto type_trace(...) -> void;

template<typename Container>
constexpr bool has_trace =
    !std::is_same_v<decltype(type_trace(std::declval<Container>())), void>;

// --------------
// intrusive mode

void type_mptr(...);

template<typename Object>
auto type_mptr(Object* obj) -> decltype(obj->yomm2_mptr());

template<typename Object>
using type_mptr_t = decltype(type_mptr(std::declval<Object*>()));

template<typename Object>
constexpr bool has_mptr = !std::is_same_v<
    decltype(type_mptr(std::declval<std::remove_reference_t<Object>*>())),
    void>;

template<typename T>
const char* default_method_name() {
#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
    return typeid(T).name();
#else
    return "method";
#endif
}

template<typename T>
const char* default_definition_name() {
#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
    return typeid(T).name();
#else
    return "definition";
#endif
}

// -------------
// hash function

struct hash_function {
    type_id mult;
    std::size_t shift;

    auto operator()(type_id tip) const {
        return (mult * tip) >> shift;
    }
};

template<class Class, class Policy>
struct virtual_ptr_traits;

inline std::size_t hash(type_id mult, std::size_t shift, type_id value) {
    return static_cast<std::size_t>((mult * value) >> shift);
}

// ----------
// class info

struct class_info : static_chain<class_info>::static_link {
    type_id ti;
    word** method_table;
    type_id *first_base, *last_base;
    bool is_abstract{false};
};

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, detail::types<Class, Bases...>>
    : class_info {
    class_declaration_aux() {
        this->ti = collect_static_type_id<Policy, Class>();
        this->first_base = type_id_list<Policy, types<Bases...>>::begin;
        this->last_base = type_id_list<Policy, types<Bases...>>::end;
        Policy::catalog.classes.push_front(*this);
        this->is_abstract = std::is_abstract_v<Class>;
        this->method_table = &Policy::template method_table<Class>;
    }

    ~class_declaration_aux() {
        Policy::catalog.classes.remove(*this);
    }
};

// -----------
// method info

struct method_info;

struct definition_info : static_chain<definition_info>::static_link {
    ~definition_info();
    method_info* method;
    std::string_view name;
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

struct yOMM2_API method_info : static_chain<method_info>::static_link {
    std::string_view name;
    type_id *vp_begin, *vp_end;
    static_chain<definition_info> specs;
    void* ambiguous;
    void* not_implemented;
    size_t* slots_strides_p;

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

inline definition_info::~definition_info() {
    if (method) {
        method->specs.remove(*this);
    }
}

template<typename T>
struct is_policy_aux : std::is_base_of<policy::abstract_policy, T> {};

template<typename... T>
struct is_policy_aux<types<T...>> : std::false_type {};

template<typename T>
constexpr bool is_policy = is_policy_aux<T>::value;

template<bool, typename... Classes>
struct split_policy_aux;

template<typename Policy, typename... Classes>
struct split_policy_aux<true, Policy, Classes...> {
    using policy = Policy;
    using classes = types<Classes...>;
};

template<typename... Classes>
struct split_policy_aux<false, Classes...> {
    using policy = default_policy;
    using classes = types<Classes...>;
};

template<typename ClassOrPolicy, typename... Classes>
struct split_policy
    : split_policy_aux<is_policy<ClassOrPolicy>, ClassOrPolicy, Classes...> {};

template<typename... Classes>
using get_policy = typename split_policy<Classes...>::policy;

template<typename... Classes>
using remove_policy = typename split_policy<Classes...>::classes;

template<typename Signature>
struct next_ptr_t;

template<typename R, typename... T>
struct next_ptr_t<R(T...)> {
    using type = R (*)(T...);
};

template<typename Method, typename Signature>
inline typename next_ptr_t<Signature>::type next;

template<typename B, typename D, typename = void>
struct requires_dynamic_cast_aux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_aux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};

template<class B, class D>
constexpr bool requires_dynamic_cast = requires_dynamic_cast_aux<B, D>::value;

template<class Policy, class D, class B>
decltype(auto) optimal_cast(B&& obj) {
    if constexpr (requires_dynamic_cast<B, D>) {
        return Policy::template dynamic_cast_<D>(obj);
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
    using resolve_arg_type = const T&;

    static resolve_arg_type rarg(resolve_arg_type arg) {
        return arg;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(arg);
    }

    template<typename D>
    static D& cast(T& obj) {
        return optimal_cast<Policy, D&>(obj);
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, T&&> {
    using polymorphic_type = std::remove_cv_t<T>;
    using resolve_arg_type = const T&;

    static resolve_arg_type rarg(resolve_arg_type arg) {
        return arg;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(arg);
    }

    template<typename D>
    static D&& cast(T&& obj) {
        return optimal_cast<Policy, D&&>(obj);
    }
};

template<class Policy, typename T>
struct virtual_traits<Policy, T*> {
    using polymorphic_type = std::remove_cv_t<T>;
    using resolve_arg_type = const T*;

    static resolve_arg_type rarg(resolve_arg_type arg) {
        return arg;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
    }

    template<typename D>
    static D cast(T* obj) {
        return &optimal_cast<Policy, std::remove_pointer_t<D>&>(*obj);
    }
};

template<class Policy, class Class, bool IsSmartPtr>
struct virtual_traits<Policy, virtual_ptr<Class, Policy, IsSmartPtr>> {
    using polymorphic_type = Class;
    using resolve_arg_type = const virtual_ptr<Class, Policy, IsSmartPtr>&;

    static resolve_arg_type rarg(resolve_arg_type ptr) {
        return ptr;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
    }

    template<typename Derived>
    static Derived cast(virtual_ptr<Class, Policy, IsSmartPtr> ptr) {
        using derived_type = decltype(*std::declval<Derived>());
        return Derived(
            detail::optimal_cast<Policy, derived_type&>(*ptr), ptr.mptr);
    }

    static Class* store(Class& obj) {
        return &obj;
    }
};

template<class Policy, class Class, bool IsSmartPtr>
struct virtual_traits<Policy, const virtual_ptr<Class, Policy, IsSmartPtr>&> {
    using polymorphic_type = Class;
    using resolve_arg_type = const virtual_ptr<Class, Policy, IsSmartPtr>&;

    static resolve_arg_type rarg(resolve_arg_type ptr) {
        return ptr;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Policy, IsSmartPtr>& ptr) {
        using derived_type = decltype(*std::declval<Derived>());
        return virtual_ptr<derived_type, Policy, IsSmartPtr>(
            detail::optimal_cast<Policy, derived_type&>(*ptr), ptr.mptr);
    }

    static Class* store(Class& obj) {
        return &obj;
    }
};

template<class Policy, class Class>
struct virtual_traits<
    Policy, virtual_ptr<std::shared_ptr<Class>, Policy, true>> {
    using resolve_arg_type =
        const virtual_ptr<std::shared_ptr<Class>, Policy, true>&;
    using polymorphic_type = Class;

    static auto rarg(resolve_arg_type ptr) {
        return ptr;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
    }

    template<typename Derived>
    static Derived
    cast(const virtual_ptr<std::shared_ptr<Class>, Policy, true>& ptr) {
        return ptr.template cast<Derived>();
    }
};

template<class Policy, class Class>
struct virtual_traits<
    Policy, const virtual_ptr<std::shared_ptr<Class>, Policy, true>&> {
    using resolve_arg_type =
        const virtual_ptr<std::shared_ptr<Class>, Policy, true>&;
    using polymorphic_type = Class;

    static auto rarg(resolve_arg_type ptr) {
        return ptr;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
    }

    template<typename Derived>
    static auto
    cast(const virtual_ptr<std::shared_ptr<Class>, Policy, true>& ptr) {
        return ptr.template cast<std::remove_reference_t<Derived>>();
    }
};

// -----------
// virtual_ptr

template<typename>
struct is_virtual_ptr_aux : std::false_type {};

template<class Class, class Policy, bool IsSmartPtr>
struct is_virtual_ptr_aux<virtual_ptr<Class, Policy, IsSmartPtr>>
    : std::true_type {};

template<class Class, class Policy, bool IsSmartPtr>
struct is_virtual_ptr_aux<const virtual_ptr<Class, Policy, IsSmartPtr>&>
    : std::true_type {};

template<typename T>
constexpr bool is_virtual_ptr = is_virtual_ptr_aux<T>::value;

template<class Class, class Policy>
struct virtual_ptr_traits {
    static bool constexpr is_smart_ptr = false;
};

template<class Class, class Policy>
struct virtual_ptr_traits<std::shared_ptr<Class>, Policy> {
    static bool constexpr is_smart_ptr = true;

    template<typename OtherPtrRef>
    static auto
    cast(const virtual_ptr<std::shared_ptr<Class>, Policy, true>& ptr) {
        using OtherPtr = typename std::remove_reference_t<OtherPtrRef>;
        using OtherClass = typename OtherPtr::box_type::element_type;

        if constexpr (requires_dynamic_cast<Class&, OtherClass&>) {
            return OtherPtr(
                std::dynamic_pointer_cast<OtherClass>(ptr.obj), ptr.mptr);
        } else {
            return OtherPtr(
                std::static_pointer_cast<OtherClass>(ptr.obj), ptr.mptr);
        }
    }
};

template<class Class, class Policy>
struct virtual_ptr_traits<const std::shared_ptr<Class>&, Policy> {
    static bool constexpr is_smart_ptr = true;

    template<typename OtherPtr>
    static auto
    cast(const virtual_ptr<std::shared_ptr<Class>, Policy, true>& ptr) {
        using OtherClass = typename OtherPtr::box_type::element_type;

        if constexpr (requires_dynamic_cast<Class&, OtherClass&>) {
            return OtherPtr(
                std::dynamic_pointer_cast<OtherClass>(ptr.obj), ptr.mptr);
        } else {
            return OtherPtr(
                std::static_pointer_cast<OtherClass>(ptr.obj), ptr.mptr);
        }
    }
};

template<class Policy, typename T, bool IsVirtual>
struct resolver_type_impl {
    using type = const T&;
};

template<class Policy, typename T>
struct resolver_type_impl<Policy, T*, false> {
    using type = const T*;
};

template<class Policy, typename T>
struct resolver_type_impl<Policy, T&, false> {
    using type = const T&;
};

template<class Policy, typename T>
struct resolver_type_impl<Policy, T&&, false> {
    using type = const T&;
};

template<class Policy, typename T>
struct resolver_type_impl<Policy, T, true> {
    using type = typename virtual_traits<Policy, T>::resolve_arg_type;
};

template<class Policy, typename T>
using resolver_type =
    typename resolver_type_impl<Policy, T, is_virtual<T>::value>::type;

template<class Policy, typename T>
struct argument_traits {
    static const T& rarg(const T& arg) {
        return arg;
    }

    static auto dynamic_type(const T& arg) {
        return Policy::dynamic_type(arg);
    }

    template<typename>
    static T cast(T obj) {
        return std::forward<T>(obj);
    }
};

template<class Policy, typename T>
struct argument_traits<Policy, virtual_<T>> : virtual_traits<Policy, T> {};

template<class Policy, class Class, bool IsSmartPtr>
struct argument_traits<Policy, virtual_ptr<Class, Policy, IsSmartPtr>>
    : virtual_traits<Policy, virtual_ptr<Class, Policy, IsSmartPtr>> {};

template<class Policy, class Class, bool IsSmartPtr>
struct argument_traits<Policy, const virtual_ptr<Class, Policy, IsSmartPtr>&>
    : virtual_traits<Policy, const virtual_ptr<Class, Policy, IsSmartPtr>&> {};

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
struct virtual_traits<Policy, std::shared_ptr<T>> {
    using polymorphic_type = std::remove_cv_t<T>;
    using resolve_arg_type = const std::shared_ptr<T>&;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static resolve_arg_type rarg(resolve_arg_type arg) {
        return arg;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
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

template<class Policy, typename T>
struct virtual_traits<Policy, const std::shared_ptr<T>&> {
    using polymorphic_type = std::remove_cv_t<T>;
    using resolve_arg_type = const std::shared_ptr<T>&;

    static resolve_arg_type rarg(resolve_arg_type arg) {
        return arg;
    }

    static auto dynamic_type(resolve_arg_type arg) {
        return Policy::dynamic_type(*arg);
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
struct virtual_traits<Policy, std::shared_ptr<T>&>
    : virtual_traits<Policy, const std::shared_ptr<T>&> {};

template<typename Method, typename ArgType>
inline auto get_mptr(resolver_type<typename Method::policy_type, ArgType> arg);

template<class Policy>
inline auto check_method_pointer(const word* mptr, type_id dynamic_type);
template<typename MethodArgList>
using polymorphic_types = mp11::mp_transform<
    remove_virtual, mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux {
    using type = void;
};

template<class Policy, typename P, typename Q>
struct select_spec_polymorphic_type_aux<Policy, virtual_<P>, Q> {
    using type = polymorphic_type<Policy, Q>;
};

template<class Policy, typename P, typename Q, bool IsSmartPtr>
struct select_spec_polymorphic_type_aux<
    Policy, virtual_ptr<P, Policy, IsSmartPtr>,
    virtual_ptr<Q, Policy, IsSmartPtr>> {
    using type = typename virtual_traits<
        Policy, virtual_ptr<Q, Policy, IsSmartPtr>>::polymorphic_type;
};

template<class Policy, typename P, typename Q, bool IsSmartPtr>
struct select_spec_polymorphic_type_aux<
    Policy, const virtual_ptr<P, Policy, IsSmartPtr>&,
    const virtual_ptr<Q, Policy, IsSmartPtr>&> {
    using type = typename virtual_traits<
        Policy, const virtual_ptr<Q, Policy, IsSmartPtr>&>::polymorphic_type;
};

template<class Policy, typename P, typename Q>
using select_spec_polymorphic_type =
    typename select_spec_polymorphic_type_aux<Policy, P, Q>::type;

template<class Policy, typename MethodArgList, typename SpecArgList>
using spec_polymorphic_types = mp11::mp_rename<
    mp11::mp_remove<
        mp11::mp_transform_q<
            mp11::mp_bind_front<select_spec_polymorphic_type, Policy>,
            MethodArgList, SpecArgList>,
        void>,
    types>;

template<class Policy, typename ArgType, typename T>
inline uintptr_t get_tip(const T& arg) {
    if constexpr (is_virtual<ArgType>::value) {
        return virtual_traits<Policy, ArgType>::dynamic_type(arg);
    } else {
        return Policy::dynamic_type(arg);
    }
}

// -------
// wrapper

template<class Policy, typename, auto, typename>
struct wrapper;

template<
    class Policy, typename BASE_RETURN, typename... BASE_PARAM, auto SPEC,
    typename... SPEC_PARAM>
struct wrapper<Policy, BASE_RETURN(BASE_PARAM...), SPEC, types<SPEC_PARAM...>> {
    static BASE_RETURN fn(remove_virtual<BASE_PARAM>... arg) {
        using base_type = mp11::mp_first<types<BASE_PARAM...>>;
        using spec_type = mp11::mp_first<types<SPEC_PARAM...>>;
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
struct member_function_wrapper;

template<auto F, class R, class C, typename... Args>
struct member_function_wrapper<F, R (C::*)(Args...)> {
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
using inheritance_map = types<mp11::mp_push_front<
    mp11::mp_filter_q<mp11::mp_bind_back<std::is_base_of, Cs>, types<Cs...>>,
    Cs>...>;

template<class Policy, class... Classes>
struct use_classes_aux;

template<class Policy, class... Classes>
struct use_classes_aux<Policy, types<Classes...>> {
    using type = mp11::mp_apply<
        std::tuple,
        mp11::mp_transform_q<
            mp11::mp_bind_front<class_declaration_aux, Policy>,
            mp11::mp_apply<inheritance_map, types<Classes...>>>>;
};

template<class Policy, class... Classes, class... MoreClassLists>
struct use_classes_aux<Policy, types<types<Classes...>, MoreClassLists...>>
    : use_classes_aux<
          Policy, mp11::mp_append<types<Classes...>, MoreClassLists...>>

{};

template<class First, class Second, class... Rest>
using use_classes_macro = typename std::conditional_t<
    is_policy<Second>, use_classes_aux<Second, types<Rest...>>,
    use_classes_aux<First, types<Second, Rest...>>>::type;

std::ostream* log_on(std::ostream* os);
std::ostream* log_off();

template<class Policy>
inline auto
check_intrusive_method_pointer(const word* mptr, type_id dynamic_type) {
    // Intrusive mode only.

    if constexpr (Policy::runtime_checks) {
        auto& ctx = Policy::context;
        auto p = reinterpret_cast<const char*>(mptr);

        if (ctx.gv.empty()) {
            // no declared methods
            return mptr;
        }

        if (p < reinterpret_cast<const char*>(ctx.gv.data()) ||
            p >= reinterpret_cast<const char*>(ctx.gv.data() + ctx.gv.size())) {
            // probably some random value
            Policy::error(method_table_error{dynamic_type});
        }

        using namespace policy;
        auto index = dynamic_type;

        if constexpr (has_facet<Policy, projection>) {
            index = Policy::project_type_id(index);
        }

        if (index >= ctx.mptrs.size() || mptr != ctx.mptrs[index]) {
            // probably a missing derived<> in a derived class
            Policy::error(method_table_error{dynamic_type});
        }
    }

    return mptr;
}

// -----------------------------------------------------------------------------
// lightweight ostream

struct ostdstream {
    FILE* stream = nullptr;

    ostdstream() {
        if (auto env_trace = getenv("YOMM2_TRACE")) {
            if (std::atoi(env_trace) != 0) {
                on();
            }
        }
    }

    void on(FILE* stream = stderr) {
        this->stream = stream;
    }

    void off() {
        this->stream = nullptr;
    }

    bool is_on() const {
        return stream != nullptr;
    }
};

inline ostdstream cerr;

inline ostdstream& operator<<(ostdstream& os, const char* str) {
    if (os.stream) {
        fputs(str, os.stream);
    }

    return os;
}

inline ostdstream& operator<<(ostdstream& os, const std::string_view& view) {
    if (os.stream) {
        fwrite(view.data(), sizeof(*view.data()), view.length(), os.stream);
    }

    return os;
}

template<typename T>
inline ostdstream& operator<<(ostdstream& os, T* value) {
    if (os.stream) {
        std::array<char, 20> str;
        auto end = std::to_chars(
                       str.data(), str.data() + str.size(),
                       reinterpret_cast<uintptr_t>(value), 16)
                       .ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

inline ostdstream& operator<<(ostdstream& os, size_t value) {
    if (os.stream) {
        std::array<char, 20> str;
        auto end =
            std::to_chars(str.data(), str.data() + str.size(), value).ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

struct empty_base {};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
