// Copyright (c) 2018-2024 Jean-Louis Leroy

#ifndef YOREL_YOMM2_MACROS_HPP
#define YOREL_YOMM2_MACROS_HPP

#include <yorel/yomm2/macros/register.hpp>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>

#define YOMM2_METHOD_NAME(NAME) yorel_yomm2_##NAME
#define YOMM2_OVERRIDERS(NAME)                                                 \
    BOOST_PP_CAT(YOMM2_METHOD_NAME(NAME), _overriders)

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
    struct YOMM2_METHOD_NAME(NAME);                                            \
    ::yorel::yomm2::method<YOMM2_METHOD_NAME(NAME) ARGS, __VA_ARGS__>          \
        BOOST_PP_CAT(YOMM2_METHOD_NAME(NAME), guide_)(                         \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS));    \
    inline decltype(auto) NAME(                                                \
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
    using method_type = yorel_yomm2_detail_locate_method_aux<void ARGS>::type

#define YOREL_YOMM2_DETAIL_RETURN_TYPE(...)                                    \
    boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define YOREL_YOMM2_DETAIL_DEFINE(INLINE, OVERRIDERS, NAME, ARGS, ...)         \
    template<typename...>                                                      \
    struct OVERRIDERS;                                                         \
    template<>                                                                 \
    struct OVERRIDERS<YOREL_YOMM2_DETAIL_RETURN_TYPE(__VA_ARGS__) ARGS> {      \
        YOREL_YOMM2_DETAIL_LOCATE_METHOD(NAME, ARGS);                          \
        static auto fn ARGS->YOREL_YOMM2_DETAIL_RETURN_TYPE(__VA_ARGS__);      \
        static auto has_next() {                                               \
            return method_type::next<fn> != method_type::fn.not_implemented;   \
        }                                                                      \
        template<typename... Args>                                             \
        static decltype(auto) next(Args&&... args) {                           \
            BOOST_ASSERT(has_next());                                          \
            return method_type::next<fn>(std::forward<Args>(args)...);         \
        }                                                                      \
    };                                                                         \
    INLINE YOMM2_REGISTER(                                                     \
        OVERRIDERS<YOREL_YOMM2_DETAIL_RETURN_TYPE(__VA_ARGS__) ARGS>::         \
            method_type::override<OVERRIDERS<YOREL_YOMM2_DETAIL_RETURN_TYPE(   \
                __VA_ARGS__) ARGS>::fn>);                                      \
    INLINE auto                                                                \
        OVERRIDERS<YOREL_YOMM2_DETAIL_RETURN_TYPE(__VA_ARGS__) ARGS>::fn ARGS  \
            ->boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define YOMM2_OVERRIDE_INLINE(NAME, ARGS, ...)                                 \
    YOREL_YOMM2_DETAIL_DEFINE(                                                 \
        inline, YOMM2_OVERRIDERS(NAME), YOMM2_METHOD_NAME(NAME), ARGS,         \
        __VA_ARGS__)

#define YOMM2_OVERRIDE(NAME, ARGS, ...)                                        \
    YOREL_YOMM2_DETAIL_DEFINE(                                                 \
        , YOMM2_OVERRIDERS(NAME), YOMM2_METHOD_NAME(NAME), ARGS, __VA_ARGS__)

#define YOMM2_CLASSES(...)                                                     \
    static ::yorel::yomm2::use_classes<__VA_ARGS__> YOMM2_GENSYM;

#define YOMM2_METHOD_CLASS(NAME, ARGS, ...)                                    \
    ::yorel::yomm2::method<YOMM2_METHOD_NAME(NAME) ARGS, __VA_ARGS__>

#endif
