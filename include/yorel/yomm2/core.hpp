#ifndef YOREL_YOMM2_CORE_INCLUDED
#define YOREL_YOMM2_CORE_INCLUDED

#include <array>
#include <charconv>
#include <functional>
#include <limits>
#include <memory>
#include <stdio.h>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#if defined(YOMM2_SHARED)
    #if defined(_MSC_VER)
        #if !defined(yOMM2_API_msc)
            #define yOMM2_API_msc __declspec(dllimport)
        #endif
    #endif
#endif

#if !defined(yOMM2_API_gcc)
    #define yOMM2_API_gcc
#endif

#if !defined(yOMM2_API_msc)
    #define yOMM2_API_msc
#endif

#define yOMM2_API yOMM2_API_gcc yOMM2_API_msc

#include <yorel/yomm2/policy.hpp>

#pragma push_macro("max")
#undef max

namespace yorel {
namespace yomm2 {

// -----------------------------------------------------------------------------
// Error handling

struct error {};

struct resolution_error : error {
    enum status_type { no_definition = 1, ambiguous } status;
    std::string_view method_name;
    size_t arity;
    type_id* tis;
};

struct unknown_class_error : error {
    enum { update = 1, call } context;
    type_id type;
};

struct hash_search_error : error {
    size_t attempts;
    size_t buckets;
};

struct method_table_error : error {
    type_id type;
};

using error_type = std::variant<
    resolution_error, unknown_class_error, hash_search_error,
    method_table_error>;

using error_handler_type = std::function<void(const error_type& error)>;

// deprecated

struct method_call_error {
    resolution_error::status_type code;
    static constexpr auto not_implemented = resolution_error::no_definition;
    static constexpr auto ambiguous = resolution_error::ambiguous;
    std::string_view method_name;
};

using method_call_error_handler =
    void (*)(const method_call_error& error, size_t arity, type_id tis[]);

// -----------------------------------------------------------------------------
// Policies

} // namespace yomm2
} // namespace yorel

// -----------------------------------------------------------------------------
// details

