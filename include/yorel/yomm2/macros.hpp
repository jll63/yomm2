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

#define yOMM2_SELECTOR(ID) ID##_yOMM2_selector_

#define yOMM2_method(ReturnType, ID, ARGS, POLICY)                                      \
    ::yorel::yomm2::method<YOMM2_SYMBOL(ID), ReturnType ARGS, POLICY>

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DECLARE(...)                                                     \
    BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__)                             \
    (__VA_ARGS__)
#define YOMM2_STATIC_DECLARE(...)                                              \
    BOOST_PP_OVERLOAD(YOMM2_STATIC_DECLARE_, __VA_ARGS__)                      \
    (__VA_ARGS__)
#else
#define YOMM2_DECLARE(...)                                                     \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__)(__VA_ARGS__),           \
        BOOST_PP_EMPTY())
#define YOMM2_STATIC_DECLARE(...)                                              \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_STATIC_DECLARE_, __VA_ARGS__)(__VA_ARGS__),    \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_DECLARE_3(ReturnType, ID, ARGS)                                           \
    yOMM2_DECLARE(ReturnType, ID, ARGS, YOMM2_DEFAULT_POLICY, yOMM2_WHEN_NOT_STATIC)

#define YOMM2_DECLARE_4(ReturnType, ID, ARGS, POLICY)                                   \
    yOMM2_DECLARE(ReturnType, ID, ARGS, POLICY, yOMM2_WHEN_NOT_STATIC)

#define YOMM2_STATIC_DECLARE_3(ReturnType, ID, ARGS)                                    \
    yOMM2_DECLARE(ReturnType, ID, ARGS, YOMM2_DEFAULT_POLICY, yOMM2_WHEN_STATIC)

#define YOMM2_STATIC_DECLARE_4(ReturnType, ID, ARGS, POLICY)                            \
    yOMM2_DECLARE(ReturnType, ID, ARGS, POLICY, yOMM2_WHEN_STATIC)

#define yOMM2_DECLARE(ReturnType, ID, ARGS, POLICY, IF_STATIC)                          \
    struct YOMM2_SYMBOL(ID);                                                   \
    IF_STATIC(static, )                                                        \
    yOMM2_method(ReturnType, ID, ARGS, POLICY) yOMM2_SELECTOR(ID)(                      \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS));        \
    IF_STATIC(static, )                                                        \
    inline const char* yOMM2_SELECTOR(ID)(                                     \
        const yOMM2_method(ReturnType, ID, ARGS, POLICY)&) {                            \
        return #ReturnType " " #ID #ARGS;                                               \
    }                                                                          \
    IF_STATIC(static, )                                                        \
    inline ReturnType ID(                                                               \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS)) {       \
        return yOMM2_method(ReturnType, ID, ARGS, POLICY)::fn(                          \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_ALIST, ARGS));    \
    }

#if defined(YOMM2_ENABLE_TRACE) && (YOMM2_ENABLE_TRACE & 1) || !defined(NDEBUG)
#define yOMM2_NAME(FROM_TYPEID, HUMAN_READABLE) HUMAN_READABLE
#else
#define yOMM2_NAME(FROM_TYPEID, HUMAN_READABLE) FROM_TYPEID
#endif

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DEFINE(...)                                                      \
    BOOST_PP_OVERLOAD(YOMM2_DEFINE_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_DEFINE(...)                                                      \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_DEFINE_, __VA_ARGS__)(__VA_ARGS__),            \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_DEFINE_3(RETURN_T, ID, ARGS)                                     \
    yOMM2_DEFINE(YOMM2_GENSYM, RETURN_T, ID, ARGS)

#define YOMM2_DEFINE_4(CONTAINER, RETURN_T, ID, ARGS)                          \
    yOMM2_DEFINE_IN_CONTAINER(YOMM2_GENSYM, , CONTAINER, RETURN_T, ID, ARGS)

#define yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS)                                \
    template<typename T>                                                       \
    struct _yOMM2_select;                                                      \
    template<typename... A>                                                    \
    struct _yOMM2_select<void(A...)> {                                         \
        using type = decltype(yOMM2_SELECTOR(ID)(std::declval<A>()...));       \
    };                                                                         \
    using _yOMM2_method = _yOMM2_select<void ARGS>::type;                      \
    using _yOMM2_return_t = _yOMM2_method::return_type;

