// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_INCLUDED
#define YOREL_YOMM2_INCLUDED

#include <algorithm> // IWYU pragma: keep
#include <cstdint>
#include <iostream>
#include <iterator> // IWYU pragma: keep
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/preprocessor/cat.hpp>                   
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
#define YOMM2_TRACE_ELSE(X, Y) X
#define YOMM2_TRACE_COMMA(X) , X

#else
#define YOMM2_TRACE(ST)
#define YOMM2_TRACE_ELSE(X, Y) Y
#define YOMM2_TRACE_COMMA(X)
#endif

#define yOMM2_GENSYM BOOST_PP_CAT(YoMm2_nS_, __COUNTER__)

#define yOMM2_PLIST(N, I, A)                                                  \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::detail::virtual_arg_t<BOOST_PP_TUPLE_ELEM(I, A)>          \
    BOOST_PP_CAT(a, I)

#define yOMM2_PPLIST(N, I, A)                                                 \
    BOOST_PP_COMMA_IF(I)                                                      \
    ::yorel::yomm2::detail::virtual_arg_t<BOOST_PP_TUPLE_ELEM(I, A)>*         \
    BOOST_PP_CAT(a, I)

#define yOMM2_ALIST(N, I, A) \
    BOOST_PP_COMMA_IF(I)     \
    std::forward<::yorel::yomm2::detail::virtual_arg_t<BOOST_PP_TUPLE_ELEM(I, A)>>(BOOST_PP_CAT(a, I))

#define yOMM2_RLIST(N, I, A) \
    BOOST_PP_COMMA_IF(I)     \
    BOOST_PP_CAT(a, I)

#define yOMM2_DECLARE_KEY(ID) \
    BOOST_PP_CAT(_yomm2_method_, ID)

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DECLARE(...)                         \
    BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__) \
    (__VA_ARGS__)
#define YOMM2_STATIC_DECLARE(...)                         \
    BOOST_PP_OVERLOAD(YOMM2_STATIC_DECLARE_, __VA_ARGS__) \
    (__VA_ARGS__)
#else
#define YOMM2_DECLARE(...) \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(YOMM2_DECLARE_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
#define YOMM2_STATIC_DECLARE(...) \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(YOMM2_STATIC_DECLARE_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
#endif

#define yOMM2_WHEN_STATIC(CODE1, CODE2) CODE1
#define yOMM2_WHEN_NOT_STATIC(CODE1, CODE2) CODE2

#define YOMM2_DECLARE_3(R, ID, ARGS) yOMM2_DECLARE(yOMM2_GENSYM, R, ID, ARGS, yOMM2_WHEN_NOT_STATIC, ::yorel::yomm2::default_policy)

#define YOMM2_DECLARE_4(R, ID, ARGS, POLICY) \
    yOMM2_DECLARE(yOMM2_GENSYM, R, ID, ARGS, yOMM2_WHEN_NOT_STATIC, POLICY)

#define YOMM2_STATIC_DECLARE_3(R, ID, ARGS) yOMM2_DECLARE(yOMM2_GENSYM, R, ID, ARGS, yOMM2_WHEN_STATIC, ::yorel::yomm2::default_policy)

#define YOMM2_STATIC_DECLARE_4(R, ID, ARGS, POLICY) \
    yOMM2_DECLARE(yOMM2_GENSYM, R, ID, ARGS, yOMM2_WHEN_STATIC, POLICY)

#define yOMM2_OPEN_BRACE {
#define yOMM2_CLOSE_BRACE }

#define yOMM2_SELECTOR(ID) ID ## _yOMM2_selector_