namespace yorel {
namespace yomm2 {

// -----------------------------------------------------------------------------
// Scope

struct catalog {
    detail::static_chain<detail::class_info> classes;
    detail::static_chain<detail::method_info> methods;
};

// -----------------------------------------------------------------------------
// Method

template<typename A, typename B, typename C = void>
struct method;

template<typename Key, typename Signature>
struct method<Key, Signature, void> : method<default_policy, Key, Signature> {};

template<class Policy, typename Key, typename R, typename... A>
struct method<Policy, Key, R(A...)> : detail::method_info {
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

template<class Policy, typename Key, typename R, typename... A>
method<Policy, Key, R(A...)> method<Policy, Key, R(A...)>::fn;

template<class Policy, typename Key, typename R, typename... A>
typename method<Policy, Key, R(A...)>::function_pointer_type
    method<Policy, Key, R(A...)>::fake_definition;

template<class Policy, typename Key, typename R, typename... A>
template<typename Container>
typename method<Policy, Key, R(A...)>::next_type
    method<Policy, Key, R(A...)>::use_next<Container>::next;

// -----------------------------------------------------------------------------
// class_declaration

template<typename First, class... Rest>
struct class_declaration : detail::class_declaration_aux<
                               detail::get_policy<First, Rest...>,
                               detail::remove_policy<First, Rest...>> {};

template<class... First>
struct class_declaration<detail::types<First...>>
    : detail::class_declaration_aux<default_policy, detail::types<First...>> {};

template<class Policy, class... First>
struct class_declaration<Policy, detail::types<First...>>
    : detail::class_declaration_aux<Policy, detail::types<First...>> {};

template<typename First, class... Rest>
using use_classes = typename detail::use_classes_aux<
    detail::get_policy<First, Rest...>,
    detail::remove_policy<First, Rest...>>::type;

// -----------------------------------------------------------------------------
// policy

namespace policy {

template<class Key>
struct yOMM2_API_gcc yOMM2_API_msc method_tables {
    // Why is yOMM2_API_msc needed here???
    template<class Class>
    static std::uintptr_t* static_vptr;
};

template<class Key>
template<class Class>
std::uintptr_t* method_tables<Key>::static_vptr;

struct domain {};

template<class Key>
struct yOMM2_API_gcc basic_domain : domain, method_tables<Key> {
    static struct catalog catalog;
    static std::vector<std::uintptr_t> dispatch_data;
};

template<class Key>
catalog basic_domain<Key>::catalog;

template<class Key>
std::vector<std::uintptr_t> basic_domain<Key>::dispatch_data;

template<typename Policy, class Facet>
struct rebind_facet {
    using type = Facet;
};

template<
    typename NewPolicy, typename OldPolicy,
    template<typename...> class GenericFacet, typename... Args>
struct rebind_facet<NewPolicy, GenericFacet<OldPolicy, Args...>> {
    using type = GenericFacet<NewPolicy, Args...>;
};

template<class Policy, class... Facets>
struct basic_policy : virtual abstract_policy,
                      virtual basic_domain<Policy>,
                      virtual Facets... {
    using facets = detail::types<Facets...>;

    template<class Facet>
    static constexpr bool has_facet = std::is_base_of_v<Facet, Policy>;

    template<class Facet>
    using use_facet = boost::mp11::mp_first<boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_front_q<
            boost::mp11::mp_quote_trait<std::is_base_of>, Facet>,
        facets>>;

    template<class NewPolicy>
    using rebind = basic_policy<
        NewPolicy, typename rebind_facet<NewPolicy, Facets>::type...>;

    template<class Base, class Facet>
    using replace = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            boost::mp11::mp_replace_if_q<
                facets,
                boost::mp11::mp_bind_front_q<
                    boost::mp11::mp_quote_trait<std::is_base_of>, Base>,
                Facet>,
            Policy>>;

    template<class Base>
    using remove = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            boost::mp11::mp_remove_if_q<
                facets,
                boost::mp11::mp_bind_front_q<
                    boost::mp11::mp_quote_trait<std::is_base_of>, Base>>,
            Policy>>;
};

template<class Policy, class Facet>
constexpr bool has_facet = Policy::template has_facet<Facet>;

struct vptr {};
struct external_vptr : virtual vptr {};

template<class Policy>
struct yOMM2_API_gcc external_vptr_vector : virtual external_vptr {
    static std::vector<const std::uintptr_t*> vptrs;

    template<typename ForwardIterator>
    static void register_vptrs(ForwardIterator first, ForwardIterator last) {
        using namespace policy;

        size_t size;

        if constexpr (has_facet<Policy, type_hash>) {
            size = Policy::hash_type_initialize(first, last);
        } else {
            size = 1 + std::max_element(first, last, [](auto a, auto b) {
                           return a < b;
                       })->first;
        }

        vptrs.resize(size);

        if constexpr (has_facet<Policy, indirect_vptr>) {
            Policy::indirect_vptrs.resize(size);
        }

        for (auto iter = first; iter != last; ++iter) {
            auto index = iter->first;

            if constexpr (has_facet<Policy, type_hash>) {
                index = Policy::hash_type_id(index);
            }

            vptrs[index] = *iter->second;

            if constexpr (has_facet<Policy, indirect_vptr>) {
                Policy::indirect_vptrs[index] = iter->second;
            }
        }
    }

    template<class Class>
    static auto vptr(const Class& arg) {
        auto index = Policy::dynamic_type(arg);

        if constexpr (has_facet<Policy, type_hash>) {
            index = Policy::hash_type_id(index);
        }

        return vptrs[index];
    }
};

template<class Policy>
std::vector<const std::uintptr_t*> external_vptr_vector<Policy>::vptrs;

template<class Policy>
struct yOMM2_API_gcc external_vptr_map : virtual external_vptr {
    static std::unordered_map<type_id, const std::uintptr_t*> vptrs;

    static void resize_vptrs(size_t n) {
    }

