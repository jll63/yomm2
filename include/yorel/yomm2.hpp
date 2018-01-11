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

#include <boost/type_traits/is_virtual_base_of.hpp>

#include <boost/mpl/and.hpp>

#ifndef YOMM2_DEBUG
#ifdef NDEBUG
#define YOMM2_DEBUG 0
#else
#define YOMM2_DEBUG 1
#endif
#endif

#if YOMM2_DEBUG

#pragma(message, "Trace is enabled.")

#define _YOMM2_DEBUG(X) X
#define _YOMM2_COMMA_DEBUG(X) , X

#include <iostream>
#include <iterator>

#else
#define _YOMM2_DEBUG(ST)
#define _YOMM2_COMMA_DEBUG(X)
#endif

#define _YOMM2_NS BOOST_PP_CAT(_YOMM2_NS_, __COUNTER__)

#define _YOMM2_PLIST(N, I, A)                                                 \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::virtual_arg_t<BOOST_PP_TUPLE_ELEM(I, A)>  \
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
      init _YOMM2_DEBUG( = #ID "(" #__VA_ARGS__ ")" ); } }                    \
    ::yorel::yomm2::method<REGISTRY, _YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> ID( \
        ::yorel::yomm2::details::discriminator,                               \
        BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                  \
                        _YOMM2_PLIST, (__VA_ARGS__)));                        \
    R ID(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                 \
                             _YOMM2_PLIST, (__VA_ARGS__))) {                  \
        return ::yorel::yomm2::method<REGISTRY, _YOMM2_DECLARE_KEY(ID), R, __VA_ARGS__> \
            ::dispatch(BOOST_PP_REPEAT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),   \
                                     _YOMM2_ALIST, (__VA_ARGS__))); }

#define YOMM2_DEFINE(RETURN_T, ID, ...)                                              \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    template<typename T> struct select_method;                                \
    template<typename... A> struct select_method<void(A...)> {                \
        using type = decltype(ID(::yorel::yomm2::details::discriminator(),    \
                                 std::declval<A>()...));                      \
    };                                                                        \
    using _YOMM2_RETURN_T = RETURN_T;                                         \
    using _YOMM2_METHOD = select_method<void(__VA_ARGS__)>::type;             \
    using _YOMM2_SIGNATURE = void(__VA_ARGS__);                               \
    const char* _YOMM2_NAME = "(" #__VA_ARGS__ ")";                           \
    struct _YOMM2_SPEC {                                                      \
        static RETURN_T body(__VA_ARGS__)

#define YOMM2_END                                                             \
    };                                                                        \
    ::yorel::yomm2::register_spec<_YOMM2_RETURN_T, _YOMM2_METHOD, _YOMM2_SPEC, _YOMM2_SIGNATURE> \
    init _YOMM2_DEBUG( = _YOMM2_NAME  );                                   \
    } }

#define _YOMM2_CLASS_NAME(CLASS, ...) \
    #CLASS

#define YOMM2_CLASS_(REG, ...)                                                \
    namespace {                                                               \
    namespace _YOMM2_NS {                                                     \
    ::yorel::yomm2::                                                          \
    init_class_info<REG _YOMM2_CLASS_LIST(BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__)) \
        > init _YOMM2_DEBUG( { _YOMM2_CLASS_NAME(__VA_ARGS__ ) } ); } }

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
    void* pf;
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

struct dynamic_cast_ {};
struct static_cast_ {};

template<typename T>
struct virtual_traits {
    using base_type = typename std::remove_cv_t<std::remove_reference_t<T>>;
    using argument_type = T;
    template<typename>
    static T cast(T val, static_cast_) {
        return val;
    }
    template<typename>
    static T cast(T val, dynamic_cast_) {
        return val;
    }
};

