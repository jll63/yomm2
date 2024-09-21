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

#define yOMM2_OPEN_BRACE {
#define yOMM2_CLOSE_BRACE }

#define yOMM2_SELECTOR(NAME) NAME##_yOMM2_selector_

// Find method given the arguments. We cannot detect if __VAR_ARGS__ is empty,
// so we cannot express the 'method<...>' type directly. Instead, we wrap
// __VAR_ARGS__ in 'types<...>' and use 'method_va_args_first' find the method.

#define yOMM2_method(NAME, ARGS, ...)                                          \
    ::yorel::yomm2::method<YOMM2_SYMBOL(NAME) ARGS, __VA_ARGS__>

#define YOMM2_DECLARE(NAME, ARGS, ...)                                         \
    yOMM2_DECLARE(yOMM2_WHEN_NOT_STATIC, NAME, ARGS, __VA_ARGS__)

#define YOMM2_STATIC_DECLARE(NAME, ARGS, ...)                                  \
    yOMM2_DECLARE(yOMM2_WHEN_STATIC, NAME, ARGS, __VA_ARGS__)

#define yOMM2_DECLARE(IF_STATIC, NAME, ARGS, ...)                              \
    struct YOMM2_SYMBOL(NAME);                                                 \
    IF_STATIC(static, )                                                        \
    yOMM2_method(NAME, ARGS, __VA_ARGS__) yOMM2_SELECTOR(NAME)(                \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS));        \
    IF_STATIC(static, )                                                        \
    inline decltype(auto) NAME(                                                \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS)) {       \
        return yOMM2_method(NAME, ARGS, __VA_ARGS__)::fn(                      \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_ALIST, ARGS));    \
    }

#define YOMM2_DEFINE(NAME, ARGS, ...)                                        \
    yOMM2_DEFINE(YOMM2_GENSYM, NAME, ARGS, __VA_ARGS__)

#define YOMM2_DEFINE_IN(CONTAINER, NAME, ARGS, ...)                     \
    yOMM2_DEFINE_IN_CONTAINER(                                                 \
        YOMM2_GENSYM, , CONTAINER, NAME, ARGS, __VA_ARGS__)

#define yOMM2_SELECT_METHOD(NAME, ARGS)                                        \
    template<typename T>                                                       \
    struct _yOMM2_select;                                                      \
    template<typename... A>                                                    \
    struct _yOMM2_select<void(A...)> {                                         \
        using type = decltype(yOMM2_SELECTOR(NAME)(std::declval<A>()...));     \
    };                                                                         \
    using _yOMM2_method = _yOMM2_select<void ARGS>::type;                      \
    using _yOMM2_return_t = _yOMM2_method::return_type;

#define yOMM2_DEFINE(NS, NAME, ARGS, ...)                                      \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(NAME, ARGS);                                           \
    _yOMM2_method::next_type next;                                             \
    struct _yOMM2_spec {                                                       \
        static boost::mp11::mp_first<boost::mp11::mp_list<__VA_ARGS__>>        \
            yOMM2_body ARGS;                                                   \
    };                                                                         \
    _yOMM2_method::override_fn<_yOMM2_spec::yOMM2_body> YOMM2_GENSYM(&next);   \
    }                                                                          \
    }                                                                          \
    boost::mp11::mp_first<boost::mp11::mp_list<__VA_ARGS__>>                   \
        NS::_yOMM2_spec::yOMM2_body ARGS

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DECLARE_METHOD_CONTAINER(...)                                    \
    BOOST_PP_OVERLOAD(YOMM2_DECLARE_METHOD_CONTAINER_, __VA_ARGS__)            \
    (__VA_ARGS__)
#else
#define YOMM2_DECLARE_METHOD_CONTAINER(...)                                    \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_DECLARE_METHOD_CONTAINER_, __VA_ARGS__)(       \
            __VA_ARGS__),                                                      \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_DECLARE_METHOD_CONTAINER_1(CONTAINER)                            \
    template<typename S>                                                       \
    struct CONTAINER

#define YOMM2_DECLARE_METHOD_CONTAINER_4(CONTAINER, NAME, ARGS, ...)           \
    template<typename S>                                                       \
    struct CONTAINER;                                                          \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                       \
        YOMM2_GENSYM, CONTAINER, NAME, ARGS, __VA_ARGS__)

#define YOMM2_DECLARE_METHOD_CONTAINER_4_NS(NS, CONTAINER, NAME, ARGS, ...)    \
    template<typename S>                                                       \
    struct CONTAINER;                                                          \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(NAME, ARGS);                                           \
    }                                                                          \
    }                                                                          \
    template<>                                                                 \
    struct CONTAINER<YOMM2_SYMBOL(NAME) ARGS> {                                \
        static NS::_yOMM2_method::next_type next;                              \
        static boost::mp11::mp_first<boost::mp11::mp_list<__VA_ARGS__>> fn     \
            ARGS;                                                              \
    }

#define YOMM2_DEFINE_INLINE(CONTAINER, NAME, ARGS, ...)                        \
    yOMM2_DEFINE_IN_CONTAINER(                                                 \
        YOMM2_GENSYM, inline, CONTAINER, NAME, ARGS, __VA_ARGS__)

#define yOMM2_DEFINE_IN_CONTAINER(NS, INLINE, CONTAINER, NAME, ARGS, ...)      \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                       \
        NS, CONTAINER, NAME, ARGS, __VA_ARGS__);                               \
    INLINE NS::_yOMM2_method::next_type                                        \
        CONTAINER<YOMM2_SYMBOL(NAME) ARGS>::next;                              \
    namespace {                                                                \
    namespace NS {                                                             \
    INLINE _yOMM2_method::override_fn<CONTAINER<YOMM2_SYMBOL(NAME) ARGS>::fn>  \
        YOMM2_GENSYM(&CONTAINER<YOMM2_SYMBOL(NAME) ARGS>::next);               \
    }                                                                          \
    }                                                                          \
    INLINE boost::mp11::mp_first<boost::mp11::mp_list<__VA_ARGS__>>            \
        CONTAINER<YOMM2_SYMBOL(NAME) ARGS>::fn ARGS

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

#define YOMM2_FRIEND_3(CONTAINER, NAME, ARGS)                                  \
    friend struct CONTAINER<YOMM2_SYMBOL(NAME) ARGS>

#define YOMM2_DEFINITION(CONTAINER, NAME, ARGS)                                \
    CONTAINER<YOMM2_SYMBOL(NAME) ARGS>::fn

#define YOMM2_CLASSES(...)                                                     \
    static ::yorel::yomm2::detail::use_classes_macro<                          \
        __VA_ARGS__, YOMM2_DEFAULT_POLICY>                                     \
        YOMM2_GENSYM;

#define YOMM2_METHOD_CLASS(NAME, ARGS, ...)                                    \
    ::yorel::yomm2::method<YOMM2_SYMBOL(NAME) ARGS, __VA_ARGS__>

#define register_classes YOMM2_CLASSES

#define declare_method YOMM2_DECLARE
#define declare_static_method YOMM2_STATIC_DECLARE
#define define_method YOMM2_DEFINE
#define define_method_inline YOMM2_DEFINE_INLINE
#define method_class YOMM2_METHOD_CLASS

#define method_container YOMM2_DECLARE_METHOD_CONTAINER
#define friend_method YOMM2_FRIEND
#define method_definition YOMM2_DEFINITION

#endif
