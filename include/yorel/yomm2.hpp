// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_INCLUDED
#define YOREL_YOMM2_INCLUDED

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>

#include <boost/type_traits/is_virtual_base_of.hpp>

#ifndef YOMM2_ENABLE_TRACE
#ifdef NDEBUG
#define YOMM2_ENABLE_TRACE 0
#else
#define YOMM2_ENABLE_TRACE 1
#endif
#endif

#if YOMM2_ENABLE_TRACE

#define YOMM2_TRACE(X) X
#define YOMM2_TRACE_COMMA(X) , X

#include <iostream>
#include <iterator>

#else
#define YOMM2_TRACE(ST)
#define YOMM2_TRACE_COMMA(X)
#endif

#define yOMM2_WITH_GENSYM(MACRO, ...)                                             \
    MACRO(BOOST_PP_CAT(YoMm2_nS_, __COUNTER__), __VA_ARGS__)

#define yOMM2_PLIST(N, I, A)                                                  \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::detail::virtual_arg_t<BOOST_PP_TUPLE_ELEM(I, A)>          \
    BOOST_PP_CAT(a, I)

#define yOMM2_ALIST(N, I, ARGS)                                               \
    BOOST_PP_COMMA_IF(I) BOOST_PP_CAT(a, I)

#define yOMM2_DECLARE_KEY(ID)                                                 \
    BOOST_PP_CAT(_yomm2_method_, ID)

#define YOMM2_DECLARE_(REGISTRY, R, ID, ARGS)                                 \
    yOMM2_WITH_GENSYM(yOMM2_DECLARE, REGISTRY, R, ID, ARGS,                   \
                      ::yorel::yomm2::default_policy)

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DECLARE(...)                                                    \
    BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_DECLARE(...)                                                    \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__) \
                 (__VA_ARGS__), BOOST_PP_EMPTY())
#endif

#define YOMM2_DECLARE_3(R, ID, ARGS)                                \
    yOMM2_WITH_GENSYM(yOMM2_DECLARE, void, R, ID, ARGS,             \
                      ::yorel::yomm2::default_policy)

#define YOMM2_DECLARE_4(R, ID, ARGS, POLICY)                                  \
    yOMM2_WITH_GENSYM(yOMM2_DECLARE, void, R, ID, ARGS, POLICY)

#define yOMM2_DECLARE(NS, REGISTRY, R, ID, ARGS, POLICY)                     \
    struct yOMM2_DECLARE_KEY(ID);                                             \
    namespace {                                                               \
    namespace NS {                                                            \
    using _yOMM2_method = ::yorel::yomm2::detail::method                      \
        <REGISTRY, yOMM2_DECLARE_KEY(ID), R ARGS, POLICY>;                    \
    _yOMM2_method::init_method init YOMM2_TRACE( = #ID  #ARGS ); } }          \
    NS::_yOMM2_method ID(                                                     \
        ::yorel::yomm2::detail::discriminator,                                \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS),                            \
                        yOMM2_PLIST, ARGS));                                  \
    inline R ID(BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS),                    \
                                yOMM2_PLIST, ARGS)) {                         \
        return reinterpret_cast<R (*)(                                        \
            BOOST_PP_REPEAT(                                                  \
                BOOST_PP_TUPLE_SIZE(ARGS),                                    \
                yOMM2_PLIST, ARGS))>(                                         \
                    NS::_yOMM2_method::resolve( \
                        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS),            \
                                        yOMM2_ALIST, ARGS)))                  \
            (BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS),                       \
                             yOMM2_ALIST, ARGS));                             \
    }

#define YOMM2_DEFINE(RETURN_T, ID, ARGS)                                      \
    yOMM2_WITH_GENSYM(yOMM2_DEFINE, RETURN_T, ID, ARGS)

