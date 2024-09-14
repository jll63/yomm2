#ifndef YOREL_YOMM2_CAST_HPP
#define YOREL_YOMM2_CAST_HPP

#include <type_traits>

namespace yorel {
namespace yomm2 {

namespace detail {

template<typename B, typename D, typename = void>
struct requires_dynamic_cast_ref_aux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_ref_aux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};
} // namespace detail

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
} // namespace yomm2
} // namespace yorel

#endif
