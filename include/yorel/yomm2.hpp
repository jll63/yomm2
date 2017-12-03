#ifndef YOREL_YOMM2_INCLUDED
#define YOREL_YOMM2_INCLUDED

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

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

#define _YOMM2_INIT_METHOD(ID, R, ...)                                   \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::method<_YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__>::init_method  \
    init(#ID "(" #__VA_ARGS__ ")"); } }                                       \

#include <iostream>

#else
#define _YOMM2_DEBUG(ST)
#define _YOMM2_COMMA_DEBUG(X)
#define _YOMM2_INIT_METHOD(ID, R, ...)
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

#define YOMM2_DECLARE(R, ID, ...) \
        YOMM2_DECLARE_(::yorel::yomm2::global_method_registry(), R, ID, __VA_ARGS__)

#define YOMM2_DECLARE_(REGISTRY, R, ID, ...)                                   \
    struct _YOMM2_DECLARE_KEY(ID);                                            \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::method<_YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__>::init_method  \
    init(REGISTRY  _YOMM2_COMMA_DEBUG(#ID "(" #__VA_ARGS__ ")")); } }         \
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
    R body(__VA_ARGS__);                                                      \
    select_method<void(__VA_ARGS__)>::type::register_spec                     \
    init(body _YOMM2_COMMA_DEBUG("(" #__VA_ARGS__ ")"));                      \
    R body(__VA_ARGS__)

#define YOMM2_END } }

namespace yorel {
namespace yomm2 {

class method_info;

using method_registry_t = std::vector<const method_info*>;

method_registry_t& global_method_registry();

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

struct spec_info {
    _YOMM2_DEBUG(const char* description;)
};

struct method_info {
    _YOMM2_DEBUG(const char* description;)
    std::vector<const spec_info*> specs;
};

template<typename ID, typename R, typename... A>
struct method {

    static method_info& info();

    struct register_spec {
        template<typename F>
        register_spec(F _YOMM2_COMMA_DEBUG(const char* description)) {
            static spec_info si;
            _YOMM2_DEBUG(si.description = description);
            info().specs.push_back(&si);
        }
    };

    static R dispatch(typename details::remove_virtual<A>::type... a) {
        _YOMM2_DEBUG(std::cerr << "call " << description() << "\n");
    }
#if YOMM2_DEBUG
    static const char* description() { return info().description; }

    struct init_method {
        init_method(method_registry_t& registry
                    _YOMM2_COMMA_DEBUG(const char* description)) {
            info().description = description;
            registry.push_back(&info());
        }
    };
#endif
};

template<typename ID, typename R, typename... A>
method_info& method<ID, R, A...>::info() {
    static method_info info;
    return info;
}

struct class_info {
    std::unordered_set<const std::type_info*> ti;
};

template<class C>
struct class_info_singleton {
    static class_info ci;
};

using class_registry_t = std::unordered_map<std::type_index, class_info>;

class_registry_t& global_class_registry();

template<class C, class... B>
struct init_class_info {

    init_class_info(class_registry_t& registry) {
        auto& ci = registry[std::type_index(typeid(C))];
    }

};

void update_methods(const class_registry_t& classes = global_class_registry(),
                    const method_registry_t& methods = global_method_registry());

} // namespace yomm2
} // namespace yorel

#endif