#define yOMM2_DEFINE(NS, RETURN_T, ID, ARGS)                                  \
    namespace {                                                               \
    namespace NS {                                                            \
        template<typename T> struct _yOMM2_select;                            \
    template<typename... A> struct _yOMM2_select<void(A...)> {                \
        using type = decltype(ID(::yorel::yomm2::detail::discriminator(),     \
                                 std::declval<A>()...));                      \
    };                                                                        \
    using _yOMM2_method = _yOMM2_select<void ARGS>::type;                     \
    using _yOMM2_return_t = _yOMM2_method::return_type;                       \
    _yOMM2_return_t (*next) ARGS;                                             \
    struct _yOMM2_spec { static RETURN_T body ARGS; };                        \
    ::yorel::yomm2::detail::                                                  \
    register_spec<_yOMM2_return_t, _yOMM2_method, _yOMM2_spec, void ARGS>     \
          _yOMM2_init((void**)&next YOMM2_TRACE_COMMA(#ARGS));                \
    } }                                                                       \
       RETURN_T NS::_yOMM2_spec::body ARGS

#define yOMM2_CLASS_NAME(CLASS, ...) \
    #CLASS

#define YOMM2_CLASS_(...)                                                     \
    yOMM2_WITH_GENSYM(yOMM2_CLASS2, __VA_ARGS__)

#define yOMM2_CLASS2(NS, REG, ...)                                            \
    namespace {                                                               \
       namespace NS {                                                \
           ::yorel::yomm2::detail::                                           \
           init_class_info                                                    \
           <REG yOMM2_CLASS_LIST(BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))     \
            > init YOMM2_TRACE( { yOMM2_CLASS_NAME(__VA_ARGS__ ) } ); } }

#define YOMM2_CLASS(...)                                                      \
    YOMM2_CLASS_(void, __VA_ARGS__)

#define yOMM2_CLIST(N, I, A)                                                  \
    , BOOST_PP_TUPLE_ELEM(I, A)

#define yOMM2_CLASS_LIST(TUPLE)                                               \
    BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(TUPLE),                               \
                    yOMM2_CLIST, TUPLE)                                       \

namespace yorel {
namespace yomm2 {

template<typename T>
struct virtual_;

void update_methods();

struct method_call_error {
    enum type { not_implemented = 0, ambiguous = 1 } code;
    YOMM2_TRACE(const char* method_name);
};

using method_call_error_handler = void (*)(const method_call_error& error);

method_call_error_handler set_method_call_error_handler(method_call_error_handler handler);

struct policy {
    struct hash_factors_in_globals {};
    struct hash_factors_in_vector {};
};

struct default_policy : policy {
    using hash_factors_placement = hash_factors_in_globals;
};

namespace detail {

extern method_call_error_handler call_error_handler;

struct method_info;
struct class_info;

union word {
    void* pf;
    const word* pw;
    int i;
    unsigned long ul;
};

struct registry {
    std::vector<const class_info*> classes;
    std::vector<method_info*> methods;
    template<typename T> static registry& get();
};

template<typename T> registry& registry::get() {
    static registry r;
    return r;
}

struct hash_function {
    std::uintptr_t mult;
    std::size_t shift;

    std::size_t operator ()(const void* p) const {
        return static_cast<std::size_t>(
            (mult * reinterpret_cast<std::uintptr_t>(const_cast<void*>(p)))
            >> shift);
    }
};


inline std::size_t hash(std::uintptr_t mult, std::size_t shift, const void* p) {
    return static_cast<std::size_t>(
        (mult * reinterpret_cast<std::uintptr_t>(const_cast<void*>(p)))
        >> shift);
}

struct dispatch_data {
    // global vector:
    std::vector<word> gv;
    hash_function hash;
    template<typename T> static dispatch_data instance;
};

template<typename T>
dispatch_data dispatch_data::instance;

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
    using base_type = std::remove_cv_t<T>;
    using argument_type = T&;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(argument_type arg) {
        return &typeid(arg);
    }

