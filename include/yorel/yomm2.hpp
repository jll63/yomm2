#ifndef YOREL_YOMM2_INCLUDED
#define YOREL_YOMM2_INCLUDED

#include <vector>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <unordered_set>

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>
#include <boost/preprocessor/comparison/greater.hpp>

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
#include <iterator>

#else
#define _YOMM2_DEBUG(ST)
#define _YOMM2_COMMA_DEBUG(X)
#define _YOMM2_INIT_METHOD(ID, R, ...)
#endif

#define _YOMM2_NS BOOST_PP_CAT(_YOMM2_NS_, __COUNTER__)

#define _YOMM2_PLIST(N, I, A)                                                 \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::virtual_traits<BOOST_PP_TUPLE_ELEM(I, A)>::type  \
    BOOST_PP_CAT(a, I)

#define _YOMM2_ALIST(N, I, ARGS) \
    BOOST_PP_COMMA_IF(I) BOOST_PP_CAT(a, I)

#define _YOMM2_DECLARE_KEY(ID)                                                \
    BOOST_PP_CAT(_yomm2_method_, ID)

#define YOMM2_DECLARE(R, ID, ...) \
    YOMM2_DECLARE_(::yorel::yomm2::registry::global_, R, ID, __VA_ARGS__)

#define YOMM2_DECLARE_(REGISTRY, R, ID, ...)                                   \
    struct _YOMM2_DECLARE_KEY(ID);                                            \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::method<REGISTRY, _YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__>::init_method \
    init(_YOMM2_DEBUG(#ID "(" #__VA_ARGS__ ")")); } }         \
    ::yorel::yomm2::method<REGISTRY, _YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> ID( \
        ::yorel::yomm2::details::discriminator,                               \
        BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                  \
                        _YOMM2_PLIST, (__VA_ARGS__)));                        \
    R ID(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                 \
                             _YOMM2_PLIST, (__VA_ARGS__))) {                  \
        return ::yorel::yomm2::method<REGISTRY, _YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> \
            ::dispatch(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),   \
                                     _YOMM2_ALIST, (__VA_ARGS__))); }

#define YOMM2_DEFINE(R, ID, ...)                                              \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    template<typename T> struct select_method;                                \
    template<typename... A> struct select_method<void(A...)> {                \
        using type = decltype(ID(::yorel::yomm2::details::discriminator(),    \
                                 std::declval<A>()...));                      \
    };                                                                        \
    using _YOMM2_METHOD = select_method<void(__VA_ARGS__)>::type;             \
    using _YOMM2_SIGNATURE = void(__VA_ARGS__);                               \
    const char* _YOMM2_NAME = "(" #__VA_ARGS__ ")";                           \
    struct _YOMM2_SPEC {                                                      \
        static R body(__VA_ARGS__)

#define YOMM2_END                                                             \
    };                                                                        \
    ::yorel::yomm2::register_spec<_YOMM2_METHOD, _YOMM2_SPEC, _YOMM2_SIGNATURE> \
    init(_YOMM2_DEBUG(_YOMM2_NAME));                                   \
    } }

#define _YOMM2_CLASS_NAME(CLASS, ...) \
    #CLASS

#define YOMM2_CLASS_(REG, ...)                                                \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::                                                          \
    init_class_info<REG _YOMM2_CLASS_LIST(BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__)) \
                    > init(_YOMM2_CLASS_NAME(__VA_ARGS__)); } }

#define YOMM2_CLASS(...)                                               \
    YOMM2_CLASS_(::yorel::yomm2::registry::global_, __VA_ARGS__)

#define _YOMM2_CLIST(N, I, A) \
    , BOOST_PP_TUPLE_ELEM(I, A)

#define _YOMM2_CLASS_LIST(TUPLE)                                              \
    BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(TUPLE),                               \
                    _YOMM2_CLIST, TUPLE)                                      \

namespace yorel {
namespace yomm2 {

struct method_info;
struct class_info;

union word {
    const void* pv;
    const word* pw;
    int i;
};

struct registry {
    std::vector<const class_info*> classes;
    std::vector<method_info*> methods;
    std::uintptr_t hash_mult;
    std::size_t hash_shift;

    // global vector:
    std::vector<word> gv;

    template<typename T> static registry& get();