    static void assign_vptr(type_id type, std::uintptr_t* vptr) {
        vptrs[type] = vptr;
    }

    template<typename ForwardIterator>
    static void register_vptrs(ForwardIterator first, ForwardIterator last) {
        for (auto iter = first; iter != last; ++iter) {
            vptrs[iter->first] = *iter->second;
        }
    }

    template<class Class>
    static auto vptr(const Class& arg) {
        return vptrs.find(Policy::dynamic_type(arg))->second;
    }
};

template<class Policy>
std::unordered_map<type_id, const std::uintptr_t*>
    external_vptr_map<Policy>::vptrs;

template<class Policy>
struct yOMM2_API_gcc basic_indirect_vptr : virtual indirect_vptr {
    static std::vector<std::uintptr_t const* const*> indirect_vptrs;
};

template<class Policy>
std::vector<std::uintptr_t const* const*>
    basic_indirect_vptr<Policy>::indirect_vptrs;

struct error_output {};

template<class Policy, typename Stream = detail::ostdstream>
struct yOMM2_API_gcc basic_error_output : virtual error_output {
    static Stream error_stream;
};

template<class Policy, typename Stream>
Stream basic_error_output<Policy, Stream>::error_stream(stderr);

struct update_output {};

template<class Policy, typename Stream = detail::ostdstream>
struct yOMM2_API_gcc basic_update_output : virtual update_output {
    static Stream update_stream;
};

template<class Policy, typename Stream>
Stream basic_update_output<Policy, Stream>::update_stream([]() {
    auto env = getenv("YOMM2_TRACE");
    return env && !(*env++ == '0' && !*env) ? stderr : nullptr;
}());

template<class Policy>
struct yOMM2_API_gcc simple_perfect_hash : virtual type_hash {
    static type_id mult;
    static std::size_t shift;
    static auto constexpr invalid = std::numeric_limits<type_id>::max();

#ifdef _MSC_VER
    __forceinline
#endif
        static type_id
        hash_type_id(type_id type) {
        return (mult * type) >> shift;
    }

    template<typename ForwardIterator>
    static size_t
    hash_type_initialize(ForwardIterator first, ForwardIterator last) {
        std::vector<type_id> buckets;
        return hash_type_initialize(first, last, buckets);
    }

    template<typename ForwardIterator>
    static size_t hash_type_initialize(
        ForwardIterator first, ForwardIterator last,
        std::vector<type_id>& buckets);
};

template<class Policy>
type_id simple_perfect_hash<Policy>::mult;

template<class Policy>
std::size_t simple_perfect_hash<Policy>::shift;

template<class Policy>
struct yOMM2_API_gcc checked_simple_perfect_hash
    : virtual simple_perfect_hash<Policy>,
      virtual runtime_checks {
    static std::vector<type_id> control;

    static type_id hash_type_id(type_id type) {
        auto index = simple_perfect_hash<Policy>::hash_type_id(type);

        if (control[index] != type) {
            using namespace policy;

            if constexpr (Policy::template has_facet<error_handler>) {
                unknown_class_error error;
                error.context = unknown_class_error::call;
                error.type = type;
                Policy::error(error);
            }

            abort();
        }

        return index;
    }

    template<typename ForwardIterator>
    static size_t
    hash_type_initialize(ForwardIterator first, ForwardIterator last) {
        return simple_perfect_hash<Policy>::hash_type_initialize(
            first, last, control);
    }
};

template<class Policy>
std::vector<type_id> checked_simple_perfect_hash<Policy>::control;

template<class Policy>
struct yOMM2_API_gcc vectored_error_handler : virtual error_handler {
    static error_handler_type error;

