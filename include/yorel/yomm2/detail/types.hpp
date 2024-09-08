#ifndef YOREL_YOMM2_DETAIL_TYPES_HPP
#define YOREL_YOMM2_DETAIL_TYPES_HPP

#include <cstdint>
#include <limits>

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

namespace yorel {
namespace yomm2 {

using type_id = std::uintptr_t;
constexpr type_id invalid_type = (std::numeric_limits<type_id>::max)();

namespace detail {

template<typename... Types>
struct types;

template<typename Iterator>
struct range {
    range(Iterator first, Iterator last) : first(first), last(last) {
    }

    Iterator first, last;

    auto begin() const -> Iterator {
        return first;
    }

    auto end() const -> Iterator {
        return last;
    }
};

}
} // namespace yomm2
} // namespace yorel

#endif
