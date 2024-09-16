// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICIES_CORE_HPP
#define YOREL_YOMM2_POLICIES_CORE_HPP

#include <yorel/yomm2/detail/types.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <functional>
#include <variant>

namespace yorel {
namespace yomm2 {
namespace detail {

template<class Name>
struct yOMM2_API_gcc yOMM2_API_msc method_tables {
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

// -----------------------------------------------------------------------------
// Error handling

struct error {};

struct resolution_error : error {
    enum status_type { no_definition = 1, ambiguous } status;
    type_id method;
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

namespace policies {

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
    static auto type_index(type_id type) -> type_id {
        return type;
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << "type_id(" << type << ")";
    }
};

struct deferred_static_rtti : virtual rtti {};

} // namespace policies

} // namespace yomm2
} // namespace yorel

#endif
