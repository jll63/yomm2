#ifndef YOREL_YOMM2_DETAIL_INCLUDED
#define YOREL_YOMM2_DETAIL_INCLUDED

#include <yorel/yomm2/detail/chain.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

struct runtime;

yOMM2_API void update_methods(catalog& cat, context& ht);

namespace mp11 = boost::mp11;

using ti_ptr = const std::type_info*;

enum { TRACE_RUNTIME = 1, TRACE_CALLS = 2 };
extern std::ostream* logs;
extern unsigned trace_flags;

#if defined(YOMM2_TRACE) && (YOMM2_TRACE & 1)
constexpr unsigned trace_enabled = YOMM2_TRACE;
#elif !defined(NDEBUG)
constexpr unsigned trace_enabled = TRACE_RUNTIME;
#else
constexpr unsigned trace_enabled = 0;
#endif

template<unsigned Flags>
struct trace_type {
    trace_type& operator++();
    int indent{0};
};

inline trace_type<TRACE_CALLS> call_trace;

template<typename T, unsigned Flags>
inline trace_type<Flags>& operator<<(trace_type<Flags>& trace, T&& value) {
    if constexpr (bool(trace_enabled & Flags)) {
        if (trace_flags & Flags) {
            *logs << value;
        }
    }
    return trace;
}

union word {
    void* pf;
    const word* pw;
    int i;
    uintptr_t ul;
    const void* ti;
};

inline word make_word(int i) {
    word w;
    w.i = i;
    return w;
}

inline word make_word(uintptr_t value) {
    word w;
    w.ul = value;
    return w;
}

inline word make_word(void* pf) {
    word w;
    w.pf = pf;
    return w;
}

template<auto Function, typename Type = decltype(Function)>
struct parameter_type_list;

template<auto Function, typename ReturnType, typename... ParameterTypes>
struct parameter_type_list<Function, ReturnType (*)(ParameterTypes...)> {
    using type = types<ParameterTypes...>;
};

template<auto Function, typename Type = decltype(Function)>
using parameter_type_list_t =
    typename parameter_type_list<Function, Type>::type;

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

void type_next(...);

template<typename Container>
auto type_next(Container t) -> decltype(t.next);

template<typename Container>
using type_next_t = decltype(type_next(std::declval<Container>()));

template<typename Container, typename Next>
constexpr bool has_next_v = std::is_same_v<type_next_t<Container>, Next>;

extern yOMM2_API error_handler_type error_handler;

struct hash_function {
    std::uintptr_t mult;
    std::size_t shift;

    std::size_t operator()(const void* p) const {
        return static_cast<std::size_t>(
            (mult * reinterpret_cast<std::uintptr_t>(const_cast<void*>(p))) >>
            shift);
    }
};

inline std::size_t hash(std::uintptr_t mult, std::size_t shift, const void* p) {
    return static_cast<std::size_t>(
        (mult * reinterpret_cast<std::uintptr_t>(const_cast<void*>(p))) >>
        shift);
}

struct class_info : static_chain<class_info>::static_link {
    detail::ti_ptr ti;
    const detail::ti_ptr *first_base, *last_base;
    const char* name() const {
        return ti->name();
    }
    bool is_abstract{false};
};

struct method_info;

struct definition_info : static_chain<definition_info>::static_link {
    ~definition_info();
    method_info* method;
    std::string_view name;
    void** next;
    const std::type_info *const *vp_begin, *const *vp_end;
    void* pf;
};

struct yOMM2_API method_info : static_chain<method_info>::static_link {
    std::string_view name;
    const std::type_info *const *vp_begin, *const *vp_end;
    static_chain<definition_info> specs;
    void* ambiguous;
    void* not_implemented;
    const std::type_info* hash_factors_placement;

    // Filled by 'runtime'. Keep them together, to fit in cache line. Also, put
    // them at the end because method_info may be extended with more runtime
    // info (see hash_factors_in_method).
    detail::word* hash_table;
    detail::word slots_strides; // slot 0, slot 1,  stride 1, slot 2, ...

    virtual void install_hash_factors(runtime&);

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

inline definition_info::~definition_info() {
    if (method) {
        method->specs.remove(*this);
    }
}

template<bool, typename... Classes>
struct split_policy_aux;

template<typename Policy, typename... Classes>
struct split_policy_aux<true, Policy, Classes...> {
    using policy = Policy;
    using classes = types<Classes...>;
};

template<typename... Classes>
struct split_policy_aux<false, Classes...> {
    using policy = policy::default_policy;
    using classes = types<Classes...>;
};

template<typename ClassOrPolicy, typename... Classes>
struct split_policy
    : split_policy_aux<
          std::is_base_of_v<policy::abstract_policy, ClassOrPolicy>,
          ClassOrPolicy, Classes...> {};

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

template<typename T>
struct virtual_traits;

template<typename T>
using polymorphic_type = typename virtual_traits<T>::polymorphic_type;

template<typename B, typename D, typename R = D>
using static_cast_t = std::enable_if_t<
    !boost::is_virtual_base_of<B, polymorphic_type<D>>::value, R>;

template<class B, class D, typename R = D>
using dynamic_cast_t = std::enable_if_t<
    boost::is_virtual_base_of<B, polymorphic_type<D>>::value, R>;

template<typename T>
struct virtual_traits<T&> {
    using argument_type = T&;
    using resolver_type = const T&;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(arg);
    }