    struct global_;
    static registry& global() { return get<global_>(); }
};

template<typename T> registry& registry::get() {
    static registry r;
    return r;
}

template<typename T>
struct virtual_;

template<typename T>
struct virtual_traits {
    using type = T;
    using argument_type = T;
};

template<typename T>
struct virtual_traits< virtual_<T&> > {
    using type = typename std::remove_cv<T>::type;
    using argument_type = T&;
};

namespace details {

struct discriminator {};

inline std::size_t hash(const registry& reg, const void* p) {
    return static_cast<std::size_t>(
        (reg.hash_mult * reinterpret_cast<std::uintptr_t>(const_cast<void*>(p)))
        >> reg.hash_shift);
}

inline const word* mptr(const registry& reg, const std::type_info* ti) {
    return reg.gv[hash(reg, ti)].pw;
}

} // namespace details

struct class_info {
    std::vector<const class_info*> direct_bases;
    _YOMM2_DEBUG(const char* name);
    std::unordered_set<const void*> ti_ptrs;

    template<typename REG, class CLASS> static class_info& get();
};

template<typename REG, class CLASS>
class_info& class_info::get() {
    static class_info info;
    return info;
}

template<typename REG, class CLASS, class... BASE>
struct init_class_info {

    init_class_info(_YOMM2_DEBUG(const char* name)) {
        auto& info = class_info::get<REG, CLASS>();
        static int called;
        if (!called++) {
            _YOMM2_DEBUG(info.name = name);
            info.direct_bases = { &class_info::get<REG, BASE>()... };
            info.ti_ptrs.insert(&typeid(CLASS));
            registry::get<REG>().classes.push_back(&info);
        }
    }

};

struct spec_info {
    _YOMM2_DEBUG(const char* name);
    std::vector<const class_info*> vp;
    const void* pf;
};

struct method_info {
    _YOMM2_DEBUG(const char* name);
    std::vector<const class_info*> vp;
    std::vector<const spec_info*> specs;
    void* ambiguous_call;
    void* not_implemented;
    const word** slots_strides_p{nullptr};

};

template<typename REG, typename... ARGS>
struct for_each_vp;

template<typename REG, typename FIRST, typename... REST>
struct for_each_vp<REG, FIRST, REST...> {

    static void collect_class_info(std::vector<const class_info*>& vp) {
        for_each_vp<REG, REST...>::collect_class_info(vp);
    }

    template<typename SPEC_FIRST, typename... SPEC_REST>
    struct for_spec {
        static void collect_class_info(std::vector<const class_info*>& vp) {
            for_each_vp<REG, REST...>::template for_spec<SPEC_REST...>::collect_class_info(vp);
        }
    };

    enum { count = for_each_vp<REG, REST...>::count };

    static const void* resolve(const word* ssp, typename virtual_traits<FIRST>::type first, REST... rest) {
        return for_each_vp<REG, REST...>::resolve(ssp, rest...);
    }
};

template<typename REG, typename FIRST, typename... REST>
struct for_each_vp<REG, virtual_<FIRST>, REST...> {

    static void collect_class_info(std::vector<const class_info*>& vp) {
        vp.push_back(
            &class_info::get<REG, typename virtual_traits<virtual_<FIRST>>::type>());
        for_each_vp<REG, REST...>::collect_class_info(vp);
    }

    template<typename SPEC_FIRST, typename... SPEC_REST>
    struct for_spec {
        static void collect_class_info(std::vector<const class_info*>& vp) {
            vp.push_back(
                &class_info::get<REG, typename virtual_traits<virtual_<SPEC_FIRST>>::type>());
            for_each_vp<REG, REST...>::template for_spec<SPEC_REST...>::collect_class_info(vp);
        }
    };

    enum { count = 1 + for_each_vp<REG, REST...>::count };
};

template<typename REG>
struct for_each_vp<REG> {

    static void collect_class_info(std::vector<const class_info*>& vp) {
    }

    template<typename...>
    struct for_spec {
        static void collect_class_info(std::vector<const class_info*>& vp) {
        }
    };

    enum { count = 0 };
};

template<typename REG, int ARITY, typename... A>
struct resolver;

template<typename REG, typename FIRST, typename... REST>
struct resolver<REG, 1, FIRST, REST...>
{
    static const void* resolve(
        const word* ssp,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest) {
        return resolver<REG, 1, REST...>::resolve(ssp, rest...);
    }
};

template<typename REG, typename FIRST, typename... REST>
struct resolver<REG, 1, virtual_<FIRST>, REST...>
{
    static const void* resolve(
        const word* ssp,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest) {
        _YOMM2_DEBUG(std::cerr << "  slot = " << ssp->i << " key = " << &typeid(first));
        auto pf = details::mptr(registry::get<REG>(), &typeid(first))[ssp->i].pv;
        _YOMM2_DEBUG(std::cerr << " pf = " << pf << "\n");
        return pf;
    }

