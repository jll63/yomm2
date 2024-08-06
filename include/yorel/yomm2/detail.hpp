#ifndef YOREL_YOMM2_DETAIL_INCLUDED
#define YOREL_YOMM2_DETAIL_INCLUDED

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/dynamic_bitset.hpp>

#include "yorel/yomm2/detail/chain.hpp"

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
struct range {
    range(Iterator first, Iterator last) : first(first), last(last) {
    }

    Iterator first, last;

    Iterator begin() const {
        return first;
    }

    Iterator end() const {
        return last;
    }
};

template<typename Iterator>
range(Iterator b, Iterator e) -> range<Iterator>;

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

template<typename T>
const char* default_method_name() {
#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
    return typeid(T).name();
#else
    return "method";
#endif
}

// -----------------------------------------------------------------------------
// iterator adapter for passing range from external_vpt to fast_perfect_hash

template<class PairIterator>
class pair_first_iterator {
    PairIterator iter;

  public:
    using iterator_category = typename std::forward_iterator_tag;
    using difference_type = typename PairIterator::difference_type;
    using value_type = decltype(std::declval<PairIterator>()->first);
    using pointer = const value_type*;
    using reference = const value_type&;

    explicit pair_first_iterator(PairIterator iter) : iter(iter) {
    }

    reference operator*() const {
        return iter->first;
    }

    pointer operator->() const {
        return &iter->first;
    }

    pair_first_iterator& operator++() {
        ++iter;
        return *this;
    }

    pair_first_iterator operator++(int) const {
        return pair_first_iterator(iter++);
    }

    friend bool
    operator==(const pair_first_iterator& a, const pair_first_iterator& b) {
        return a.iter == b.iter;
    }

    friend bool
    operator!=(const pair_first_iterator& a, const pair_first_iterator& b) {
        return a.iter != b.iter;
    }
};

// -----------------------------------------------------------------------------
// class info

struct class_info : static_chain<class_info>::static_link {
    type_id type;
    std::uintptr_t** static_vptr;
    type_id *first_base, *last_base;
    bool is_abstract{false};

    const std::uintptr_t* vptr() const {
        return *static_vptr;
    }

    const std::uintptr_t* const* indirect_vptr() const {
        return static_vptr;
    }

    auto type_id_begin() const {
        return &type;
    }

    auto type_id_end() const {
        return &type + 1;
    }
};

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, detail::types<Class, Bases...>>
    : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Policy, Class>();
        this->first_base = type_id_list<Policy, types<Bases...>>::begin;
        this->last_base = type_id_list<Policy, types<Bases...>>::end;
        Policy::classes.push_front(*this);
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

struct definition_info : static_chain<definition_info>::static_link {
    ~definition_info();
    method_info* method; // todo: is this used?
    type_id type;        // of the function, for trace
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

template<typename... Ts>
constexpr auto arity =
    boost::mp11::mp_count_if<types<Ts...>, is_virtual>::value;

struct yOMM2_API method_info : static_chain<method_info>::static_link {
    std::string_view name;
    type_id *vp_begin, *vp_end;
    static_chain<definition_info> specs;
    void* ambiguous;
    void* not_implemented;
    type_id method_type;
    size_t* slots_strides_ptr;

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
struct requires_dynamic_cast_refaux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_refaux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};

template<class B, class D>
constexpr bool requires_dynamic_cast =
    requires_dynamic_cast_refaux<B, D>::value;

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
    sizeof...(Ts) == 2, boost::mp11::mp_second<detail::types<Ts..., void>>,
    boost::mp11::mp_first<detail::types<Ts...>>>;

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
        return Policy::dynamic_type(virtual_traits<Policy, ArgType>::rarg(arg));
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

std::ostream* log_on(std::ostream* os);
std::ostream* log_off();

// -----------------------------------------------------------------------------
// tracing

struct rflush {
    size_t width;
    size_t value;
    explicit rflush(size_t width, size_t value) : width(width), value(value) {
    }
};

struct type_name {
    type_name(type_id type) : type(type) {
    }
    type_id type;
};

template<class Policy>
struct trace_type {
    static constexpr bool trace_enabled =
        Policy::template has_facet<policy::trace_output>;

    size_t indentation_level{0};

    trace_type& operator++() {
        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                for (int i = 0; i < indentation_level; ++i) {
                    Policy::trace_stream << "  ";
                }
            }
        }

