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

#define yOMM2_SELECTOR(Name) Name##_yOMM2_selector_

#define yOMM2_method(ReturnType, Name, Args, Policy)                           \
    ::yorel::yomm2::method<YOMM2_SYMBOL(Name), ReturnType Args, Policy>

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

#define YOMM2_DECLARE_3(ReturnType, Name, Args)                                \
    yOMM2_DECLARE(                                                             \
        ReturnType, Name, Args, YOMM2_DEFAULT_POLICY, yOMM2_WHEN_NOT_STATIC)

#define YOMM2_DECLARE_4(ReturnType, Name, Args, Policy)                        \
    yOMM2_DECLARE(ReturnType, Name, Args, Policy, yOMM2_WHEN_NOT_STATIC)

#define YOMM2_STATIC_DECLARE_3(ReturnType, Name, Args)                         \
    yOMM2_DECLARE(                                                             \
        ReturnType, Name, Args, YOMM2_DEFAULT_POLICY, yOMM2_WHEN_STATIC)

#define YOMM2_STATIC_DECLARE_4(ReturnType, Name, Args, Policy)                 \
    yOMM2_DECLARE(ReturnType, Name, Args, Policy, yOMM2_WHEN_STATIC)

#define yOMM2_DECLARE(ReturnType, Name, Args, Policy, IF_STATIC)               \
    struct YOMM2_SYMBOL(Name);                                                 \
    IF_STATIC(static, )                                                        \
    yOMM2_method(ReturnType, Name, Args, Policy) yOMM2_SELECTOR(Name)(         \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(Args), yOMM2_PLIST, Args));        \
    IF_STATIC(static, )                                                        \
    inline const char* yOMM2_SELECTOR(Name)(                                   \
        const yOMM2_method(ReturnType, Name, Args, Policy)&) {                 \
        return #ReturnType " " #Name #Args;                                    \
    }                                                                          \
    IF_STATIC(static, )                                                        \
    inline ReturnType Name(                                                    \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(Args), yOMM2_PLIST, Args)) {       \
        return yOMM2_method(ReturnType, Name, Args, Policy)::fn(               \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(Args), yOMM2_ALIST, Args));    \
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

#define YOMM2_DEFINE_3(ReturnType, Name, Args)                                 \
    yOMM2_DEFINE(YOMM2_GENSYM, ReturnType, Name, Args)

#define YOMM2_DEFINE_4(Container, ReturnType, Name, Args)                      \
    yOMM2_DEFINE_IN_CONTAINER(YOMM2_GENSYM, , Container, ReturnType, Name, Args)

#define yOMM2_SELECT_METHOD(ReturnType, Name, Args)                            \
    template<typename T>                                                       \
    struct _yOMM2_select;                                                      \
    template<typename... A>                                                    \
    struct _yOMM2_select<void(A...)> {                                         \
        using type = decltype(yOMM2_SELECTOR(Name)(std::declval<A>()...));     \
    };                                                                         \
    using _yOMM2_method = _yOMM2_select<void Args>::type;                      \
    using _yOMM2_return_t = _yOMM2_method::return_type;

#define yOMM2_DEFINE(NS, ReturnType, Name, Args)                               \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(ReturnType, Name, Args);                               \
    _yOMM2_method::function_pointer_type next;                                 \
    struct _yOMM2_spec {                                                       \
        static NS::_yOMM2_method::return_type yOMM2_body Args;                 \
    };                                                                         \
    _yOMM2_method::add_function<_yOMM2_spec::yOMM2_body> YOMM2_GENSYM(&next);  \
    }                                                                          \
    }                                                                          \
    NS::_yOMM2_method::return_type NS::_yOMM2_spec::yOMM2_body Args

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

#define YOMM2_DECLARE_METHOD_CONTAINER_1(Container)                            \
    template<typename S>                                                       \
    struct Container

#define YOMM2_DECLARE_METHOD_CONTAINER_4(Container, ReturnType, Name, Args)    \
    template<typename S>                                                       \
    struct Container;                                                          \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                       \
        YOMM2_GENSYM, Container, ReturnType, Name, Args)

#define YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                   \
    NS, Container, ReturnType, Name, Args)                                     \
    template<typename S>                                                       \
    struct Container;                                                          \
    namespace {                                                                \
    namespace NS {                                                             \
    yOMM2_SELECT_METHOD(ReturnType, Name, Args);                               \
    }                                                                          \
    }                                                                          \
    template<>                                                                 \
    struct Container<ReturnType Args> {                                        \
        static NS::_yOMM2_method::function_pointer_type next;                  \
        static ReturnType fn Args;                                             \
    }

#define YOMM2_DEFINE_INLINE(Container, ReturnType, Name, Args)                 \
    yOMM2_DEFINE_IN_CONTAINER(                                                 \
        YOMM2_GENSYM, inline, Container, ReturnType, Name, Args)

#define yOMM2_DEFINE_IN_CONTAINER(                                             \
    NS, Inline, Container, ReturnType, Name, Args)                             \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(                                       \
        NS, Container, ReturnType, Name, Args);                                \
    Inline NS::_yOMM2_method::function_pointer_type                            \
        Container<ReturnType Args>::next;                                      \
    namespace {                                                                \
    namespace NS {                                                             \
    Inline _yOMM2_method::add_function<Container<ReturnType Args>::fn>         \
        YOMM2_GENSYM(&Container<ReturnType Args>::next);                       \
    }                                                                          \
    }                                                                          \
    Inline NS::_yOMM2_method::return_type Container<ReturnType Args>::fn Args

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_FRIEND(...)                                                      \
    BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_FRIEND(...)                                                      \
    BOOST_PP_CAT(                                                              \
        BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)(__VA_ARGS__),            \
        BOOST_PP_EMPTY())
#endif

#define YOMM2_FRIEND_1(Container)                                              \
    template<typename>                                                         \
    friend struct Container

#define YOMM2_FRIEND_3(Container, ReturnType, Args)                            \
    friend struct Container<ReturnType Args>

#define YOMM2_DEFINITION(Container, ReturnType, Args)                          \
    Container<ReturnType Args>::fn

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

#define YOMM2_METHOD_CLASS_3(ReturnType, Name, Args)                           \
    ::yorel::yomm2::method<YOMM2_SYMBOL(Name), ReturnType Args>

#define YOMM2_METHOD_CLASS_4(ReturnType, Name, Args, Policy)                   \
    ::yorel::yomm2::method<YOMM2_SYMBOL(Name), ReturnType Args, Policy>


#define register_class YOMM2_CLASS
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
