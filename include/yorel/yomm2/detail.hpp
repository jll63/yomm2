#ifndef YOREL_YOMM2_DETAIL_HPP
#define YOREL_YOMM2_DETAIL_HPP

#include <yorel/yomm2/detail/static_list.hpp>
#include <yorel/yomm2/detail/types.hpp>

#include <boost/assert.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

template<class Policy, class TypeList>
struct type_id_list;

template<class Policy, typename... T>
struct type_id_list<Policy, types<T...>> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr std::size_t values =
        sizeof...(T) + std::is_base_of_v<policy::deferred_static_rtti, Policy>;
    static type_id value[values];
    static type_id* begin;
    static type_id* end;
};

template<class Policy, class Class>
type_id collect_static_type_id() {
    if constexpr (std::is_base_of_v<policy::deferred_static_rtti, Policy>) {
        return reinterpret_cast<type_id>(Policy::template static_type<Class>);
    } else {
        return Policy::template static_type<Class>();
    }
}

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

template<typename T>
struct is_policy_aux : std::is_base_of<policy::abstract_policy, T> {};

template<typename... T>
struct is_policy_aux<types<T...>> : std::false_type {};

template<typename T>
constexpr bool is_policy = is_policy_aux<T>::value;

template<typename T>
constexpr bool is_not_policy = !is_policy<T>;

template<class Policy, typename T>
struct virtual_traits;

template<class Policy, typename T>
struct virtual_traits<Policy, virtual_<T>> : virtual_traits<Policy, T> {};

template<class Policy, typename T>
using polymorphic_type = typename virtual_traits<Policy, T>::polymorphic_type;

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

template<typename... Classes>
using get_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<types<Classes...>>>,
    boost::mp11::mp_back<types<Classes...>>, YOMM2_DEFAULT_POLICY>;

template<typename... Classes>
using remove_policy = std::conditional_t<
    is_policy<boost::mp11::mp_back<types<Classes...>>>,
    boost::mp11::mp_pop_back<types<Classes...>>, types<Classes...>>;

template<class... Ts>
using virtual_ptr_policy = std::conditional_t<
    sizeof...(Ts) == 2, boost::mp11::mp_first<types<Ts...>>,
    YOMM2_DEFAULT_POLICY>;

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

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