        return *this;
    }

    struct indent {
        trace_type& trace;
        int by;

        explicit indent(trace_type& trace, int by = 2) : trace(trace), by(by) {
            trace.indentation_level += by;
        }

        ~indent() {
            trace.indentation_level -= by;
        }
    };
};

template<class Policy, typename T, typename F>
auto& write_range(trace_type<Policy>& trace, detail::range<T> range, F fn) {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            trace << "(";
            const char* sep = "";
            for (auto value : range) {
                trace << sep << fn(value);
                sep = ", ";
            }

            trace << ")";
        }
    }

    return trace;
}

template<class Policy, typename T>
auto& operator<<(trace_type<Policy>& trace, const T& value) {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            Policy::trace_stream << value;
        }
    }
    return trace;
}

template<class Policy>
auto& operator<<(trace_type<Policy>& trace, const rflush& rf) {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            auto pad = rf.width;
            auto remain = rf.value;

            int digits = 1;
            auto tmp = rf.value / 10;

            while (tmp) {
                ++digits;
                tmp /= 10;
            }

            while (digits < rf.width) {
                trace << " ";
                ++digits;
            }

            trace << rf.value;
        }
    }

    return trace;
}

template<class Policy>
auto& operator<<(trace_type<Policy>& trace, const boost::dynamic_bitset<>& bits) {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            if (Policy::trace_enabled) {
                auto i = bits.size();
                while (i != 0) {
                    --i;
                    Policy::trace_stream << bits[i];
                }
            }
        }
    }

    return trace;
}

template<class Policy>
auto& operator<<(trace_type<Policy>& trace, const detail::range<type_id*>& tips) {
    return write_range(trace, tips, [](auto tip) { return type_name(tip); });
}

template<class Policy, typename T>
auto& operator<<(trace_type<Policy>& trace, const detail::range<T>& range) {
    return write_range(trace, range, [](auto value) { return value; });
}

// -----------------------------------------------------------------------------
// lightweight ostream

struct ostdstream {
    FILE* stream = nullptr;

