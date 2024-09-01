#ifndef YOREL_YOMM2_POLICIES_CORE_HPP
#define YOREL_YOMM2_POLICIES_CORE_HPP

#include <yorel/yomm2/detail/list.hpp>

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
constexpr type_id invalid_type = (std::numeric_limits<type_id>::max)();

namespace detail {

// -----------------------------------------------------------------------------
// class info

struct class_info : static_list<class_info>::static_link {
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

// -----------
// method info

struct definition_info;

struct yOMM2_API method_info : static_list<method_info>::static_link {
    std::string_view name;
    type_id *vp_begin, *vp_end;
    static_list<definition_info> specs;
    void* ambiguous;
    void* not_implemented;
    type_id method_type;
    std::size_t* slots_strides_ptr;

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

struct definition_info : static_list<definition_info>::static_link {
    ~definition_info();
    method_info* method; // for the destructor, to remove definition
    type_id type;        // of the function, for trace
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

template<class Key>
struct yOMM2_API_gcc yOMM2_API_msc method_tables {
    // Why is yOMM2_API_msc needed here???
    template<class Class>
    static std::uintptr_t* static_vptr;
};

template<class Key>
template<class Class>
std::uintptr_t* method_tables<Key>::static_vptr;

using class_catalog = detail::static_list<detail::class_info>;
using method_catalog = detail::static_list<detail::method_info>;

struct domain {};

} // namespace detail

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
    std::size_t arity;
    static constexpr std::size_t max_types = 16;
    type_id types[max_types];
};

struct unknown_class_error : error {
    enum { update = 1, call } context;
    type_id type;
};

struct hash_search_error : error {
    std::size_t attempts;
    std::size_t buckets;
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
    void (*)(const method_call_error& error, std::size_t arity, type_id* types);

namespace policy {

struct abstract_policy {};

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

template<class Key>
struct yOMM2_API_gcc basic_domain : detail::domain, detail::method_tables<Key> {
    static detail::class_catalog classes;
    static detail::method_catalog methods;
    static std::vector<std::uintptr_t> dispatch_data;
};

template<class Key>
detail::class_catalog basic_domain<Key>::classes;

template<class Key>
detail::method_catalog basic_domain<Key>::methods;

template<class Key>
std::vector<std::uintptr_t> basic_domain<Key>::dispatch_data;

} // namespace policy

} // namespace yomm2
} // namespace yorel

#endif
