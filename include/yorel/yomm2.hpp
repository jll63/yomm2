#ifndef YOREL_YOMM2_INCLUDED
#define YOREL_YOMM2_INCLUDED

#include <vector>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/cat.hpp>

#ifndef YOMM2_DEBUG
#ifdef NDEBUG
#define YOMM2_DEBUG 0
#else
#define YOMM2_DEBUG 1
#endif
#endif

#if YOMM2_DEBUG

#define _YOMM2_DEBUG(X) X
#define _YOMM2_COMMA_DEBUG(X) , X

#define _YOMM2_INIT_METHOD_NAME(ID, R, ...)                                   \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::method<_YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__>::init_name  \
    init(#ID "(" #__VA_ARGS__ ")"); } }                                       \

#include <iostream>

#else
#define _YOMM2_DEBUG(ST)
#define _YOMM2_COMMA_DEBUG(X)
#define _YOMM2_INIT_METHOD_NAME(ID, R, ...)
#endif

#define _YOMM2_NS BOOST_PP_CAT(_YOMM2_NS_, __COUNTER__)

#define _YOMM2_PLIST(N, I, A)                                                 \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::details::remove_virtual<BOOST_PP_TUPLE_ELEM(I, A)>::type  \
    BOOST_PP_CAT(a, I)

#define _YOMM2_ALIST(N, I, ARGS) \
    BOOST_PP_COMMA_IF(I) BOOST_PP_CAT(a, I)

#define _YOMM2_DECLARE_KEY(ID)                                                \
    BOOST_PP_CAT(_yomm2_method_, ID)

#define YOMM2_DECLARE(R, ID, ...)                                             \
    struct _YOMM2_DECLARE_KEY(ID);                                            \
    _YOMM2_INIT_METHOD_NAME(ID, R, __VA_ARGS__)                               \
    ::yorel::yomm2::method<_YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> ID(        \
        ::yorel::yomm2::details::discriminator,                               \
        BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                  \
                        _YOMM2_PLIST, (__VA_ARGS__)));                        \
    R ID(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                 \
                             _YOMM2_PLIST, (__VA_ARGS__))) {                  \
        return ::yorel::yomm2::method<_YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> \
            ::dispatch(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),   \
                                     _YOMM2_ALIST, (__VA_ARGS__))); }


#define YOMM2_DEFINE(R, ID, ...)                                              \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    template<typename T> struct select_method;                                \
    template<typename... A> struct select_method<void(A...)> {                \
        using type = decltype(ID(::yorel::yomm2::details::discriminator(),    \
                                   std::declval<A>()...));                    \
    };                                                                        \
    select_method<void(__VA_ARGS__)>::type::register_spec                     \
    init("(" #__VA_ARGS__ ")");                                               \
    R body(__VA_ARGS__)

#define YOMM2_END } }

namespace yorel {
namespace yomm2 {

template<typename T>
struct virtual_;

namespace details {

template<typename T>
struct remove_virtual {
    using type = T;
};

template<typename T>
struct remove_virtual< virtual_<T> > {
    using type = T;
};

struct discriminator {};

} // namespace details

template<typename ID, typename R, typename... A>
struct method {
    static R dispatch(typename details::remove_virtual<A>::type... a) {
        _YOMM2_DEBUG(std::cerr << "call " << name() << "\n");
    }
    static const char* name() { return _name; }

    static const char* _name;

    struct init_name {
        init_name(const char* name) {
            _name = name;
        }
    };

    struct register_spec {
        register_spec(const char* description) {
            _YOMM2_DEBUG(std::cerr << name() << " += " << description << "\n");
        }
    };
};

template<typename ID, typename R, typename... A>
const char* method<ID, R, A...>::_name;

} // namespace yomm2
} // namespace yorel

#endif