    ostdstream(FILE* stream = nullptr) : stream(stream) {
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

struct ostderr : ostdstream {
    ostderr() : ostdstream(stderr) {
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

inline ostdstream& operator<<(ostdstream& os, const void* value) {
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
// encode/decode dispatch data

constexpr size_t method_bits = sizeof(uint16_t) * 4;
constexpr std::uintptr_t spec_mask = (1 << method_bits) - 1;
constexpr std::uintptr_t stop_bit = 1 << (sizeof(uint16_t) * 8 - 1);
constexpr std::uintptr_t index_bit = 1 << (sizeof(uint16_t) * 8 - 2);

template<class Policy, typename Data>
void decode_dispatch_data(Data& init) {
    using namespace yorel::yomm2::detail;

    constexpr auto pointer_size = sizeof(std::uintptr_t);

    trace_type<Policy> trace;
    using indent = typename trace_type<Policy>::indent;

    trace << "Decoding dispatch data for ";
    Policy::type_name(Policy::template static_type<Policy>(), trace);
    trace << "\n";

    auto method_count = 0, multi_method_count = 0;

    for (auto& method : Policy::methods) {
        ++method_count;

        if (method.arity() >= 2) {
            ++multi_method_count;
        }
    }

    ++trace << method_count << " methods, " << multi_method_count
            << " multi-methods\n";

    // First copy the slots and strides to the static arrays in methods. Also
    // build an array of arrays of pointer to method definitions. Methods and
    // definitions are in reverse order, because of how 'chain' works. While
    // building the array of array of defintions, we put them back in the order
    // in which the compiler saw them.
    auto packed_slots_iter = init.packed.slots;
    auto methods = (method_info**)alloca(method_count * pointer_size);
    auto methods_iter = methods;
    auto method_defs = (uintptr_t**)alloca(method_count * pointer_size);
    auto method_defs_iter = method_defs;
    auto dispatch_tables =
        (std::uintptr_t**)alloca(method_count * pointer_size);
    auto multi_method_to_method =
        (size_t*)alloca(multi_method_count * sizeof(size_t));
    auto multi_method_to_method_iter = multi_method_to_method;
    auto method_index = 0;

    for (auto& method : Policy::methods) {
        ++trace << "method " << method.name << "\n";
        indent _(trace);

        *methods_iter++ = &method;

        ++trace << "specializations:\n";

        for (auto& spec : method.specs) {
            indent _(trace);
            ++trace << spec.pf << " ";
            Policy::type_name(spec.type, trace);
            trace << "\n";
        }

        auto slots_strides_count = 2 * method.arity() - 1;

        // copy slots and strides into the method's static
        ++trace << "installing " << slots_strides_count
                << " slots and strides\n";
        std::copy_n(
            packed_slots_iter, slots_strides_count, method.slots_strides_ptr);
        packed_slots_iter += slots_strides_count;

        auto specs =
            (uintptr_t*)alloca((method.specs.size() + 2) * pointer_size);
        *method_defs_iter++ = specs;
        ++trace << "specs index: " << specs << "\n";
        specs = std::transform(
            method.specs.begin(), method.specs.end(), specs,
            [](auto& spec) { return (uintptr_t)spec.pf; });
        *specs++ = (uintptr_t)method.not_implemented;
        *specs++ = (uintptr_t)method.ambiguous;

        if (method.arity() >= 2) {
            ++trace << "m-method "
                    << (multi_method_to_method_iter - multi_method_to_method)
                    << " is method " << method_index << "\n";
            *multi_method_to_method_iter++ = method_index;
        }

        ++method_index;
    }

    // Build a table of pointers to dispatch tables, for multi-methods only.

    // decode the dispatch tables for multi-methods, and keep track of them in
    // an array. We will use it when we fill the vtables. The packed dispatch
    // tables are in compiler order.

    auto packed_iter = init.packed.dispatch;
    auto decode_iter = init.decode;

    ++trace << "decoding multi-method dispatch tables\n";

    for (auto i = 0; i < multi_method_count; ++i) {
        assert((char*)decode_iter < (char*)packed_iter);
        indent _(trace);

        dispatch_tables[i] = decode_iter;
        ++trace << "multi-method " << i << " dispatch table at " << decode_iter
                << "\n";

        indent __(trace);
        ++trace << "specs:";

        auto defs = method_defs[multi_method_to_method[i]];

        do {
            auto spec_index = *packed_iter & ~stop_bit;
            trace << " " << spec_index;
            *decode_iter++ = defs[spec_index];
        } while (!(*packed_iter++ & stop_bit));

        trace << "\n";
    }

    ++trace << "decoding v-tables\n";
    auto vtbl_iter = init.packed.vtbl;

    for (auto& cls : Policy::classes) {
        if (*cls.static_vptr != nullptr) {
            continue;
        }

        indent _1(trace);
        ++trace << "class ";
        Policy::type_name(cls.type, trace);
        trace << "\n";

        indent _2(trace);

        auto first_slot = *vtbl_iter++;
        ++trace << "first slot: " << first_slot << "\n";

        *cls.static_vptr = decode_iter - first_slot;

        do {
            auto entry = *vtbl_iter & ~stop_bit;

            if (entry & index_bit) {
                auto index = *vtbl_iter & ~index_bit;
                ++trace << "multi-method index " << index << "\n";
                *decode_iter++ = index;
            } else {
                auto method_index = entry;
                auto group_index = *++vtbl_iter & ~stop_bit;
                auto method = methods[method_index];

                if (method->arity() == 1) {
                    ++trace << "uni-method " << method_index << " group "
                            << group_index;
                    *decode_iter++ = method_defs[method_index][group_index];
                } else {
                    ++trace << "multi-method " << method_index << " group "
                            << group_index;
                    *decode_iter++ = (std::uintptr_t)(
                        dispatch_tables[method_index] + group_index);
                }

                trace << " ";
                Policy::type_name(method->method_type, trace);
                trace << "\n";
            }
        } while (!(*vtbl_iter++ & stop_bit));
    }

    using namespace policy;

    if constexpr (Policy::template has_facet<policy::external_vptr>) {
        Policy::publish_vptrs(Policy::classes.begin(), Policy::classes.end());
    }
}

// -----------------------------------------------------------------------------
// report

struct update_method_report {
    size_t cells = 0;
    size_t concrete_cells = 0;
    size_t not_implemented = 0;
    size_t concrete_not_implemented = 0;
    size_t ambiguous = 0;
    size_t concrete_ambiguous = 0;
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