#define yOMM2_DECLARE(NS, R, ID, ARGS, IF_STATIC, POLICY)                         \
    struct yOMM2_DECLARE_KEY(ID);                                                 \
    IF_STATIC(, namespace yOMM2_OPEN_BRACE)                                       \
    struct NS                                                                     \
    {                                                                             \
        using _yOMM2_method = ::yorel::yomm2::detail::method<                     \
            yOMM2_DECLARE_KEY(ID), R ARGS, POLICY>;                               \
    };                                                                            \
    IF_STATIC(, yOMM2_CLOSE_BRACE)                                                \
    IF_STATIC(static, )                                                           \
    NS::_yOMM2_method yOMM2_SELECTOR(ID)(                                         \
        ::yorel::yomm2::detail::discriminator,                                    \
        BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS));           \
    IF_STATIC(static, )                                                           \
    inline const char *yOMM2_SELECTOR(ID)(NS::_yOMM2_method)                      \
    {                                                                             \
        return #R " " #ID #ARGS;                                                  \
    }                                                                             \
    IF_STATIC(static, )                                                           \
    inline R ID(BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_PLIST, ARGS))    \
    {                                                                             \
        auto pf = NS::_yOMM2_method::resolve(                                     \
            BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_RLIST, ARGS));       \
        return pf(BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ARGS), yOMM2_ALIST, ARGS)); \
    }

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DEFINE(...) \
    BOOST_PP_OVERLOAD(YOMM2_DEFINE_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_DEFINE(...) \
    BOOST_PP_CAT(         \
        BOOST_PP_OVERLOAD(YOMM2_DEFINE_, __VA_ARGS__)(__VA_ARGS__), BOOST_PP_EMPTY())
#endif

#define YOMM2_DEFINE_3(RETURN_T, ID, ARGS)                                    \
    yOMM2_DEFINE(yOMM2_GENSYM, RETURN_T, ID, ARGS)

#define yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS)                                           \
    template <typename T>                                                                 \
    struct _yOMM2_select;                                                                 \
    template <typename... A>                                                              \
    struct _yOMM2_select<void(A...)>                                                      \
    {                                                                                     \
        using type = decltype(yOMM2_SELECTOR(ID)(::yorel::yomm2::detail::discriminator(), \
                                                 std::declval<A>()...));                  \
    };                                                                                    \
    using _yOMM2_method = _yOMM2_select<void ARGS>::type;                                 \
    using _yOMM2_return_t = _yOMM2_method::return_type;                                   \
    inline _yOMM2_method::init_method _yOMM2_init_method(yOMM2_SELECTOR(ID)(_yOMM2_method()));

#define yOMM2_DEFINE(NS, RETURN_T, ID, ARGS)                                  \
    namespace {                                                               \
    namespace NS {                                                            \
    yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS);                                  \
    _yOMM2_method::function_pointer_type next;                                \
    struct _yOMM2_spec { static RETURN_T yOMM2_body ARGS; };                  \
    ::yorel::yomm2::detail::                                                  \
    register_spec<_yOMM2_return_t, _yOMM2_method, _yOMM2_spec, void ARGS>     \
    _yOMM2_init(                                                              \
        (void**)&next, YOMM2_TRACE_ELSE(#ARGS, typeid(_yOMM2_spec).name()));  \
    } }                                                                       \
    RETURN_T NS::_yOMM2_spec::yOMM2_body ARGS


#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_DECLARE_METHOD_CONTAINER(...)                                   \
    BOOST_PP_OVERLOAD(YOMM2_DECLARE_METHOD_CONTAINER_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_DECLARE_METHOD_CONTAINER(...)                                   \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(YOMM2_DECLARE_METHOD_CONTAINER_, __VA_ARGS__) \
                 (__VA_ARGS__), BOOST_PP_EMPTY())
#endif

#define YOMM2_DECLARE_METHOD_CONTAINER_1(CONTAINER)                           \
    template<typename S> struct CONTAINER

#define YOMM2_DECLARE_METHOD_CONTAINER_4(CONTAINER, RETURN_T, ID, ARGS)       \
    template<typename S> struct CONTAINER;                                    \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(yOMM2_GENSYM, CONTAINER, RETURN_T, ID, ARGS) \

#define YOMM2_DECLARE_METHOD_CONTAINER_4_NS(NS, CONTAINER, RETURN_T, ID, ARGS) \
    template<typename S> struct CONTAINER;                                    \
    namespace { namespace NS {                                                \
        yOMM2_SELECT_METHOD(RETURN_T, ID, ARGS);                              \
    } }                                                                       \
    template<> struct CONTAINER<RETURN_T ARGS> {                              \
        static NS::_yOMM2_method::function_pointer_type next;                 \
        static RETURN_T yOMM2_body ARGS;                                      \
    }

#define YOMM2_DEFINE_4(CONTAINER, RETURN_T, ID, ARGS)                         \
    yOMM2_DEFINE_METHOD_IN(yOMM2_GENSYM, , CONTAINER, RETURN_T, ID, ARGS)

#define YOMM2_DEFINE_INLINE(CONTAINER, RETURN_T, ID, ARGS)                    \
    yOMM2_DEFINE_METHOD_IN(yOMM2_GENSYM, inline, CONTAINER, RETURN_T, ID, ARGS)

#define yOMM2_DEFINE_METHOD_IN(NS, INLINE, CONTAINER, RETURN_T, ID, ARGS)                          \
    YOMM2_DECLARE_METHOD_CONTAINER_4_NS(NS, CONTAINER, RETURN_T, ID, ARGS);                        \
    INLINE NS::_yOMM2_method::function_pointer_type CONTAINER<RETURN_T ARGS>::next;                \
    namespace                                                                                      \
    {                                                                                              \
        namespace NS                                                                               \
        {                                                                                          \
            INLINE ::yorel::yomm2::detail::                                                        \
                register_spec<_yOMM2_return_t, _yOMM2_method, CONTAINER<RETURN_T ARGS>, void ARGS> \
                    _yOMM2_init(                                                                   \
                        (void **)&CONTAINER<RETURN_T ARGS>::next,                                  \
                        YOMM2_TRACE_ELSE(#ARGS, typeid(CONTAINER<RETURN_T ARGS>).name()));         \
        }                                                                                          \
    }                                                                                              \
    INLINE RETURN_T CONTAINER<RETURN_T ARGS>::yOMM2_body ARGS

#if !BOOST_PP_VARIADICS_MSVC
#define YOMM2_FRIEND(...)                                                     \
    BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)(__VA_ARGS__)
#else
#define YOMM2_FRIEND(...)                                                     \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(YOMM2_FRIEND_, __VA_ARGS__)                \
                 (__VA_ARGS__), BOOST_PP_EMPTY())
#endif

#define YOMM2_FRIEND_1(CONTAINER)                                             \
    template<typename> friend struct CONTAINER

#define YOMM2_FRIEND_3(CONTAINER, RETURN_T, ARGS)                             \
    friend struct CONTAINER<RETURN_T ARGS>

#define YOMM2_DEFINITION(CONTAINER, RETURN_T, ARGS)                           \
    CONTAINER<RETURN_T ARGS>::yOMM2_body

#define YOMM2_CLASS(...)                                                      \
    YOMM2_CLASS_(::yorel::yomm2::default_registry, __VA_ARGS__)

#define YOMM2_CLASS_(...)                                                     \
    yOMM2_CLASS2(yOMM2_GENSYM, BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__))

#define yOMM2_CLASS2(NS, TUPLE)                                               \
    namespace {                                                               \
    namespace NS {                                                            \
    ::yorel::yomm2::detail::                                                  \
    init_class_info                                                           \
    <BOOST_PP_TUPLE_REM(BOOST_PP_TUPLE_SIZE(TUPLE))TUPLE> init                \
    YOMM2_TRACE( {                                                            \
            BOOST_PP_STRINGIZE(                                               \
                BOOST_PP_TUPLE_ELEM(BOOST_PP_TUPLE_SIZE(TUPLE), 1, TUPLE)) }); } }

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

method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler);

