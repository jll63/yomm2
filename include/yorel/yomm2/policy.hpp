#ifndef YOREL_YOMM2_POLICY_INCLUDED
#define YOREL_YOMM2_POLICY_INCLUDED

#include <cstdint>

#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
#include <typeinfo>
#include <typeindex>
#endif

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

template<typename Class, typename... Rest>
struct class_declaration;

namespace policy {

struct abstract_policy {};
struct facet {};
struct error_handler {};
struct runtime_checks {};
struct indirect_vptr {};
struct type_hash {};

struct deferred_static_rtti;
struct debug_static;
struct release_static;
struct debug_shared;
struct release_shared;

} // namespace policy

#ifdef NDEBUG
    #if defined(YOMM2_SHARED)
using default_policy = policy::release_shared;
    #else
using default_policy = policy::release_static;
    #endif
using default_static_policy = policy::release_static;
#else
    #if defined(YOMM2_SHARED)
using default_policy = policy::debug_shared;
    #else
using default_policy = policy::debug_static;
    #endif
using default_static_policy = policy::debug_static;
#endif

} // namespace yomm2
} // namespace yorel

#include "detail.hpp"

namespace yorel {
namespace yomm2 {

namespace policy {

struct rtti {
    static type_id type_index(type_id type) {
        return type;
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << "type_id(" << type << ")";
    }
};

struct final_only_rtti : virtual rtti {
    template<typename T>
    static type_id static_type() {
        static char id;
        return reinterpret_cast<type_id>(&id);
    }
};

struct std_rtti : virtual rtti {
#if defined(__GXX_RTTI) || defined(_HAS_STATIC_RTTI)
    template<typename T>
    static type_id static_type() {
        auto tip = &typeid(T);
        return reinterpret_cast<type_id>(tip);
    }

    template<typename T>
    static type_id dynamic_type(const T& obj) {
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
    static D dynamic_cast_(B&& obj) {
        return dynamic_cast<D>(obj);
    }
#endif
};

struct deferred_static_rtti : virtual rtti {};

template<typename T>
constexpr bool implemented = !std::is_same_v<T, void>;

} // namespace policy

} // namespace yomm2
} // namespace yorel

#endif