    template<class DERIVED>
    static DERIVED cast(T& obj, static_cast_) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static DERIVED cast(T& obj, dynamic_cast_) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct virtual_traits< virtual_<T*> > {
    using base_type = std::remove_cv_t<T>;
    using argument_type = T*;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(argument_type arg) {
        return &typeid(*arg);
    }

    template<class DERIVED>
    static DERIVED cast(T* obj, static_cast_) {
        static_assert(std::is_pointer<DERIVED>::value);
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static DERIVED cast(T* obj, dynamic_cast_) {
        static_assert(std::is_pointer<DERIVED>::value);
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct shared_ptr_traits {
    static const bool is_shared_ptr = false;
};

template<typename T>
struct shared_ptr_traits< std::shared_ptr<T> > {
    static const bool is_shared_ptr = true;
    using base_type = T;
};

template<typename T>
struct virtual_traits< virtual_< std::shared_ptr<T> > > {
    using base_type = std::remove_cv_t<T>;
    using argument_type = std::shared_ptr<T>;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(argument_type arg) {
        return &typeid(*arg);
    }

    template<class DERIVED>
    static DERIVED cast(argument_type obj, static_cast_) {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            std::is_class<typename shared_ptr_traits<DERIVED>::base_type>::value);
        return std::static_pointer_cast<typename shared_ptr_traits<DERIVED>::base_type>(obj);
    }

    template<class DERIVED>
    static DERIVED cast(argument_type obj, dynamic_cast_) {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            std::is_class<typename shared_ptr_traits<DERIVED>::base_type>::value);
        return std::dynamic_pointer_cast<typename shared_ptr_traits<DERIVED>::base_type>(obj);
    }
};

template<typename T>
using virtual_base_t = typename virtual_traits<T>::base_type;

template<typename T>
using virtual_arg_t = typename virtual_traits<T>::argument_type;

struct discriminator {};

inline const word* mptr(const dispatch_data& t, const std::type_info* ti) {
    return t.gv[t.hash(ti)].pw;
}

YOMM2_TRACE(std::ostream& log());
YOMM2_TRACE(std::ostream* log_on(std::ostream* os));
YOMM2_TRACE(std::ostream* log_off());

struct class_info {
    std::vector<const class_info*> direct_bases;
    YOMM2_TRACE(const char* name);
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
    init_class_info(YOMM2_TRACE(const char* name)) {
        auto& info = class_info::get<REG, CLASS>();
        static int called;
        if (!called++) {
            YOMM2_TRACE(info.name = name);
            registry::get<REG>().classes.push_back(&info);
            info.direct_bases = { &class_info::get<REG, BASE>()... };
        }
        auto inserted = info.ti_ptrs.insert(&typeid(CLASS));
        YOMM2_TRACE(
            if (inserted.second)
                ::yorel::yomm2::detail::log()
                      << "Register " << name
                      << " with &typeid " << &typeid(CLASS)
                      << " (" << typeid(CLASS).name() << ")"
                      << "\n");
    }
};

struct spec_info {
    YOMM2_TRACE(const char* name);
    std::vector<const class_info*> vp;
    void* pf;
    void** next;
};

struct method_info {
    YOMM2_TRACE(const char* name);
    std::vector<const class_info*> vp;
    std::vector<const spec_info*> specs;
    void* ambiguous;
    void* not_implemented;
    const word** slots_strides_p{nullptr};
    const std::type_info* hash_factors_placement;
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

template<int ARITY, typename... A>
struct resolver;

template<typename FIRST, typename... REST>
struct resolver<1, FIRST, REST...>
{
    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        return resolver<1, REST...>::resolve(
            hash_table, hash_mult, hash_shift, ssp, rest...);
    }
};

template<typename FIRST, typename... REST>
struct resolver<1, virtual_<FIRST>, REST...>
{
    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(
            detail::log() << "hash_table = " << hash_table
            << " slot = " << ssp->i << " key = " << key);
        auto mptr =
            hash_table[detail::hash(hash_mult, hash_shift, key)].pw;
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto pf = mptr[ssp->i].pf;
        YOMM2_TRACE(detail::log() << " pf = " << pf << "\n");
        return pf;
    }

    static void* resolve_next(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr =
            hash_table[detail::hash(hash_mult, hash_shift, key)].pw;
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        YOMM2_TRACE(detail::log() << " slot = " << slot);
        auto stride = ssp++->i;
        YOMM2_TRACE(detail::log() << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        YOMM2_TRACE(detail::log() << " dispatch = " << dispatch);
        auto pf = dispatch->pf;
        YOMM2_TRACE(detail::log() << " pf = " << pf << "\n");
        return pf;
    }
};

template<int ARITY, typename FIRST, typename... REST>
struct resolver<ARITY, FIRST, REST...>
{
    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest) {
        return resolver<ARITY, REST...>::resolve_first(
            hash_table, hash_mult, hash_shift, ssp, rest...);
    }

    static void* resolve_next(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        return resolver<ARITY, REST...>::resolve_next(
            hash_table, hash_mult, hash_shift, ssp, dispatch, rest...);
    }
};

template<int ARITY, typename FIRST, typename... REST>
struct resolver<ARITY, virtual_<FIRST>, REST...>
{
    static void* resolve_first(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr =
            hash_table[detail::hash(hash_mult, hash_shift, key)].pw;
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        YOMM2_TRACE(detail::log() << " slot = " << slot);
        auto dispatch = mptr[slot].pw;
        YOMM2_TRACE(detail::log() << " dispatch = " << dispatch << "\n");
        return resolver<ARITY - 1, REST...>::resolve_next(
            hash_table, hash_mult, hash_shift, ssp, dispatch, rest...);
    }