    static void default_error_handler(const error_type& error_v) {
        using namespace detail;
        using namespace policy;

        if constexpr (Policy::template has_facet<error_output>) {
            if (auto error = std::get_if<resolution_error>(&error_v)) {
                const char* explanation[] = {
                    "no applicable definition", "ambiguous call"};
                Policy::error_stream
                    << explanation
                           [error->status - resolution_error::no_definition]
                    << " for " << error->method_name << "(";
                auto comma = "";

                for (auto ti :
                     type_range{error->tis, error->tis + error->arity}) {
                    Policy::error_stream << comma;
                    Policy::type_name(ti, Policy::error_stream);
                    comma = ", ";
                }

                Policy::error_stream << ")\n";
            } else if (
                auto error = std::get_if<unknown_class_error>(&error_v)) {
                Policy::error_stream << "unknown class ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<method_table_error>(&error_v)) {
                Policy::error_stream << "invalid method table for ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<hash_search_error>(&error_v)) {
                Policy::error_stream << "could not find hash factors after "
                                     << error->attempts << "s using "
                                     << error->buckets << " buckets\n";
            }
        }

        abort();
    }
};

template<class Policy>
error_handler_type vectored_error_handler<Policy>::error;

template<class Policy>
struct yOMM2_API_gcc backward_compatible_error_handler
    : virtual vectored_error_handler<Policy> {
    static method_call_error_handler call_error;

    static void default_error_handler(const error_type& error_v) {
        using namespace detail;

        if (auto err = std::get_if<resolution_error>(&error_v)) {
            method_call_error old_error;
            old_error.code = err->status;
            old_error.method_name = err->method_name;
            call_error(std::move(old_error), err->arity, err->tis);
            abort();
        }

        vectored_error_handler<Policy>::default_error_handler(error_v);
    }

    static void default_call_error_handler(
        const method_call_error& error, size_t arity, type_id ti_ptrs[]) {

        using namespace policy;

        if constexpr (Policy::template has_facet<error_output>) {
            const char* explanation[] = {
                "no applicable definition", "ambiguous call"};
            Policy::error_stream
                << explanation[error.code - resolution_error::no_definition]
                << " for " << error.method_name << "(";
            auto comma = "";

            for (auto ti : detail::type_range{ti_ptrs, ti_ptrs + arity}) {
                Policy::error_stream << comma;
                Policy::type_name(ti, Policy::error_stream);
                comma = ", ";
            }

            Policy::error_stream << ")\n";
        }

        abort();
    }
};

template<class Policy>
method_call_error_handler
    backward_compatible_error_handler<Policy>::call_error =
        backward_compatible_error_handler<Policy>::default_call_error_handler;

template<class Policy, class... Facets>
struct yOMM2_API_gcc basic_static_policy
    : basic_policy<
          Policy, external_vptr_vector<Policy>, std_rtti,
          vectored_error_handler<Policy>, Facets...> {};

template<class Policy, class... Facets>
struct yOMM2_API_gcc basic_debug_static
    : basic_static_policy<
          Policy, checked_simple_perfect_hash<Policy>,
          basic_error_output<Policy>, basic_update_output<Policy>, Facets...> {
};

struct yOMM2_API_gcc debug_static
    : basic_debug_static<debug_static>::replace<
          error_handler, backward_compatible_error_handler<debug_static>> {};

template<class Policy, class... Facets>
struct basic_release_static
    : basic_static_policy<Policy, simple_perfect_hash<Policy>, Facets...> {};

struct release_static
    : basic_release_static<release_static>::replace<
          error_handler, backward_compatible_error_handler<release_static>> {};

struct debug_shared;

#if defined(_MSC_VER) && !defined(yOMM2_DLL)
extern template class __declspec(dllimport) basic_domain<debug_shared>;
extern template class __declspec(dllimport) external_vptr_vector<debug_shared>;
extern template class __declspec(dllimport)
    vectored_error_handler<debug_shared>;
extern template class __declspec(dllimport) simple_perfect_hash<debug_shared>;
extern template class __declspec(dllimport) basic_policy<
    debug_shared, external_vptr_vector<debug_shared>, std_rtti,
    checked_simple_perfect_hash<debug_shared>, basic_error_output<debug_shared>,
    basic_update_output<debug_shared>,
    backward_compatible_error_handler<debug_shared>>;
#endif

struct yOMM2_API_gcc debug_shared
    : basic_policy<
          debug_shared, external_vptr_vector<debug_shared>, std_rtti,
          checked_simple_perfect_hash<debug_shared>,
          basic_error_output<debug_shared>, basic_update_output<debug_shared>,
          backward_compatible_error_handler<debug_shared>> {};

struct yOMM2_API_gcc release_shared
    : debug_shared::replace<type_hash, simple_perfect_hash<debug_shared>> {};

} // namespace policy

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

template<class Policy, typename Key, typename R, typename... A>
method<Policy, Key, R(A...)>::method() {
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

template<class Policy, typename Key, typename R, typename... A>
method<Policy, Key, R(A...)>::~method() {
    Policy::catalog.methods.remove(*this);
}

template<class Policy, typename Key, typename R, typename... A>
typename method<Policy, Key, R(A...)>::return_type inline method<
    Policy, Key, R(A...)>::operator()(detail::remove_virtual<A>... args) const {
    using namespace detail;
    return resolve(argument_traits<Policy, A>::rarg(args)...)(
        std::forward<remove_virtual<A>>(args)...);
}

template<class Policy, typename Key, typename R, typename... A>
template<typename... ArgType>
inline typename method<Policy, Key, R(A...)>::function_pointer_type
method<Policy, Key, R(A...)>::resolve(const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (arity == 1) {
        pf = resolve_uni<types<A...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<0, types<A...>, ArgType...>(args...);
    }

    return reinterpret_cast<function_pointer_type>(pf);
}

template<class Policy, typename Key, typename R, typename... A>
template<typename ArgType>
inline const std::uintptr_t*
method<Policy, Key, R(A...)>::vptr(const ArgType& arg) const {
    if constexpr (detail::is_virtual_ptr<ArgType>) {
        return arg._vptr();
        // No need to check the method pointer: this was done when the
        // virtual_ptr was created.
    } else {
        return Policy::vptr(arg);
    }
}

template<class Policy, typename Key, typename R, typename... A>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
inline std::uintptr_t method<Policy, Key, R(A...)>::resolve_uni(
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

template<class Policy, typename Key, typename R, typename... A>
template<
    size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline std::uintptr_t method<Policy, Key, R(A...)>::resolve_multi_first(
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

template<class Policy, typename Key, typename R, typename... A>
template<
    size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
inline std::uintptr_t method<Policy, Key, R(A...)>::resolve_multi_next(
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

template<class Policy, typename Key, typename R, typename... A>
typename method<Policy, Key, R(A...)>::return_type
method<Policy, Key, R(A...)>::not_implemented_handler(
    detail::remove_virtual<A>... args) {
    if constexpr (Policy::template has_facet<policy::error_handler>) {
        resolution_error error;
        error.status = resolution_error::no_definition;
        error.method_name = fn.name;
        error.arity = arity;
        type_id tis[sizeof...(args)];
        error.tis = tis;
        auto ti_iter = tis;
        (..., (*ti_iter++ = detail::get_tip<Policy, A>(args)));
        Policy::error(error_type(std::move(error)));
    }

    abort(); // in case user handler "forgets" to abort
}

template<class Policy, typename Key, typename R, typename... A>
typename method<Policy, Key, R(A...)>::return_type
method<Policy, Key, R(A...)>::ambiguous_handler(
    detail::remove_virtual<A>... args) {
    if constexpr (Policy::template has_facet<policy::error_handler>) {
        resolution_error error;
        error.status = resolution_error::ambiguous;
        error.method_name = fn.name;
        error.arity = arity;
        type_id tis[sizeof...(args)];
        error.tis = tis;
        auto ti_iter = tis;
        (..., (*ti_iter++ = detail::get_tip<Policy, A>(args)));
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
    auto& call_error =
        policy::backward_compatible_error_handler<default_policy>::call_error;
    auto prev = call_error;
    call_error = handler;
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

#ifndef YOMM2_SHARED
    #include <yorel/yomm2/runtime.hpp>
#endif

#pragma pop_macro("max")

#endif
