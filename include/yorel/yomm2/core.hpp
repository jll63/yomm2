#ifndef YOREL_YOMM2_CORE_INCLUDED
#define YOREL_YOMM2_CORE_INCLUDED

#include <chrono>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

#if defined(YOMM2_TRACE) && (YOMM2_TRACE & 2)
    #include <iostream>
#else
    #include <iosfwd>
#endif

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/type_traits/is_virtual_base_of.hpp>

#if defined(YOMM2_SHARED)
    #if !defined(yOMM2_API)
        #if defined(_WIN32)
            #define yOMM2_API __declspec(dllimport)
        #else
            #define yOMM2_API
        #endif
    #endif
#else
    #define yOMM2_API
#endif

namespace yorel {
namespace yomm2 {

template<typename T>
struct virtual_;

template<typename... Types>
struct types;

struct resolution_error {
    enum status_type { no_definition = 1, ambiguous } status;
    const std::type_info* method;
    size_t arity;
    const std::type_info* const* tis;
};

struct unknown_class_error {
    enum { update = 1, call } context;
    const std::type_info* ti;
};

struct hash_search_error {
    size_t attempts;
    std::chrono::duration<double> duration;
    size_t buckets;
};

using error_type =
    std::variant<resolution_error, unknown_class_error, hash_search_error>;

using error_handler_type = void (*)(const error_type& error);

yOMM2_API error_handler_type set_error_handler(error_handler_type handler);

struct catalog;
struct context;

namespace policy {

struct abstract_policy;
struct hash_factors_in_method;
using default_policy = hash_factors_in_method;

} // namespace policy

template<typename ClassList, typename Policy>
struct class_declaration;

} // namespace yomm2
} // namespace yorel

#include <yorel/yomm2/detail.hpp>

namespace yorel {
namespace yomm2 {

// deprecated

struct method_call_error {
    resolution_error::status_type code;
    static constexpr auto not_implemented = resolution_error::no_definition;
    static constexpr auto ambiguous = resolution_error::ambiguous;
    std::string_view method_name;
};

using method_call_error_handler = void (*)(
    const method_call_error& error, size_t arity,
    const std::type_info* const tis[]);

yOMM2_API method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler);

// end deprecated

template<typename T>
struct virtual_;

template<typename... Types>
struct types;

struct catalog {
    catalog& add(detail::class_info& cls) {
        classes.push_front(cls);
        return *this;
    }

    static_chain<detail::class_info> classes;
    static_chain<detail::method_info> methods;
};

struct context {
    std::vector<detail::word> gv;
    detail::word* hash_table;
    detail::hash_function hash;
};

namespace policy {

struct abstract_policy {};

struct yOMM2_API global_context : virtual abstract_policy {
    static struct context context;
};

struct yOMM2_API global_catalog : virtual abstract_policy {
    static struct catalog catalog;
};

struct hash_factors_in_globals : global_catalog, global_context {
    using method_info_type = detail::method_info;
    template<typename Method, typename... T>
    static void* resolve(const Method& method, T... args) {
        return method.resolve(
            context.hash_table, context.hash.mult, context.hash.shift,
            method.slots_strides, args...);
    }
};

struct hash_factors_in_method : global_catalog, global_context {
    struct yOMM2_API method_info_type : detail::method_info {
        detail::hash_function hash;
        void install_hash_factors(detail::runtime& rt) override;
    };

    template<typename Method, typename... T>
    static void* resolve(const Method& method, T... args) {
        return method.resolve(
            method.hash_table, method.hash.mult, method.hash.shift,
            method.slots_strides, args...);
    }
};

using default_policy = hash_factors_in_method;

} // namespace policy

template<
    typename Key, typename Signature, class Policy = policy::default_policy>
struct method;

template<typename Key, typename R, typename... A, class Policy>
struct method<Key, R(A...), Policy> : Policy::method_info_type {
    using self_type = method;
    using declared_argument_types = types<A...>;
    using call_argument_types = boost::mp11::mp_transform<
        detail::remove_virtual, declared_argument_types>;
    using virtual_argument_types =
        typename detail::polymorphic_types<declared_argument_types>;
    using signature_type = R(A...);
    using return_type = R;
    using function_pointer_type = R (*)(detail::remove_virtual<A>...);
    using next_type = function_pointer_type;

    enum { arity = boost::mp11::mp_size<virtual_argument_types>::value };
    static_assert(arity > 0, "method must have at least one virtual parameter");

    explicit method(std::string_view name = typeid(method).name()) {
        this->name = name;
        using virtual_type_ids = detail::type_id_list<boost::mp11::mp_transform<
            detail::polymorphic_type, virtual_argument_types>>;
        this->vp_begin = virtual_type_ids::begin;
        this->vp_end = virtual_type_ids::end;
        this->not_implemented = (void*)not_implemented_handler;
        this->ambiguous = (void*)ambiguous_handler;
        this->hash_factors_placement = &typeid(Policy);
        Policy::catalog.methods.push_front(*this);
    }

    method(const method&) = delete;
    method(method&&) = delete;

    ~method() {
        Policy::catalog.methods.remove(*this);
    }

    function_pointer_type resolve(detail::resolver_type<A>... args) const {
        return reinterpret_cast<function_pointer_type>(
            Policy::template resolve<method, detail::resolver_type<A>...>(
                *this, args...));
    }