    template<class DERIVED>
    static static_cast_t<polymorphic_type, DERIVED> cast(T& obj) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<polymorphic_type, DERIVED> cast(T& obj) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct virtual_traits<const T&> {
    using argument_type = const T&;
    using resolver_type = const T&;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(arg);
    }

    template<class DERIVED>
    static static_cast_t<polymorphic_type, DERIVED> cast(const T& obj) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<polymorphic_type, DERIVED> cast(const T& obj) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct virtual_traits<T&&> {
    using argument_type = T&&;
    using resolver_type = const T&;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(arg);
    }

    template<class DERIVED>
    static static_cast_t<polymorphic_type, DERIVED> cast(T&& obj) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<polymorphic_type, DERIVED> cast(T&& obj) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct virtual_traits<T*> {
    using argument_type = T*;
    using resolver_type = const T*;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(*arg);
    }

    template<class DERIVED>
    static static_cast_t<polymorphic_type, DERIVED> cast(T* obj) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<polymorphic_type, DERIVED> cast(T* obj) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct resolver_type_impl {
    using type = T;
};

template<typename T>
struct resolver_type_impl<virtual_<T>> {
    using type = typename virtual_traits<T>::resolver_type;
};

template<typename T>
using resolver_type = typename resolver_type_impl<T>::type;

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

template<typename T>
struct virtual_traits<std::shared_ptr<T>> {
    using argument_type = std::shared_ptr<T>;
    using resolver_type = const std::shared_ptr<T>&;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(*arg);
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
    static static_cast_t<polymorphic_type, DERIVED> cast(argument_type obj) {
        check_cast<DERIVED>();
        return std::static_pointer_cast<
            typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<polymorphic_type, DERIVED> cast(argument_type obj) {
        check_cast<DERIVED>();
        return std::dynamic_pointer_cast<
            typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
    }
};

template<typename T>
struct virtual_traits<const std::shared_ptr<T>&> {
    using argument_type = const std::shared_ptr<T>&;
    using resolver_type = const std::shared_ptr<T>&;
    using polymorphic_type = std::remove_cv_t<T>;
    static_assert(std::is_polymorphic_v<polymorphic_type>);

    static auto key(resolver_type arg) {
        return &typeid(*arg);
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
    static static_cast_t<
        polymorphic_type, DERIVED,
        std::shared_ptr<typename shared_ptr_traits<DERIVED>::polymorphic_type>>
    cast(argument_type obj) {
        check_cast<DERIVED>();
        return std::static_pointer_cast<
            typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
    }

    template<class DERIVED>
    static dynamic_cast_t<
        polymorphic_type, DERIVED,
        std::shared_ptr<typename shared_ptr_traits<DERIVED>::polymorphic_type>>
    cast(argument_type obj) {
        check_cast<DERIVED>();
        return std::dynamic_pointer_cast<
            typename shared_ptr_traits<DERIVED>::polymorphic_type>(obj);
    }
};

template<typename T>
using virtual_arg_t = typename virtual_traits<T>::argument_type;

template<typename MethodArgList>
using polymorphic_types = mp11::mp_transform<
    remove_virtual, mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<typename P, typename Q>
struct select_spec_polymorphic_type {
    using type = void;
};

template<typename T>
struct universal_traits {
    static auto key(...) {
        return &typeid(T);
    }
};

template<typename T>
struct universal_traits<virtual_<T>> : virtual_traits<T> {};

template<typename P, typename Q>
struct select_spec_polymorphic_type<virtual_<P>, Q> {
    using type = polymorphic_type<Q>;
};

template<typename MethodArgList, typename SpecArgList>
using spec_polymorphic_types = mp11::mp_rename<
    mp11::mp_remove<
        mp11::mp_transform_q<
            mp11::mp_quote_trait<select_spec_polymorphic_type>, MethodArgList,
            SpecArgList>,
        void>,
    types>;

template<typename...>
struct type_id_list;

template<typename... T>
struct type_id_list<types<T...>> {
    static constexpr const std::type_info* value[] = {&typeid(T)...};
    static constexpr auto begin = value;
    static constexpr auto end = value + sizeof...(T);
};

template<>
struct type_id_list<types<>> {
    static constexpr const std::type_info** begin = nullptr;
    static constexpr auto end = begin;
};

struct resolve_init {
    const word* hash_table;
    std::uintptr_t hash_mult;
    std::size_t hash_shift;
    word ss;
};

struct only_virtual_arg {
    const word* dispatch;
};

struct next_virtual_arg {
    const word* hash_table;
    std::uintptr_t hash_mult;
    std::size_t hash_shift;
    const word* ssp;
    const word* dispatch;
};

template<int Arity, typename T>
struct resolve_arg {
    resolve_arg(...) {
    }
};

template<int Arity, typename T>
struct resolve_arg<Arity, virtual_<T>> {
    typename virtual_traits<T>::resolver_type arg;
};

template<typename Previous, int Arity, typename T>
inline auto operator<<(Previous a, resolve_arg<Arity, T>) {
    return a;
}

inline const word* get_mptr(
    const word* hash_table, std::uintptr_t hash_mult, std::size_t hash_shift,
    const std::type_info* ti) {
    auto index = hash(hash_mult, hash_shift, ti);
    auto mptr = hash_table[index].pw;
#ifndef NDEBUG
    if (!mptr || hash_table[-1].pw[index].ti != ti) {
        unknown_class_error error;
        error.ti = ti;
        error_handler(error_type(error));
        abort();
    }
#endif
    return mptr;
}

template<int Arity, typename T>
inline auto
operator<<(resolve_init a, const resolve_arg<Arity, virtual_<T>> b) {
    // Get the method table.
    auto key = virtual_traits<T>::key(b.arg);

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << "\n  key = " << key;
    }

