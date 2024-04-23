#ifndef YOREL_YOMM2_POLICY_INCLUDED
#define YOREL_YOMM2_POLICY_INCLUDED

#include <array>
#include <charconv>
#include <cstdint>
#include <limits>
#include <random> // for default_random_en...
#include <stdio.h>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
#include <typeindex>
#include <typeinfo>
#endif

#pragma push_macro("max")
#undef max

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

// -----------------------------------------------------------------------------
// Forward declarations needed by "detail.hpp"

namespace yorel {
namespace yomm2 {

struct context;
struct catalog;

using type_id = std::uintptr_t;
constexpr type_id invalid_type = std::numeric_limits<type_id>::max();

template<class Class, class Policy>
struct virtual_ptr;

template<typename T>
struct virtual_;

template<class Policy, typename Key, typename Signature>
struct method;

template<class... Classes>
struct class_declaration;

// -----------------------------------------------------------------------------
// Error handling

struct error {};

struct resolution_error : error {
    enum status_type { no_definition = 1, ambiguous } status;
    std::string_view method_name;
    size_t arity;
    static constexpr size_t max_types = 16;
    type_id types[max_types];
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

struct static_offset_error : error {
    type_id method;
    int actual, expected;
};

struct static_slot_error : static_offset_error {};
struct static_stride_error : static_offset_error {};

using error_type = std::variant<
    error, resolution_error, unknown_class_error, hash_search_error,
    method_table_error, static_slot_error, static_stride_error>;

using error_handler_type = std::function<void(const error_type& error)>;

// deprecated

struct method_call_error {
    resolution_error::status_type code;
    static constexpr auto not_implemented = resolution_error::no_definition;
    static constexpr auto ambiguous = resolution_error::ambiguous;
    std::string_view method_name;
};

using method_call_error_handler =
    void (*)(const method_call_error& error, size_t arity, type_id* types);

namespace policy {

struct abstract_policy {};
struct facet {};
struct error_handler {};
struct runtime_checks {};
struct indirect_vptr {};
struct type_hash {};
struct vptr_placement {};
struct external_vptr : virtual vptr_placement {};
struct error_output {};
struct trace_output {};

struct deferred_static_rtti;
struct debug;
struct release;
struct debug_shared;
struct release_shared;

} // namespace policy

} // namespace yomm2
} // namespace yorel

#include "detail.hpp"

namespace yorel {
namespace yomm2 {

struct catalog {
    detail::static_chain<detail::class_info> classes;
    detail::static_chain<detail::method_info> methods;
};

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

struct rtti {
    static type_id type_index(type_id type) {
        return type;
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << "type_id(" << type << ")";
    }
};

struct minimal_rtti : virtual rtti {
    template<typename T>
    static type_id static_type() {
        static char id;
        return reinterpret_cast<type_id>(&id);
    }
};

struct std_rtti : virtual rtti {
#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
    template<class Class>
    static type_id static_type() {
        auto tip = &typeid(Class);
        return reinterpret_cast<type_id>(tip);
    }

    template<class Class>
    static type_id dynamic_type(const Class& obj) {
        auto tip = &typeid(obj);
        return reinterpret_cast<type_id>(tip);
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << reinterpret_cast<const std::type_info*>(type)->name();
    }

    static std::type_index type_index(type_id type) {
        return std::type_index(*reinterpret_cast<const std::type_info*>(type));
    }

    template<typename D, typename B>
    static D dynamic_cast_ref(B&& obj) {
        return dynamic_cast<D>(obj);
    }
#endif
};

struct deferred_static_rtti : virtual rtti {};

template<typename T>
constexpr bool implemented = !std::is_same_v<T, void>;

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

template<class Policy>
struct yOMM2_API_gcc vptr_vector : virtual external_vptr {
    static std::vector<const std::uintptr_t*> vptrs;

    template<typename ForwardIterator>
    static void publish_vptrs(ForwardIterator first, ForwardIterator last) {
        using namespace policy;
        using detail::pair_first_iterator;

        size_t size;

        if constexpr (has_facet<Policy, type_hash>) {
            Policy::hash_initialize(first, last);
            size = Policy::hash_length;
        } else {
            size = 0;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    size = std::max(size, *type_iter);
                }
            }

            ++size;
        }

        vptrs.resize(size);

        if constexpr (has_facet<Policy, indirect_vptr>) {
            Policy::indirect_vptrs.resize(size);
        }

        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                auto index = *type_iter;

                if constexpr (has_facet<Policy, type_hash>) {
                    index = Policy::hash_type_id(index);
                }

                vptrs[index] = iter->vptr();

                if constexpr (has_facet<Policy, indirect_vptr>) {
                    Policy::indirect_vptrs[index] = iter->indirect_vptr();
                }
            }
        }
    }

    template<class Class>
    static const std::uintptr_t* dynamic_vptr(const Class& arg) {
        auto index = Policy::dynamic_type(arg);

        if constexpr (has_facet<Policy, type_hash>) {
            index = Policy::hash_type_id(index);
        }

        return vptrs[index];
    }
};