    void* resolve(
        const detail::word* hash_table, std::uintptr_t hash_mult,
        std::size_t hash_shift, detail::word ss,
        detail::resolver_type<A>... args) const {
        auto pf = (detail::resolve_init{hash_table, hash_mult, hash_shift, ss}
                   << ... << detail::resolve_arg<arity, A>{args})
                      .dispatch->pf;

        if constexpr (bool(detail::trace_enabled & detail::TRACE_CALLS)) {
            detail::call_trace << " pf = " << pf;
        }

        return pf;
    }

    return_type operator()(detail::remove_virtual<A>... args) {
        if constexpr (bool(detail::trace_enabled & detail::TRACE_CALLS)) {
            detail::call_trace << "call " << this->name;
        }

        return reinterpret_cast<function_pointer_type>(resolve(args...))(
            std::forward<detail::remove_virtual<A>>(args)...);
    }

    static method fn;
    static function_pointer_type fake_definition;

    static return_type
    not_implemented_handler(detail::remove_virtual<A>... args) {
        resolution_error error;
        error.status = resolution_error::no_definition;
        error.method = &typeid(method);
        detail::ti_ptr tis[sizeof...(args)];
        auto ti_iter = tis;
        (..., (*ti_iter++ = detail::universal_traits<A>::key(args)));
        detail::error_handler(error_type(std::move(error)));
        abort(); // in case user handler "forgets" to abort
    }

    static return_type ambiguous_handler(detail::remove_virtual<A>... args) {
        resolution_error error;
        error.status = resolution_error::ambiguous;
        error.method = &typeid(method);
        detail::ti_ptr tis[sizeof...(args)];
        auto ti_iter = tis;
        (..., (*ti_iter++ = detail::universal_traits<A>::key(args)));
        detail::error_handler(error_type(std::move(error)));
        abort(); // in case user handler "forgets" to abort
    }

    template<typename Container>
    using next = detail::next_aux<method, Container>;

    template<auto Function>
    struct add_function {
        explicit add_function(
            next_type* next = nullptr,
            std::string_view name = typeid(Function).name()) {

            static detail::definition_info info;

            if (info.method) {
                assert(info.method == &fn);
                return;
            }

            info.method = &fn;
            info.name = name;
            info.next = reinterpret_cast<void**>(next);
            using parameter_types = detail::parameter_type_list_t<Function>;
            info.pf = (void*)&detail::wrapper<
                signature_type, Function, parameter_types>::fn;
            using spec_type_ids =
                detail::type_id_list<detail::spec_polymorphic_types<
                    declared_argument_types, parameter_types>>;
            info.vp_begin = spec_type_ids::begin;
            info.vp_end = spec_type_ids::end;
            fn.specs.push_front(info);
        }
    };

    template<typename Container, bool has_next, bool has_name>
    struct add_definition_;

    template<typename Container>
    struct add_definition_<Container, false, false> {
        add_function<Container::fn> override_{
            nullptr, typeid(Container).name()};
    };

    template<typename Container>
    struct add_definition_<Container, true, false> {
        add_function<Container::fn> add{
            &Container::next, typeid(Container).name()};
    };

    template<typename Container>
    struct add_definition_<Container, false, true> {
        add_function<Container::fn> add{nullptr, Container::name};
    };

    template<typename Container>
    struct add_definition_<Container, true, true> {
        add_function<Container::fn> add{&Container::next, Container::name};
    };

    template<typename Container>
    struct add_definition
        : add_definition_<
              Container, detail::has_next_v<Container, next_type>,
              detail::has_name_v<Container>> {
        using type = add_definition; // make it a meta-function
    };

    template<auto F>
    struct add_member_function
        : add_function<detail::member_function_wrapper<F, decltype(F)>::fn> {};

    template<auto... F>
    struct add_member_functions {
        std::tuple<add_member_function<F>...> add;
    };

    template<typename Container>
    struct use_next {
        static next_type next;
    };
};

template<typename Key, typename R, typename... A, class Policy>
method<Key, R(A...), Policy> method<Key, R(A...), Policy>::fn;

template<typename Key, typename R, typename... A, class Policy>
typename method<Key, R(A...), Policy>::function_pointer_type
    method<Key, R(A...), Policy>::fake_definition;

template<typename Key, typename R, typename... A, class Policy>
template<typename Container>
typename method<Key, R(A...), Policy>::next_type
    method<Key, R(A...), Policy>::use_next<Container>::next;

template<typename ClassList, typename Policy = policy::default_policy>
struct class_declaration : detail::class_info {
    // Add a class to a catalog.
    // There is a possibility that the same class is registered with
    // different bases. This will be caught by augment_classes.

    using class_type = boost::mp11::mp_front<ClassList>;
    using bases_type = boost::mp11::mp_pop_front<ClassList>;

    class_declaration() {
        // TODO: make following work for root classes.
        // static_assert(std::conjunction_v<std::is_base_of<CLASS, BASE...>>);
        // runtime can cope with multiple class_info's for the same class, but
        // let's avoid growing the class list too much, in case someone puts
        // a class registration in a header file.
        ti = &typeid(class_type);
        first_base = detail::type_id_list<bases_type>::begin;
        last_base = detail::type_id_list<bases_type>::end;
        Policy::catalog.classes.push_front(*this);
        is_abstract = std::is_abstract_v<class_type>;
    }

    ~class_declaration() {
        Policy::catalog.classes.remove(*this);
    }
};

template<typename... T>
using use_classes = typename detail::use_classes_aux<T...>::type;

yOMM2_API void update_methods();

} // namespace yomm2
} // namespace yorel

#endif
