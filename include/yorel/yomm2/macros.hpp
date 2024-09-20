// Copyright (c) 2018-2024 Jean-Louis Leroy

#ifndef YOREL_YOMM2_MACROS_HPP
#define YOREL_YOMM2_MACROS_HPP

#include <boost/preprocessor/config/config.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/detail/auto_rec.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>

#include <yorel/yomm2/symbols.hpp>

#define yOMM2_PLIST(N, I, A)                                                   \
    BOOST_PP_COMMA_IF(I)                                                       \
    ::yorel::yomm2::detail::remove_virtual<BOOST_PP_TUPLE_ELEM(I, A)>          \
    BOOST_PP_CAT(a, I)

#define yOMM2_ALIST(N, I, A)                                                   \
    BOOST_PP_COMMA_IF(I)                                                       \
    std::forward<                                                              \
        ::yorel::yomm2::detail::remove_virtual<BOOST_PP_TUPLE_ELEM(I, A)>>(    \
        BOOST_PP_CAT(a, I))

#define yOMM2_RLIST(N, I, A)                                                   \
    BOOST_PP_COMMA_IF(I)                                                       \
    BOOST_PP_CAT(a, I)

#define yOMM2_WHEN_STATIC(CODE1, CODE2) CODE1
#define yOMM2_WHEN_NOT_STATIC(CODE1, CODE2) CODE2

// Find method given the arguments. We cannot detect if __VAR_ARGS__ is empty,
// so we cannot express the 'method<...>' type directly. Instead, we wrap
// __VAR_ARGS__ in 'types<...>' and use 'method_macro_aux' find the method.

#define YOREL_YOMM2_DETAIL_METHOD(RETURN_TYPE, NAME, ARGS, ...)                \
    ::yorel::yomm2::detail::method_macro_aux<                                  \
        NAME##_method, RETURN_TYPE ARGS,                                       \
        ::yorel::yomm2::detail::types<__VA_ARGS__>>::type

#define YOMM2_DECLARE(RETURN_TYPE, NAME, ARGS, ...)                            \
    yOMM2_DECLARE(                                                             \
        YOMM2_GENSYM, yOMM2_WHEN_NOT_STATIC, RETURN_TYPE, NAME, ARGS,          \
        __VA_ARGS__)

#define YOMM2_STATIC_DECLARE(RETURN_TYPE, NAME, ARGS, ...)                     \
    yOMM2_DECLARE(yOMM2_WHEN_STATIC, RETURN_TYPE, NAME, ARGS, __VA_ARGS__)

#define yOMM2_DECLARE(GENSYM, IF_STATIC, RETURN_TYPE, NAME, ARGS, ...)         \
    struct NAME##_method;                                                      \
    using GENSYM =                                                             \
        YOREL_YOMM2_DETAIL_METHOD(RETURN_TYPE, NAME, ARGS, __VA_ARGS__);       \
    IF_STATIC(static, )                                                        \
    auto NAME##_method_guide(                                                  \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS))         \
        ->GENSYM;                                                              \
    IF_STATIC(static, )                                                        \
    BOOST_FORCEINLINE RETURN_TYPE NAME(                                        \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS)) {       \
        return GENSYM::fn(                                                     \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_ALIST, ARGS));    \
    }                                                                          \
    template<typename MethodSignature, typename OverriderSignature>            \
    struct NAME##_overriders;                                                  \
    template<typename OverriderReturnType, typename... OverriderParameters>    \
    struct NAME##_overriders<                                                  \
        NAME##_method ARGS, OverriderReturnType(OverriderParameters...)> {     \
        static typename GENSYM::next_type next;                                \
        static OverriderReturnType fn(OverriderParameters...);                 \
    };