template<typename T>
struct virtual_traits< virtual_<T&> > {
    using base_type = typename std::remove_cv_t<T>;
    using argument_type = T&;
    template<class DERIVED>
    static DERIVED& cast(T& obj, static_cast_) {
        return static_cast<DERIVED&>(obj);
    }
    template<class DERIVED>
    static DERIVED& cast(T& obj, dynamic_cast_) {
        return dynamic_cast<DERIVED&>(obj);
    }
};

template<typename T>
using virtual_base_t = typename virtual_traits<T>::base_type;

template<typename T>
using virtual_arg_t = typename virtual_traits<T>::argument_type;

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

_YOMM2_DEBUG(std::ostream& log());
_YOMM2_DEBUG(std::ostream* log_on(std::ostream* os));
_YOMM2_DEBUG(std::ostream* log_off());

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
    void* pf;
};

struct method_info {
    _YOMM2_DEBUG(const char* name);
    std::vector<const class_info*> vp;
    std::vector<const spec_info*> specs;
    void* ambiguous_call;
    void* not_implemented;
    const word** slots_strides_p{nullptr};

};

template<typename BASE, typename DERIVED>
struct select_cast {
    using type = std::conditional_t<
        boost::is_virtual_base_of<BASE, DERIVED>::value, dynamic_cast_, static_cast_
    >;
};

template<typename BASE, typename DERIVED>
using select_cast_t = typename select_cast<BASE, DERIVED>::type;

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

    static void* resolve(const word* ssp, virtual_base_t<FIRST> first, REST... rest) {
        return for_each_vp<REG, REST...>::resolve(ssp, rest...);
    }
};

template<typename REG, typename FIRST, typename... REST>
struct for_each_vp<REG, virtual_<FIRST>, REST...> {

    static void collect_class_info(std::vector<const class_info*>& vp) {
        vp.push_back(
            &class_info::get<REG, virtual_base_t<virtual_<FIRST>>>());
        for_each_vp<REG, REST...>::collect_class_info(vp);
    }

    template<typename SPEC_FIRST, typename... SPEC_REST>
    struct for_spec {
        static void collect_class_info(std::vector<const class_info*>& vp) {
            vp.push_back(
                &class_info::get<REG, virtual_base_t<virtual_<SPEC_FIRST>>>());
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
    static void* resolve(
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        return resolver<REG, 1, REST...>::resolve(ssp, rest...);
    }
};

template<typename REG, typename FIRST, typename... REST>
struct resolver<REG, 1, virtual_<FIRST>, REST...>
{
    static void* resolve(
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        _YOMM2_DEBUG(details::log() << "  slot = " << ssp->i << " key = " << &typeid(first));
        auto pf = details::mptr(registry::get<REG>(), &typeid(first))[ssp->i].pf;
        _YOMM2_DEBUG(details::log() << " pf = " << pf << "\n");
        return pf;
    }