    auto mptr = get_mptr(a.hash_table, a.hash_mult, a.hash_shift, key);

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " mptr = " << mptr;
    }

    // For a multi-method, slots-strides contains a pointer to the successive
    // slot-stride pairs, one pair for each virtual parameter.
    auto ssp = a.ss.pw;
    auto slot = ssp++->i;

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " slot = " << slot;
    }

    // The first virtual parameter is special.  Since its stride is 1, there is
    // no need to store it. Also, the method table contains a pointer into the
    // multi-dimensional dispatch table, already resolved to the appropriate
    // group.
    auto dispatch = mptr[slot].pw;

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " dispatch = " << dispatch;
    }

    return next_virtual_arg{
        a.hash_table, a.hash_mult, a.hash_shift, ssp, dispatch};
}

template<int Arity, typename T>
inline auto
operator<<(next_virtual_arg a, const resolve_arg<Arity, virtual_<T>> b) {
    // Get the method table.
    auto key = virtual_traits<T>::key(b.arg);

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << "\n  key = " << key;
    }

    auto mptr = get_mptr(a.hash_table, a.hash_mult, a.hash_shift, key);

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " mptr = " << mptr;
    }

    // Get the slot and stride for the virtual parameter (other than first).
    auto ssp = a.ssp;
    auto slot = ssp++->i;

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " slot = " << slot;
    }

    auto stride = ssp++->i;

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " stride = " << stride;
    }

    // The method table contains an index in this parameter's dimension.
    auto dispatch = a.dispatch + mptr[slot].i * stride;

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << " dispatch = " << dispatch;
    }

    return next_virtual_arg{
        a.hash_table, a.hash_mult, a.hash_shift, ssp, dispatch};
}

template<typename T>
inline auto operator<<(resolve_init a, resolve_arg<1, virtual_<T>> b) {
    // Get the method table.
    auto key = virtual_traits<T>::key(b.arg);
    auto mptr = get_mptr(a.hash_table, a.hash_mult, a.hash_shift, key);

    if constexpr (bool(trace_enabled & TRACE_CALLS)) {
        call_trace << "\n  key = " << key << " mptr = " << mptr
                   << " slot = " << a.ss.i;
    }

    // Uni-methods are special, as there is no need to perform multi-dimensional
    // indexing. Instead the method's slot in the method table contains a
    // straight pointer to the function.  Just like member virtual methods,
    // except that the position of the slot floats inside the table.
    return only_virtual_arg{mptr + a.ss.i};
}

template<typename T>
struct downcast {
    template<typename U>
    static U cast(T arg) {
        return arg;
    }
};

template<class B>
struct downcast<virtual_<B>> : virtual_traits<B> {};

template<typename, auto, typename>
struct wrapper;

template<
    typename BASE_RETURN, typename... BASE_PARAM, auto SPEC,
    typename... SPEC_PARAM>
struct wrapper<BASE_RETURN(BASE_PARAM...), SPEC, types<SPEC_PARAM...>> {
    static BASE_RETURN fn(remove_virtual<BASE_PARAM>... arg) {
        return SPEC(downcast<BASE_PARAM>::template cast<SPEC_PARAM>(
            std::forward<remove_virtual<BASE_PARAM>>(arg))...);
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
        return (this_->*F)(std::forward<Args>(args)...);
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

template<typename... Classes>
struct use_classes_aux {
    using type = mp11::mp_apply<
        std::tuple,
        mp11::mp_transform_q<
            mp11::mp_bind_back<class_declaration, get_policy<Classes...>>,
            mp11::mp_apply<inheritance_map, remove_policy<Classes...>>>>;
};

template<typename... Classes, typename... ClassLists>
struct use_classes_aux<types<Classes...>, ClassLists...>
    : mp11::mp_apply<
          use_classes_aux, mp11::mp_append<types<Classes...>, ClassLists...>> {
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