template<class Policy>
std::vector<const std::uintptr_t*> vptr_vector<Policy>::vptrs;

template<class Policy>
struct yOMM2_API_gcc vptr_map : virtual external_vptr {
    static std::unordered_map<type_id, const std::uintptr_t*> vptrs;

    template<typename ForwardIterator>
    static void publish_vptrs(ForwardIterator first, ForwardIterator last) {
        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                vptrs[*type_iter] = iter->vptr();
            }
        }
    }

    template<class Class>
    static auto dynamic_vptr(const Class& arg) {
        return vptrs.find(Policy::dynamic_type(arg))->second;
    }
};

template<class Policy>
std::unordered_map<type_id, const std::uintptr_t*> vptr_map<Policy>::vptrs;

template<class Policy>
struct yOMM2_API_gcc basic_indirect_vptr : virtual indirect_vptr {
    static std::vector<std::uintptr_t const* const*> indirect_vptrs;
};

template<class Policy>
std::vector<std::uintptr_t const* const*>
    basic_indirect_vptr<Policy>::indirect_vptrs;

template<class Policy, typename Stream = detail::ostderr>
struct yOMM2_API_gcc basic_error_output : virtual error_output {
    static Stream error_stream;
};

template<class Policy, typename Stream>
Stream basic_error_output<Policy, Stream>::error_stream;

template<class Policy, typename Stream = detail::ostderr>
struct yOMM2_API_gcc basic_trace_output : virtual trace_output {
    static Stream trace_stream;
    static bool trace_enabled;
};

template<class Policy, typename Stream>
Stream basic_trace_output<Policy, Stream>::trace_stream;

template<class Policy, typename Stream>
bool basic_trace_output<Policy, Stream>::trace_enabled([]() {
    auto env = getenv("YOMM2_TRACE");
    return env && *env++ == '1' && *env++ == 0;
}());

template<class Policy>
struct yOMM2_API_gcc fast_perfect_hash : virtual type_hash {
    static type_id hash_mult;
    static std::size_t hash_shift;
    static std::size_t hash_length;

#ifdef _MSC_VER
    __forceinline
#endif
        static type_id
        hash_type_id(type_id type) {
        return (hash_mult * type) >> hash_shift;
    }

    template<typename ForwardIterator>
    static void hash_initialize(ForwardIterator first, ForwardIterator last) {
        std::vector<type_id> buckets;
        hash_initialize(first, last, buckets);
    }

    template<typename ForwardIterator>
    static void hash_initialize(
        ForwardIterator first, ForwardIterator last,
        std::vector<type_id>& buckets);
};

template<class Policy>
template<typename ForwardIterator>
void fast_perfect_hash<Policy>::hash_initialize(
    ForwardIterator first, ForwardIterator last,
    std::vector<type_id>& buckets) {
    using namespace policy;

    constexpr bool trace_enabled = Policy::template has_facet<trace_output>;
    const auto N = std::distance(first, last);

    if constexpr (trace_enabled) {
        if (Policy::trace_enabled) {
            Policy::trace_stream << "Finding hash factor for " << N
                                 << " types\n";
        }
    }

    std::default_random_engine rnd(13081963);
    size_t total_attempts = 0;
    size_t M = 1;

    for (auto size = N * 5 / 4; size >>= 1;) {
        ++M;
    }

    std::uniform_int_distribution<type_id> uniform_dist;

    for (size_t pass = 0; pass < 4; ++pass, ++M) {
        hash_shift = 8 * sizeof(type_id) - M;
        auto hash_size = 1 << M;

        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                Policy::trace_stream << "  trying with M = " << M << ", "
                                     << hash_size << " buckets\n";
            }
        }

        bool found = false;
        size_t attempts = 0;
        buckets.resize(hash_size);
        hash_length = 0;

        while (!found && attempts < 100000) {
            std::fill(buckets.begin(), buckets.end(), static_cast<type_id>(-1));
            ++attempts;
            ++total_attempts;
            found = true;
            hash_mult = uniform_dist(rnd) | 1;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto type = *type_iter;
                    auto index = (type * hash_mult) >> hash_shift;

                    if (index >= hash_length) {
                        hash_length = index + 1;
                    }

                    if (buckets[index] != static_cast<type_id>(-1)) {
                        found = false;
                        break;
                    }

                    buckets[index] = type;
                }
            }
        }

        // metrics.hash_search_attempts = total_attempts;
        // metrics.hash_search_time =
        //     std::chrono::steady_clock::now() - start_time;
        // metrics.hash_table_size = hash_size;

        if (found) {
            if constexpr (trace_enabled) {
                if (Policy::trace_enabled) {
                    Policy::trace_stream << "  found " << hash_mult << " after "
                                         << total_attempts << " attempts\n";
                }
            }

            return;
        }
    }

    hash_search_error error;
    error.attempts = total_attempts;
    // error.duration = std::chrono::steady_clock::now() - start_time;
    error.buckets = 1 << M;

    if constexpr (has_facet<Policy, error_handler>) {
        Policy::error(error_type(error));
    }

    abort();
}