#define yOMM2_DEFINE(NS, RETURN_T, ID, ARGS)                                   \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS);                                   \
    _yOMM2_method::function_pointer_type next;                                 \
    struct _yOMM2_spec {                                                       \
        static NS::_yOMM2_method::return_type yOMM2_body ARGS;                 \
    };                                                                         \
    _yOMM2_method::add_function<_yOMM2_spec::yOMM2_body> YOMM2_GENSYM(&next);  \
    }                                                                          \
    }                                                                          \
    NS::_yOMM2_method::return_type NS::_yOMM2_spec::yOMM2_body ARGS

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

#define YOMM2_DECLARE_METHOD_CONTAINER_4(CONTAINER, RETURN_T, ID, ARGS)        \
    template<typename S>                                                       \
    struct CONTAINER;                                                          \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                       \
        YOMM2_GENSYM, CONTAINER, RETURN_T, ID, ARGS)

#define YOMM2_DECLARE_METHOD_CONTAINER_4_NS(NS, CONTAINER, RETURN_T, ID, ARGS) \
    template<typename S>                                                       \
    struct CONTAINER;                                                          \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS);                                   \
    }                                                                          \
    }                                                                          \
    template<>                                                                 \
    struct CONTAINER<RETURN_T ARGS> {                                          \
        static NS::_yOMM2_method::function_pointer_type next;                  \
        static RETURN_T fn ARGS;                                               \
    }

#define YOMM2_DEFINE_INLINE(CONTAINER, RETURN_T, ID, ARGS)                     \
    yOMM2_DEFINE_IN_CONTAINER(                                                 \
        YOMM2_GENSYM, inline, CONTAINER, RETURN_T, ID, ARGS)

#define yOMM2_DEFINE_IN_CONTAINER(NS, INLINE, CONTAINER, RETURN_T, ID, ARGS)   \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(NS, CONTAINER, RETURN_T, ID, ARGS);    \
    INLINE NS::_yOMM2_method::function_pointer_type                            \
        CONTAINER<RETURN_T ARGS>::next;                                        \
    namespace {                                                                \
    namespace NS {                                                             \
    INLINE _yOMM2_method::add_function<CONTAINER<RETURN_T ARGS>::fn>           \
        YOMM2_GENSYM(&CONTAINER<RETURN_T ARGS>::next);                         \
    }                                                                          \
    }                                                                          \
    INLINE NS::_yOMM2_method::return_type CONTAINER<RETURN_T ARGS>::fn ARGS

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

#define YOMM2_FRIEND_3(CONTAINER, RETURN_T, ARGS)                              \
    friend struct CONTAINER<RETURN_T ARGS>

#define YOMM2_DEFINITION(CONTAINER, RETURN_T, ARGS) CONTAINER<RETURN_T ARGS>::fn

#define YOMM2_CLASS(...)                                                       \
    static ::yorel::yomm2::class_declaration<__VA_ARGS__> YOMM2_GENSYM;

#define YOMM2_CLASSES(...)                                                     \
    static ::yorel::yomm2::detail::use_classes_macro<                          \
        __VA_ARGS__, YOMM2_DEFAULT_POLICY>                                     \
        YOMM2_GENSYM;

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_METHOD_CLASS(...)                                                \
    BOOST_PP_OVERLOAD(YOMM2_METHOD_CLASS_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_METHOD_CLASS(...)                                                \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_METHOD_CLASS_, __VA_ARGS__)(__VA_ARGS__),      \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_METHOD_CLASS_3(RETURN_TYPE, NAME, ARGS)                          \
    ::yorel::yomm2::method<YOMM2_SYMBOL(NAME), RETURN_TYPE ARGS>

#define YOMM2_METHOD_CLASS_4(RETURN_TYPE, NAME, ARGS, POLICY)                  \
    ::yorel::yomm2::method<YOMM2_SYMBOL(NAME), RETURN_TYPE ARGS, POLICY>

#endif
