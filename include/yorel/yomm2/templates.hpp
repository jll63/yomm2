// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_TEMPLATES_INCLUDED
#define YOREL_YOMM2_TEMPLATES_INCLUDED

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/mp11/list.hpp>

// clang-format off

namespace yorel { namespace yomm2 {

struct not_defined {};

template<template<typename...> typename Template>
struct template_ {
    template<typename... Ts>
    using fn = boost::mp11::mp_apply<Template, types<Ts...>>;
    // using fn = Template<Ts...> would not work because Template is not
    // necessarily variadic, but mp_apply can cope with this situation.
};

template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;

namespace detail {

// template <template <typename...> typename... F>
// using mp_list_q = boost::mp11::mp_list<boost::mp11::mp_quote<F>...>;

template<typename...>
struct product_impl;

template<typename... Ts, typename... TypeLists>
struct product_impl<types<Ts...>, TypeLists...> {
    using type = mp11::mp_product<types, types<Ts...>, TypeLists...>;
};

template<typename...>
struct apply_product_impl;

template<template<typename...> typename... Templates, typename... TypeLists>
struct apply_product_impl<templates<Templates...>, TypeLists...> {
    using type = mp11::mp_product<
        mp11::mp_invoke_q, 
        types<boost::mp11::mp_quote<Templates>...>,
        TypeLists...
    >;
};

}

template<typename... TypeLists>
using product = boost::mp11::mp_product<types, TypeLists...>;

template<typename... Lists>
using apply_product = typename detail::apply_product_impl<Lists...>::type;

template<template<typename...> typename F, typename... TypeLists>
using transform_product = boost::mp11::mp_apply<
    boost::mp11::mp_append,
    boost::mp11::mp_transform_q<
        boost::mp11::mp_bind_front<
            boost::mp11::mp_apply_q,
            boost::mp11::mp_quote<F>
        >,
        boost::mp11::mp_product<types, TypeLists...>
    >
>;

namespace detail {

namespace detail {
template<typename T>
std::true_type has_method_aux(typename T::method*);

template<typename T>
std::false_type has_method_aux(...);
}

template<typename T>
constexpr bool has_method = decltype(detail::has_method_aux<T>(nullptr))::value;

template<template<typename...> typename Definition>
struct use_definition {
    template<bool HasMethod, typename T>
    struct impl;

    template<typename T>
    struct impl<true, T> {
        using type = typename T::method::template add_definition<T>;
    };

    template<typename First, typename... Rest>
    struct impl<false, Definition<First, Rest...>> {
        using type = typename First::self_type::template add_definition<Definition<First, Rest...>>;
    };

    template<typename TypeList>
    using fn = typename impl<
        has_method<mp11::mp_apply<Definition, TypeList>>,
        mp11::mp_apply<Definition, TypeList>
    >::type;
};

template<template<typename...> typename Definition>
struct is_defined {
    template<typename TypeList>
    using fn = mp11::mp_bool<
        !std::is_base_of_v<
            not_defined,
            mp11::mp_apply<Definition, TypeList>
        >
    >;
};

template<typename... T>
struct aggregate;

// ===========================================================================

// Some compiler have problems with classes that have a large number of bases,
// so divide-and-conquer to keep the number of bases below 1024.

template<typename... T>
struct aggregate;

template<typename... T>
struct large_aggregate : std::tuple<
    mp11::mp_apply<aggregate, mp11::mp_take_c<types<T...>, sizeof...(T) / 2>>,
    mp11::mp_apply<aggregate, mp11::mp_drop_c<types<T...>, sizeof...(T) / 2>>
> {
};

template<typename... T>
struct aggregate : boost::mp11::mp_if_c<
    sizeof...(T) <= 512,
    boost::mp11::mp_defer<std::tuple, T...>,
    boost::mp11::mp_defer<large_aggregate, T...>
>::type {};

template<typename... T>
struct aggregate<types<T...>> : aggregate<T...> {};

} // namespace detail

template<template<typename...> typename Definition, typename LoL>
using use_definitions = boost::mp11::mp_apply<
    detail::aggregate,
    boost::mp11::mp_transform_q<
        detail::use_definition<Definition>,
        boost::mp11::mp_copy_if_q<
            LoL,
            detail::is_defined<Definition>
        >
    >
>;

struct yomm2_end_of_dump {};

template<typename T>
struct dump_type {
    static_assert(std::is_same_v<T, yomm2_end_of_dump>);
};

}}  // namespace yorel::yomm2

#endif
