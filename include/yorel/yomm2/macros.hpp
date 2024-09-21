// Copyright (c) 2018-2024 Jean-Louis Leroy

#ifndef YOREL_YOMM2_MACROS_HPP
#define YOREL_YOMM2_MACROS_HPP

#include <yorel/yomm2/macros/register.hpp>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>

#define YOMM2_METHOD_NAME(NAME) NAME##_method_

#define yOMM2_PLIST(N, I, A)                                                   \
    BOOST_PP_COMMA_IF(I)                                                       \
    ::yorel::yomm2::detail::remove_virtual<BOOST_PP_TUPLE_ELEM(I, A)>          \
    BOOST_PP_CAT(a, I)

#define yOMM2_ALIST(N, I, A)                                                   \
    BOOST_PP_COMMA_IF(I)                                                       \
    std::forward<                                                              \
        ::yorel::yomm2::detail::remove_virtual<BOOST_PP_TUPLE_ELEM(I, A)>>(    \
        BOOST_PP_CAT(a, I))

#define yOMM2_RLIST(N, I, A) BOOST_PP_COMMA_IF(I) BOOST_PP_CAT(a, I)

#define YOMM2_METHOD(NAME, ARGS, ...)                                          \
    YOREL_YOMM2_DETAIL_DECLARE(, NAME, ARGS, __VA_ARGS__)

#define YOMM2_STATIC_METHOD(NAME, ARGS, ...)                                   \
    YOREL_YOMM2_DETAIL_DECLARE(static, NAME, ARGS, __VA_ARGS__)

#define YOREL_YOMM2_DETAIL_DECLARE(STATIC, NAME, ARGS, ...)                    \
    struct YOMM2_METHOD_NAME(NAME);                                            \
    STATIC ::yorel::yomm2::method<YOMM2_METHOD_NAME(NAME) ARGS, __VA_ARGS__>   \
        BOOST_PP_CAT(YOMM2_METHOD_NAME(NAME), guide_)(                         \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS));    \
    STATIC inline decltype(auto) NAME(                                         \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS)) {       \
        return ::yorel::yomm2::                                                \
            method<YOMM2_METHOD_NAME(NAME) ARGS, __VA_ARGS__>::fn(             \
                BOOST_PP_REPEAT(                                               \
                    BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_ALIST, ARGS));            \
    }

#define YOREL_YOMM2_DETAIL_LOCATE_METHOD(NAME, ARGS)                           \
    template<typename T>                                                       \
    struct yorel_yomm2_detail_locate_method_aux;                               \
    template<typename... A>                                                    \
    struct yorel_yomm2_detail_locate_method_aux<void(A...)> {                  \
        using type = decltype(NAME##guide_(std::declval<A>()...));             \
    };                                                                         \
    using method_type = yorel_yomm2_detail_locate_method_aux<void ARGS>::type;

#define YOREL_YOMM2_DETAIL_DEFINE(INLINE, OVERRIDERS, NAME, ARGS, ...)         \
    template<typename...>                                                      \
    struct OVERRIDERS;                                                         \
    template<>                                                                 \
    struct OVERRIDERS<NAME ARGS> {                                             \
        YOREL_YOMM2_DETAIL_LOCATE_METHOD(NAME, ARGS);                          \
        static method_type::next_type next;                                    \
        static boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>> fn      \
            ARGS;                                                              \
    };                                                                         \
    INLINE OVERRIDERS<NAME ARGS>::method_type::next_type                       \
        OVERRIDERS<NAME ARGS>::next;                                           \
    INLINE YOMM2_REGISTER(                                                     \
        OVERRIDERS<NAME ARGS>::method_type::override<OVERRIDERS<NAME ARGS>>);  \
    INLINE auto OVERRIDERS<NAME ARGS>::fn ARGS                                 \
        ->boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define YOMM2_OVERRIDE_IN(OVERRIDERS, NAME, ARGS, ...)                         \
    YOREL_YOMM2_DETAIL_DEFINE(                                                 \
        , OVERRIDERS, YOMM2_METHOD_NAME(NAME), ARGS, __VA_ARGS__)

#define YOMM2_OVERRIDE_INLINE(OVERRIDERS, NAME, ARGS, ...)                     \
    YOREL_YOMM2_DETAIL_DEFINE(                                                 \
        inline, OVERRIDERS, YOMM2_METHOD_NAME(NAME), ARGS, __VA_ARGS__)

#define YOMM2_OVERRIDE(NAME, ARGS, ...)                                        \
    YOREL_YOMM2_DETAIL_DEFINE(                                                 \
        , yorel_yomm2_overriders, YOMM2_METHOD_NAME(NAME), ARGS, __VA_ARGS__)

#define YOMM2_CLASSES(...)                                                     \
    static ::yorel::yomm2::detail::use_classes_macro<                          \
        __VA_ARGS__, YOMM2_DEFAULT_POLICY>                                     \
        YOMM2_GENSYM;

#define YOMM2_METHOD_CLASS(NAME, ARGS, ...)                                    \
    ::yorel::yomm2::method<YOMM2_METHOD_NAME(NAME) ARGS, __VA_ARGS__>

#define register_classes YOMM2_CLASSES
#define declare_method YOMM2_METHOD
#define declare_static_method YOMM2_STATIC_METHOD
#define define_method YOMM2_OVERRIDE
#define define_method_in YOMM2_OVERRIDE_IN
#define define_method_inline YOMM2_OVERRIDE_INLINE
#define method_class YOMM2_METHOD_CLASS

#endif