struct default_registry; 

struct policy {
    struct hash_factors_in_globals {};
    struct hash_factors_in_vector {};
};

struct default_policy : policy {
    using hash_factors_placement = hash_factors_in_globals;
    using registry = default_registry;
};

// IWYU pragma: no_forward_declare default_registry
// IWYU pragma: no_forward_declare default_policy
 
namespace detail {

template<typename Signature>
struct next_ptr_t;

template<typename R, typename... T>
struct  next_ptr_t<R(T...)> {
    using type = R(*)(T...);
};

template<typename Method, typename Signature>
inline typename next_ptr_t<Signature>::type next;

extern method_call_error_handler call_error_handler;

struct method_info;
struct class_info;

union word {
    void* pf;
    const word* pw;
    int i;
    unsigned long ul;
    const void* ti;
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
    word* hash_table;
    hash_function hash;
    template<typename T>
    struct instance {
        static dispatch_data _;
    };
};

template<typename T>
dispatch_data dispatch_data::instance<T>::_;

#ifndef NDEBUG
void unregistered_class_error(const std::type_info* ti);
#endif

inline const word* get_mptr(
    const word* hash_table, std::uintptr_t hash_mult, std::size_t hash_shift,
    const std::type_info* ti) {
    auto index = detail::hash(hash_mult, hash_shift, ti);
        auto mptr = hash_table[index].pw;
#ifndef NDEBUG
        if (!mptr || hash_table[-1].pw[index].ti != ti) {
            unregistered_class_error(ti);
        }
#endif
        return mptr;
}

struct dynamic_cast_ {};
struct static_cast_ {};

template<typename T>
struct virtual_traits {
    using base_type = typename std::remove_cv_t<std::remove_reference_t<T>>;
    using argument_type = T;
    using resolve_type = const T&;
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
    using resolve_type = T&;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(resolve_type arg) {
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
struct virtual_traits< virtual_<T&&> > {
    using base_type = T;
    using argument_type = T&&;
    using resolve_type = T&;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(resolve_type arg) {
        return &typeid(arg);
    }

    template<class DERIVED>
    static DERIVED cast(T&& obj, static_cast_) {
        return static_cast<DERIVED>(obj);
    }

    template<class DERIVED>
    static DERIVED cast(T&& obj, dynamic_cast_) {
        return dynamic_cast<DERIVED>(obj);
    }
};

template<typename T>
struct virtual_traits< virtual_<T*> > {
    using base_type = std::remove_cv_t<T>;
    using argument_type = T*;
    using resolve_type = T*;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(resolve_type arg) {
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
    using resolve_type = const std::shared_ptr<T>&;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(resolve_type arg) {
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
struct virtual_traits< virtual_< const std::shared_ptr<T>& > > {
    using base_type = std::remove_cv_t<T>;
    using argument_type = const std::shared_ptr<T>&;
    using resolve_type = const std::shared_ptr<T>&;

    static_assert(std::is_class<base_type>::value);
    static_assert(std::is_polymorphic<base_type>::value);

    static auto key(resolve_type arg) {
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

template<typename T>
using resolve_arg_t = typename virtual_traits<T>::resolve_type;

struct discriminator {};

std::ostream& log();
std::ostream* log_on(std::ostream* os);
std::ostream* log_off();

struct class_info {
    std::vector<const class_info*> direct_bases;
    const char* name;
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
    static int refs;
    init_class_info(YOMM2_TRACE(const char* name)) {
        auto& info = class_info::get<REG, CLASS>();
        if (!refs++) {
            info.name = YOMM2_TRACE_ELSE(name, typeid(CLASS).name());
            registry::get<REG>().classes.push_back(&info);
            info.direct_bases = { &class_info::get<REG, BASE>()... };
        }
        auto inserted = info.ti_ptrs.insert(&typeid(CLASS));
        if (inserted.second)
            YOMM2_TRACE(
                ::yorel::yomm2::detail::log()
                      << "Register class " << name
                      << " with &typeid " << &typeid(CLASS)
                      << "\n");
    }

    ~init_class_info() {
        auto& info = class_info::get<REG, CLASS>();
        auto iter = info.ti_ptrs.find(&typeid(CLASS));

        if (iter != info.ti_ptrs.end()) {
            YOMM2_TRACE(
                ::yorel::yomm2::detail::log()
                << "Un-register " << info.name
                << " with &typeid " << &typeid(CLASS)
                << "\n");

            info.ti_ptrs.erase(iter);
        }

        if (!--refs) {
            auto& info = class_info::get<REG, CLASS>();
            auto& classes = registry::get<REG>().classes;
            classes.erase(std::find(classes.begin(), classes.end(), &info));
        }
    }
};

template<typename REG, class CLASS, class... BASE>
int init_class_info<REG, CLASS, BASE...>::refs;

struct spec_info {
    const char* name;
    std::vector<const class_info*> vp;
    void* pf;
    void** next;
};

struct method_info {
    bool initialized;
    const char* name = "(a method)";
    std::vector<const class_info*> vp;
    std::vector<const spec_info*> specs;
    void* ambiguous;
    void* not_implemented;
    //const word** slots_strides_p{nullptr};
    word* slots_strides_p{nullptr};
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
        word ss,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest) {
        return resolver<1, REST...>::resolve(
            hash_table, hash_mult, hash_shift, ss, rest...);
    }

    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest) {
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
        word ss,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest) {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(
            detail::log() << "hash_table = " << hash_table
            << " slot = " << ss.i << " key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto pf = mptr[ss.i].pf;
        YOMM2_TRACE(detail::log() << " pf = " << pf << "\n");
        return pf;
    }

    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest) {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(
            detail::log() << "hash_table = " << hash_table
            << " slot = " << ssp->i << " key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
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
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
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
        word ss,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest) {
        return resolver<ARITY, REST...>::resolve_first(
            hash_table, hash_mult, hash_shift, ss, rest...);
    }

    static void* resolve_next(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        const word* dispatch,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
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
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
        YOMM2_TRACE(detail::log() << " mptr = " << mptr);
        auto slot = ssp++->i;
        YOMM2_TRACE(detail::log() << " slot = " << slot);
        auto dispatch = mptr[slot].pw;
        YOMM2_TRACE(detail::log() << " dispatch = " << dispatch << "\n");
        return resolver<ARITY - 1, REST...>::resolve_next(
            hash_table, hash_mult, hash_shift, ssp, dispatch, rest...);
    }

    static void* resolve_first(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        word ss,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
    {
        auto ssp = ss.pw;
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
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
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
    {
        auto key = virtual_traits<virtual_<FIRST>>::key(first);
        YOMM2_TRACE(detail::log() << "  key = " << key);
        auto mptr = detail::get_mptr(hash_table, hash_mult, hash_shift, key);
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
        word ss,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
    {
        return resolve_first(
            hash_table, hash_mult, hash_shift, ss, first, rest...);
    }

    static void* resolve(
        const word* hash_table,
        std::uintptr_t hash_mult,
        std::size_t hash_shift,
        const word* ssp,
        resolve_arg_t<FIRST> first,
        resolve_arg_t<REST>... rest)
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
struct wrapper<
    BASE_RETURN,
    FUNCTION,
    BASE_RETURN(BASE_PARAM...),
    BASE_RETURN(SPEC_PARAM...)> {
    static BASE_RETURN call(virtual_arg_t<BASE_PARAM>... arg) {
    return FUNCTION::yOMM2_body(
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
    static spec_info* this_;

    register_spec(void** next, const char* name) {
        static spec_info si;
        if (si.vp.empty()) {
            this_ = &si;
            si.name = name;
            YOMM2_TRACE(
                log() << METHOD::name() << ": add spec " << name << "\n");
            si.pf = (void*) wrapper<
                RETURN_T, SPEC, typename METHOD::signature_type, RETURN_T(SPEC_ARGS...)
                >::call;
            METHOD::for_each_vp_t::template for_spec<SPEC_ARGS...>::collect_class_info(si.vp);
            METHOD::info().specs.push_back(&si);
            si.next = next;
        }
    }

    ~register_spec() {
        auto& specs = METHOD::info().specs;
        auto iter = std::find(specs.begin(), specs.end(), this_);
        if (iter != specs.end()) {
            YOMM2_TRACE(
                log() << METHOD::name() << ": remove spec "
                << (*iter)->name << "\n");
            specs.erase(iter);
        }
    }
};

template<typename RETURN_T, class METHOD, class SPEC, class... SPEC_ARGS>
spec_info* register_spec<RETURN_T, METHOD, SPEC, void(SPEC_ARGS...)>::this_;

template<typename ID, typename SIG, class POLICY>
struct method;

template<typename ID, typename R, typename... A, typename POLICY>
struct method<ID, R(A...), POLICY> {

    static word slots_strides; // slot 0, slot 1,  stride 1, slot 2, ...

    static method_info& info();

    using signature_type = R(A...);
    using return_type = R;
    using function_pointer_type = R (*)(virtual_arg_t<A>...);
    using REG = typename POLICY::registry;
    using for_each_vp_t = for_each_vp<REG, A...>;

    enum { arity = for_each_vp_t::count };

    static function_pointer_type resolve(resolve_arg_t<A>... args) {
        YOMM2_TRACE(detail::log() << "call " << name() << "\n");
        return reinterpret_cast<function_pointer_type>(
            resolve(
                typename POLICY::hash_factors_placement(), args...));
    }

    static void* resolve(policy::hash_factors_in_globals, resolve_arg_t<A>... args) {
        return resolver<arity, A...>::resolve(
            dispatch_data::instance<REG>::_.hash_table,
            dispatch_data::instance<REG>::_.hash.mult,
            dispatch_data::instance<REG>::_.hash.shift,
            slots_strides, args...);
    }

    static void* resolve(policy::hash_factors_in_vector, resolve_arg_t<A>... args) {
        auto ssp = slots_strides.pw;
        auto hash_table = ssp++->pw;
        auto hash_mult = ssp++->ul;
        auto hash_shift = ssp++->i;

        YOMM2_TRACE(detail::log() << "call " << name() << "\n");
        return resolver<arity, A...>::resolve(
            hash_table, hash_mult, hash_shift, ssp, args...);
    }

#if YOMM2_ENABLE_TRACE
    static const char* name() { return info().name; }
#endif

    static void not_implemented(virtual_arg_t<A>...) {
        method_call_error error;
        error.code = method_call_error::not_implemented;
        YOMM2_TRACE(error.method_name = name());
        detail::call_error_handler(error);
    }

    static void ambiguous(virtual_arg_t<A>...) {
        method_call_error error;
        error.code = method_call_error::ambiguous;
        YOMM2_TRACE(error.method_name = name());
        detail::call_error_handler(error);
    }

    struct init_method {
        init_method(const char* name) {
            auto& inf = info();
            if (!inf.initialized) {
                YOMM2_TRACE(log() << "Register method " << name << "\n");
                inf.initialized = true;
                inf.name = name;
                inf.slots_strides_p = &slots_strides;
                for_each_vp_t::collect_class_info(inf.vp);
                registry::get<REG>().methods.push_back(&inf);
                inf.not_implemented = (void*) not_implemented;
                inf.ambiguous = (void*) ambiguous;
                inf.hash_factors_placement =
                    &typeid(typename POLICY::hash_factors_placement);
            }
        }

        ~init_method() {
            auto& methods = registry::get<REG>().methods;
            auto iter = std::find(methods.begin(), methods.end(), &info());
            if (iter != methods.end()) {
                YOMM2_TRACE(log() << "Un-register method " << name() << "\n");
                methods.erase(iter);
            }
        }
    };
};

template<typename ID, typename R, typename... A, typename POLICY>
word method<ID, R(A...), POLICY>::slots_strides;

template<typename ID, typename R, typename... A, typename POLICY>
method_info& method<ID, R(A...), POLICY>::info() {
    static method_info info;
    return info;
}

void update_methods(const registry& reg, dispatch_data& dd);

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