    static void* resolve_next(
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        _YOMM2_DEBUG(details::log() << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(details::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(details::log() << " slot = " << slot);
        auto stride = ssp++->i;
        _YOMM2_DEBUG(details::log() << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        _YOMM2_DEBUG(details::log() << " dispatch = " << dispatch);
        auto pf = dispatch->pf;
        _YOMM2_DEBUG(details::log() << " pf = " << pf << "\n");
        return pf;
    }
};

template<typename REG, int ARITY, typename FIRST, typename... REST>
struct resolver<REG, ARITY, FIRST, REST...>
{
    static void* resolve(
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        return resolver<REG, ARITY, REST...>::resolve_first(ssp, rest...);
    }

    static void* resolve_next(
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        return resolver<REG, ARITY, REST...>::resolve_next(ssp, dispatch, rest...);
    }
};

template<typename REG, int ARITY, typename FIRST, typename... REST>
struct resolver<REG, ARITY, virtual_<FIRST>, REST...>
{
    static void* resolve_first(
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        _YOMM2_DEBUG(details::log() << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(details::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(details::log() << " slot = " << slot);
        auto dispatch = mptr[slot].pw;
        _YOMM2_DEBUG(details::log() << " dispatch = " << dispatch << "\n");
        return resolver<REG, ARITY - 1, REST...>::resolve_next(
            ssp, dispatch, rest...);
    }

    static void* resolve_next(
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        _YOMM2_DEBUG(details::log() << "  key = " << &typeid(first));
        auto mptr = details::mptr(registry::get<REG>(), &typeid(first));
        _YOMM2_DEBUG(details::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        _YOMM2_DEBUG(details::log() << " slot = " << slot);
        auto stride = ssp++->i;
        _YOMM2_DEBUG(details::log() << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        _YOMM2_DEBUG(details::log() << " dispatch = " << dispatch << "\n");
        return resolver<REG, ARITY - 1, REST...>::resolve_next(
            ssp, dispatch, rest...);
    }

    static void* resolve(
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        return resolve_first(ssp, first, rest...);
    }
};

template<typename BASE_RETURN, class FUNCTION, typename BASE, typename SPEC>
struct wrapper;

template<
    typename BASE_RETURN,
    class FUNCTION,
    typename... BASE_PARAM,
    typename... SPEC_PARAM
    >
struct wrapper<BASE_RETURN, FUNCTION, BASE_RETURN(BASE_PARAM...), BASE_RETURN(SPEC_PARAM...)> {
    static BASE_RETURN body(virtual_arg_t<BASE_PARAM>... arg) {
    return FUNCTION::body(
        virtual_traits<BASE_PARAM>::template cast<SPEC_PARAM>(
            arg,
            typename select_cast<
                virtual_base_t<BASE_PARAM>,
                virtual_base_t<SPEC_PARAM>>::type())...);
  }
};

template<typename RETURN_T, class METHOD, class SPEC, typename F>
struct register_spec;

// template<
//     typename BASE_RETURN,
//     class FUNCTION,
//     typename... BASE_PARAM,
//     typename... SPEC_PARAM
//     >
// struct wrapper<BASE_RETURN, FUNCTION, BASE_RETURN(BASE_PARAM...), BASE_RETURN(SPEC_PARAM...)> {

template<typename RETURN_T, class METHOD, class SPEC, class... SPEC_ARGS>
struct register_spec<RETURN_T, METHOD, SPEC, void(SPEC_ARGS...)>
{
    register_spec(_YOMM2_DEBUG(const char* name)) {
        static spec_info si;
        _YOMM2_DEBUG(si.name = name);
        // si.pf = (const void*) SPEC::body;
        si.pf = (void*) wrapper<
            RETURN_T, SPEC, typename METHOD::signature_t, RETURN_T(SPEC_ARGS...)
        >::body;
        METHOD::for_each_vp_t::template for_spec<SPEC_ARGS...>::collect_class_info(si.vp);
        METHOD::info().specs.push_back(&si);
    }
};

template<typename REG, typename ID, typename R, typename... A>
struct method {

    static const word* slots_strides; // slot 0, slot 1,  stride 1, slot 2, ...

    static method_info& info();

    static R dispatch(virtual_arg_t<A>... args) {
        return reinterpret_cast<R (*)(virtual_arg_t<A>...)>(
            const_cast<void*>(resolve(args...)))(
            args...
            );
    }

    using signature_t = R(A...);
    using for_each_vp_t = for_each_vp<REG, A...>;

    enum { arity = for_each_vp_t::count };

    static void* resolve(virtual_arg_t<A>... args) {
        _YOMM2_DEBUG(details::log() << "call " << name() << " slots_strides = " << slots_strides << "\n");
        return resolver<REG, arity, A...>::resolve(slots_strides, args...);
    }

#if YOMM2_DEBUG
    static const char* name() { return info().name; }
#endif

    struct init_method {
        init_method(_YOMM2_DEBUG(const char* name)) {
            _YOMM2_DEBUG(info().name = name);
            info().slots_strides_p = &slots_strides;
            for_each_vp_t::collect_class_info(info().vp);
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

void update_methods(registry& reg = registry::global());

} // namespace yomm2
} // namespace yorel


#endif