    static void* resolve_next(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        const word* dispatch,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr =
            hash_table[detail::hash(hash_mult, hash_shift, key)].pw;
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        YOMM2_TRACE(detail::log() << " slot = " << slot);
        auto stride = ssp++->i;
        YOMM2_TRACE(detail::log() << " stride = " << stride);
        dispatch += mptr[slot].i * stride;
        YOMM2_TRACE(detail::log() << " dispatch = " << dispatch << "\n");
        return resolver<ARITY - 1, REST...>::resolve_next(
            hash_table, hash_mult, hash_shift, ssp, dispatch, rest...);
    }

    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        virtual_arg_t<FIRST> first,
        virtual_arg_t<REST>... rest)
    {
        return resolve_first(
            hash_table, hash_mult, hash_shift, ssp, first, rest...);
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
            std::forward<virtual_arg_t<BASE_PARAM>>(arg),
            typename select_cast<
                virtual_base_t<BASE_PARAM>,
                virtual_base_t<SPEC_PARAM>>::type())...);
  }
};

template<typename RETURN_T, class METHOD, class SPEC, typename F>
struct register_spec;

template<typename RETURN_T, class METHOD, class SPEC, class... SPEC_ARGS>
struct register_spec<RETURN_T, METHOD, SPEC, void(SPEC_ARGS...)>
{
    register_spec(void** next YOMM2_TRACE_COMMA(const char* name)) {
        static spec_info si;
        if (si.vp.empty()) {
            YOMM2_TRACE(si.name = name);
            si.pf = (void*) wrapper<
                RETURN_T, SPEC, typename METHOD::signature_type, RETURN_T(SPEC_ARGS...)
                >::body;
            METHOD::for_each_vp_t::template for_spec<SPEC_ARGS...>::collect_class_info(si.vp);
            METHOD::info().specs.push_back(&si);
            si.next = next;
        }
    }
};

template<typename REG, typename ID, typename SIG, class POLICY>
struct method;

template<typename REG, typename ID, typename R, typename... A, typename POLICY>
struct method<REG, ID, R(A...), POLICY> {

    static const word* slots_strides; // slot 0, slot 1,  stride 1, slot 2, ...

    static method_info& info();

    using signature_type = R(A...);
    using return_type = R;
    using for_each_vp_t = for_each_vp<REG, A...>;

    enum { arity = for_each_vp_t::count };

    static  void* resolve(virtual_arg_t<A>... args) {
        return resolve(typename POLICY::hash_factors_placement(), args...);
    }

    static  void* resolve(policy::hash_factors_in_globals, virtual_arg_t<A>... args) {
        YOMM2_TRACE(detail::log() << "call " << name()
                    << " slots_strides = " << slots_strides << "\n");
        return resolver<arity, A...>::resolve(
            dispatch_data::instance<REG>.gv.data(),
            dispatch_data::instance<REG>.hash.mult,
            dispatch_data::instance<REG>.hash.shift,
            slots_strides, args...);
    }

    static void* resolve(policy::hash_factors_in_vector, virtual_arg_t<A>... args) {
        auto ssp = slots_strides;
        auto hash_table = ssp++->pw;
        auto hash_mult = ssp++->ul;
        auto hash_shift = ssp++->i;

        YOMM2_TRACE(detail::log() << "call " << name()
                    << " slots_strides = " << slots_strides << "\n");
        return resolver<arity, A...>::resolve(
            hash_table, hash_mult, hash_shift, ssp, args...);
    }

#if YOMM2_ENABLE_TRACE
    static const char* name() { return info().name; }
#endif

    static void not_implemented(virtual_arg_t<A>...) {
        method_call_error error;
        error.code = method_call_error::not_implemented;
        YOMM2_TRACE(error.method_name = info().name);
        detail::call_error_handler(error);
    }

    static void ambiguous(virtual_arg_t<A>...) {
        method_call_error error;
        error.code = method_call_error::ambiguous;
        YOMM2_TRACE(error.method_name = info().name);
        detail::call_error_handler(error);
    }

    struct init_method {
        init_method(YOMM2_TRACE(const char* name)) {
            if (info().vp.empty()) {
                YOMM2_TRACE(info().name = name);
                auto& inf = info();
                inf.slots_strides_p = &slots_strides;
                for_each_vp_t::collect_class_info(inf.vp);
                registry::get<REG>().methods.push_back(&inf);
                inf.not_implemented = (void*) not_implemented;
                inf.ambiguous = (void*) ambiguous;
                inf.hash_factors_placement =
                    &typeid(typename POLICY::hash_factors_placement);
            }
        }
    };
};

template<typename REG, typename ID, typename R, typename... A, typename POLICY>
const word* method<REG, ID, R(A...), POLICY>::slots_strides;

template<typename REG, typename ID, typename R, typename... A, typename POLICY>
method_info& method<REG, ID, R(A...), POLICY>::info() {
    static method_info info;
    return info;
}

void update_methods(const registry& reg, dispatch_data& dd);

} // namespace detail
} // namespace yomm2
} // namespace yorel


#endif