template<class Policy>
type_id fast_perfect_hash<Policy>::hash_mult;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_shift;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_length;

template<class Policy>
struct yOMM2_API_gcc checked_perfect_hash : virtual fast_perfect_hash<Policy>,
                                            virtual runtime_checks {
    static std::vector<type_id> control;

    static type_id hash_type_id(type_id type) {
        auto index = fast_perfect_hash<Policy>::hash_type_id(type);

        if (index >= fast_perfect_hash<Policy>::hash_length ||
            control[index] != type) {
            using namespace policy;

            if constexpr (Policy::template has_facet<error_handler>) {
                unknown_class_error error;
                error.context = unknown_class_error::update;
                error.type = type;
                Policy::error(error);
            }

            abort();
        }

        return index;
    }

    template<typename ForwardIterator>
    static void hash_initialize(ForwardIterator first, ForwardIterator last) {
        fast_perfect_hash<Policy>::hash_initialize(first, last, control);
        control.resize(fast_perfect_hash<Policy>::hash_length);
    }
};

template<class Policy>
std::vector<type_id> checked_perfect_hash<Policy>::control;

#ifdef __cpp_exceptions
struct yOMM2_API_gcc throw_error : virtual error_handler {
    static void error(const error_type& error_v) {
        std::visit([](auto&& arg) { throw arg; }, error_v);
    }
};
#endif

template<class Policy, typename DefaultHandlerProvider = void>
struct yOMM2_API_gcc vectored_error : virtual error_handler {
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
                     type_range{error->types, error->types + error->arity}) {
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
    }
};

template<class Policy, typename DefaultHandlerProvider>
error_handler_type vectored_error<Policy, DefaultHandlerProvider>::error =
    std::conditional_t<
        std::is_same_v<DefaultHandlerProvider, void>, vectored_error<Policy>,
        DefaultHandlerProvider>::default_error_handler;

template<class Policy>
struct yOMM2_API_gcc backward_compatible_error_handler
    : vectored_error<Policy, backward_compatible_error_handler<Policy>> {
    static method_call_error_handler call_error;

    static void default_error_handler(const error_type& error_v) {
        using namespace detail;

        if (auto err = std::get_if<resolution_error>(&error_v)) {
            method_call_error old_error;
            old_error.code = err->status;
            old_error.method_name = err->method_name;
            call_error(std::move(old_error), err->arity, (type_id*)err->types);
            abort();
        }

        vectored_error<Policy>::default_error_handler(error_v);
    }

    static void default_call_error_handler(
        const method_call_error& error, size_t arity, type_id* ti_ptrs) {

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

struct yOMM2_API_gcc release
    : basic_policy<
          release, std_rtti, fast_perfect_hash<release>, vptr_vector<release>,
          backward_compatible_error_handler<release>> {};

struct yOMM2_API_gcc debug
    : basic_policy<
          debug, std_rtti, checked_perfect_hash<debug>, vptr_vector<debug>,
          basic_error_output<debug>, basic_trace_output<debug>,
          backward_compatible_error_handler<debug>> {};

#if defined(_MSC_VER) && !defined(yOMM2_DLL)
extern template class __declspec(dllimport) basic_domain<debug_shared>;
extern template class __declspec(dllimport) vptr_vector<debug_shared>;
extern template class __declspec(dllimport) vectored_error<
    debug_shared, backward_compatible_error_handler<debug_shared>>;
extern template class __declspec(dllimport) fast_perfect_hash<debug_shared>;
extern template class __declspec(dllimport) checked_perfect_hash<debug_shared>;
extern template class __declspec(dllimport)
    basic_trace_output<debug_shared, detail::ostderr>;
extern template class __declspec(dllimport)
    basic_error_output<debug_shared, detail::ostderr>;
extern template class __declspec(dllimport) checked_perfect_hash<debug_shared>;
extern template class __declspec(dllimport)
    backward_compatible_error_handler<debug_shared>;
extern template class __declspec(dllimport) basic_policy<
    debug_shared, vptr_vector<debug_shared>, std_rtti,
    checked_perfect_hash<debug_shared>, basic_error_output<debug_shared>,
    basic_trace_output<debug_shared>,
    backward_compatible_error_handler<debug_shared>>;
#endif

#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
struct yOMM2_API_gcc debug_shared
    : basic_policy<
          debug_shared, std_rtti, checked_perfect_hash<debug_shared>,
          vptr_vector<debug_shared>, basic_error_output<debug_shared>,
          basic_trace_output<debug_shared>,
          backward_compatible_error_handler<debug_shared>> {};

struct yOMM2_API_gcc release_shared : debug_shared {
    template<class Class>
    static const std::uintptr_t* dynamic_vptr(const Class& arg) {
        auto index = dynamic_type(arg);
        index = fast_perfect_hash<debug_shared>::hash_type_id(index);
        return vptrs[index];
    }
};
#endif

#ifdef NDEBUG
using default_static = policy::release;
#else
using default_static = policy::debug;
#endif

} // namespace policy

} // namespace yomm2
} // namespace yorel

#pragma pop_macro("max")

#endif