    static const void* resolve_next(
        const word* ssp,
        const word* dispatch,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest)
    {
        _YOMM2_DEBUG(std::cerr << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(std::cerr << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(std::cerr << " slot = " << slot);
        auto stride = ssp++->i;
        _YOMM2_DEBUG(std::cerr << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        _YOMM2_DEBUG(std::cerr << " dispatch = " << dispatch);
        auto pf = dispatch->pv;
        _YOMM2_DEBUG(std::cerr << " pf = " << pf << "\n");
        return pf;
    }
};

template<typename REG, int ARITY, typename FIRST, typename... REST>
struct resolver<REG, ARITY, FIRST, REST...>
{
    static const void* resolve(
        const word* ssp,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest) {
        return resolver<REG, ARITY, REST...>::resolve_first(ssp, rest...);
    }

    static const void* resolve_next(
        const word* ssp,
        const word* dispatch,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest)
    {
        return resolver<REG, ARITY, REST...>::resolve_next(ssp, dispatch, rest...);
    }
};

template<typename REG, int ARITY, typename FIRST, typename... REST>
struct resolver<REG, ARITY, virtual_<FIRST>, REST...>
{
    static const void* resolve_first(
        const word* ssp,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest)
    {
        _YOMM2_DEBUG(std::cerr << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(std::cerr << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(std::cerr << " slot = " << slot);
        auto dispatch = mptr[slot].pw;
        _YOMM2_DEBUG(std::cerr << " dispatch = " << dispatch << "\n");
        return resolver<REG, ARITY - 1, REST...>::resolve_next(
            ssp, dispatch, rest...);
    }

    static const void* resolve_next(
        const word* ssp,
        const word* dispatch,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest)
    {
        _YOMM2_DEBUG(std::cerr << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(std::cerr << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(std::cerr << " slot = " << slot);
        auto stride = ssp++->i;
        _YOMM2_DEBUG(std::cerr << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        _YOMM2_DEBUG(std::cerr << " dispatch = " << dispatch << "\n");
        return resolver<REG, ARITY - 1, REST...>::resolve_next(
            ssp, dispatch, rest...);
    }

    static const void* resolve(
        const word* ssp,
        typename virtual_traits<FIRST>::argument_type first,
        typename virtual_traits<REST>::argument_type... rest)
    {
        return resolve_first(ssp, first, rest...);
    }
};

template<class METHOD, class SPEC, typename F>
struct register_spec;

template<class METHOD, class SPEC, class... ARGS>
struct register_spec<METHOD, SPEC, void(ARGS...)>
{
    register_spec(_YOMM2_DEBUG(const char* name)) {
        static spec_info si;
        _YOMM2_DEBUG(si.name = name);
        si.pf = (const void*) SPEC::body;
        METHOD::for_each_vp::template for_spec<ARGS...>::collect_class_info(si.vp);
        METHOD::info().specs.push_back(&si);
    }
};

template<typename REG, typename ID, typename R, typename... A>
struct method {

    static const word* slots_strides; // slot 0, slot 1,  stride 1, slot 2, ...

    static method_info& info();

    static R dispatch(typename virtual_traits<A>::type... a) {
        _YOMM2_DEBUG(std::cerr << "call " << name() << "\n");
        return R();
    }

    using for_each_vp = for_each_vp<REG, A...>;

    enum { arity = for_each_vp::count };

    static const void* resolve(typename virtual_traits<A>::argument_type... args) {
        _YOMM2_DEBUG(std::cerr << "call " << name() << " slots_strides = " << slots_strides << "\n");
        return resolver<REG, arity, A...>::resolve(slots_strides, args...);
    }

#if YOMM2_DEBUG
    static const char* name() { return info().name; }
#endif

    struct init_method {
        init_method(_YOMM2_DEBUG(const char* name)) {
            _YOMM2_DEBUG(info().name = name);
            info().slots_strides_p = &slots_strides;
            for_each_vp::collect_class_info(info().vp);
            registry::get<REG>().methods.push_back(&info());
        }
    };
};

template<typename REG, typename ID, typename R, typename... A>
const word* method<REG, ID, R, A...>::slots_strides;

template<typename REG, typename ID, typename R, typename... A>
method_info& method<REG, ID, R, A...>::info() {
    static method_info info;
    return info;
}

void update_methods(const registry& reg = registry::global());

} // namespace yomm2
} // namespace yorel


#endif
