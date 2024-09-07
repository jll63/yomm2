// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICIES_CORE_HPP
#define YOREL_YOMM2_POLICIES_CORE_HPP

#include <yorel/yomm2/detail/types.hpp>
#include <yorel/yomm2/detail/static_list.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <string_view>

namespace yorel {
namespace yomm2 {
namespace detail {

// -----------------------------------------------------------------------------
// class info

struct class_info : static_list<class_info>::static_link {
    type_id type;
    std::uintptr_t** static_vptr;
    type_id *first_base, *last_base;
    bool is_abstract{false};

    const std::uintptr_t* vptr() const {
        return *static_vptr;
    }

    const std::uintptr_t* const* indirect_vptr() const {
        return static_vptr;
    }

    auto type_id_begin() const {
        return &type;
    }

    auto type_id_end() const {
        return &type + 1;
    }
};

// -----------
// method info

struct method_info;

struct definition_info : static_list<definition_info>::static_link {
    ~definition_info();
    method_info* method; // for the destructor, to remove definition
    type_id type;        // of the function, for trace
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

template<class Name>
struct yOMM2_API_gcc yOMM2_API_msc method_tables {
    // Why is yOMM2_API_msc needed here???
    template<class Class>
    static std::uintptr_t* static_vptr;
};

template<class Name>
template<class Class>
std::uintptr_t* method_tables<Name>::static_vptr;

using class_catalog = detail::static_list<detail::class_info>;
using method_catalog = detail::static_list<detail::method_info>;

struct domain {};

} // namespace detail

template<class Class, class Policy>
struct virtual_ptr;

template<typename T>
struct virtual_;

template<class Policy, typename Name, typename Signature>
struct method;

template<class... Classes>
struct class_declaration;

// -----------------------------------------------------------------------------
// Error handling

struct error {};

struct resolution_error : error {
    enum status_type { no_definition = 1, ambiguous } status;
    std::string_view method_name;
    std::size_t arity;
    static constexpr std::size_t max_types = 16;
    type_id types[max_types];
};

struct unknown_class_error : error {
    enum { update = 1, call } context;
    type_id type;
};

struct hash_search_error : error {
    std::size_t attempts;
    std::size_t buckets;
};

struct method_table_error : error {
    type_id type;
};

struct static_offset_error : error {
    type_id method;
    int actual, expected;
};

struct static_slot_error : static_offset_error {};
struct static_stride_error : static_offset_error {};

using error_type = std::variant<
    error, resolution_error, unknown_class_error, hash_search_error,
    method_table_error, static_slot_error, static_stride_error>;

using error_handler_type = std::function<void(const error_type& error)>;

// deprecated

struct method_call_error {
    resolution_error::status_type code;
    static constexpr auto not_implemented = resolution_error::no_definition;
    static constexpr auto ambiguous = resolution_error::ambiguous;
    std::string_view method_name;
};

using method_call_error_handler =
    void (*)(const method_call_error& error, std::size_t arity, type_id* types);

namespace policy {

struct abstract_policy {};

struct error_handler {};
struct runtime_checks {};
struct indirect_vptr {};
struct type_hash {};
struct vptr_placement {};
struct external_vptr : virtual vptr_placement {};
struct error_output {};
struct trace_output {};

struct deferred_static_rtti;
struct debug;
struct release;
struct debug_shared;
struct release_shared;

template<class Name>
struct yOMM2_API_gcc basic_domain : detail::domain,
                                    detail::method_tables<Name> {
    static detail::class_catalog classes;
    static detail::method_catalog methods;
    static std::vector<std::uintptr_t> dispatch_data;
};

template<class Name>
detail::class_catalog basic_domain<Name>::classes;

template<class Name>
detail::method_catalog basic_domain<Name>::methods;

template<class Name>
std::vector<std::uintptr_t> basic_domain<Name>::dispatch_data;

template<typename Policy, class Facet>
struct rebind_facet {
    using type = Facet;
};

template<
    typename NewPolicy, typename OldPolicy,
    template<typename...> class GenericFacet, typename... Args>
struct rebind_facet<NewPolicy, GenericFacet<OldPolicy, Args...>> {
    using type = GenericFacet<NewPolicy, Args...>;
};

template<class Policy, class... Facets>
struct basic_policy : virtual abstract_policy,
                      virtual basic_domain<Policy>,
                      virtual Facets... {
    using facets = detail::types<Facets...>;

    template<class Facet>
    static constexpr bool has_facet = std::is_base_of_v<Facet, Policy>;

    template<class Facet>
    using use_facet = boost::mp11::mp_first<boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_front_q<
            boost::mp11::mp_quote_trait<std::is_base_of>, Facet>,
        facets>>;

    template<class NewPolicy>
    using rebind = basic_policy<
        NewPolicy, typename rebind_facet<NewPolicy, Facets>::type...>;

    template<class Base, class Facet>
    using replace = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            boost::mp11::mp_replace_if_q<
                facets,
                boost::mp11::mp_bind_front_q<
                    boost::mp11::mp_quote_trait<std::is_base_of>, Base>,
                Facet>,
            Policy>>;

    template<class Base>
    using remove = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            boost::mp11::mp_remove_if_q<
                facets,
                boost::mp11::mp_bind_front_q<
                    boost::mp11::mp_quote_trait<std::is_base_of>, Base>>,
            Policy>>;
};

template<class Policy, class Facet>
constexpr bool has_facet = Policy::template has_facet<Facet>;

struct rtti {
    static type_id type_index(type_id type) {
        return type;
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << "type_id(" << type << ")";
    }
};

struct deferred_static_rtti : virtual rtti {};

} // namespace policy

} // namespace yomm2
} // namespace yorel

#endif