#define YOMM2_DEFINE_X_(NS, PREFIX, RETURN_TYPE, NAME, ARGS, SUFFIX)           \
    template<typename...>                                                      \
    struct NS;                                                                 \
    template<typename... Parameters>                                           \
    struct NS<void(Parameters...)> {                                           \
        using type =                                                           \
            decltype(NAME##_method_guide(std::declval<Parameters>()...));      \
    };                                                                         \
    template<>                                                                 \
    auto NAME##_overriders<NS<void ARGS>::type::this_type, RETURN_TYPE ARGS>:: \
        fn ARGS->RETURN_TYPE;                                                  \
    YOMM2_STATIC(                                                              \
        NS<void ARGS>::type::override<                                         \
            NAME##_overriders<NS<void ARGS>::type, RETURN_TYPE ARGS>>);        \
    template<>                                                                 \
    auto NAME##_overriders<NS<void ARGS>::type::this_type, RETURN_TYPE ARGS>:: \
        fn ARGS->RETURN_TYPE

#define YOMM2_DEFINE_X(PREFIX, RETURN_TYPE, NAME, ARGS, SUFFIX)                \
    YOMM2_DEFINE_X_(YOMM2_GENSYM, PREFIX, RETURN_TYPE, NAME, ARGS, SUFFIX)

#define YOMM2_DEFINE(RETURN_TYPE, NAME, ARGS)                                  \
    YOMM2_DEFINE_X(, RETURN_TYPE, NAME, ARGS, )

#define YOMM2_DEFINE_INLINE(CONTAINER, RETURN_TYPE, NAME, ARGS)                \
    yOMM2_DEFINE_IN_CONTAINER(INLINE, CONTAINER, RETURN_TYPE, NAME, ARGS)

#define yOMM2_DEFINE_IN_CONTAINER(INLINE, CONTAINER, RETURN_TYPE, NAME, ARGS)  \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(CONTAINER, RETURN_TYPE, NAME, ARGS);   \
    INLINE _YOREL_YOMM2_DETAIL_METHOD::next_type                               \
        CONTAINER<RETURN_TYPE ARGS>::next;                                     \
    INLINE _YOREL_YOMM2_DETAIL_METHOD::override_fn<                            \
        CONTAINER<RETURN_TYPE ARGS>::fn>                                       \
        YOMM2_GENSYM(&CONTAINER<RETURN_TYPE ARGS>::next);                      \
    INLINE _YOREL_YOMM2_DETAIL_METHOD::return_type                             \
        CONTAINER<RETURN_TYPE ARGS>::fn ARGS

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_FRIEND(...)                                                      \
    BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_FRIEND(...)                                                      \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)(__VA_ARGS__),            \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_FRIEND_1(CONTAINER)                                              \
    template<typename>                                                         \
    friend struct CONTAINER

#define YOMM2_FRIEND_3(CONTAINER, RETURN_TYPE, ARGS)                           \
    friend struct CONTAINER<RETURN_TYPE ARGS>

#define YOMM2_DEFINITION(CONTAINER, RETURN_TYPE, ARGS)                         \
    CONTAINER<RETURN_TYPE ARGS>::fn

#define YOMM2_CLASSES(...)                                                     \
    static ::yorel::yomm2::detail::use_classes_macro<                          \
        __VA_ARGS__, YOMM2_DEFAULT_POLICY>                                     \
        YOMM2_GENSYM;

// #define YOREL_YOMM2_DETAIL_METHOD_CLASS(RETURN_TYPE, NAME, ...)                \
//     ::yorel::yomm2::method<NAME##_method, RETURN_TYPE __VA_ARGS__>

#define register_classes YOMM2_CLASSES

#define declare_method YOMM2_DECLARE
#define declare_static_method YOMM2_STATIC_DECLARE
#define define_method YOMM2_DEFINE
#define define_method_INLINE YOMM2_DEFINE_INLINE
//#define method_class YOREL_YOMM2_DETAIL_METHOD_CLASS

#define method_container YOMM2_DECLARE_METHOD_CONTAINER
#define friend_method YOMM2_FRIEND
#define method_definition YOMM2_DEFINITION

#endif
