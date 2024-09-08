// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_HPP
#define YOREL_YOMM2_POLICY_HPP

#include <functional>
#include <memory>
#include <variant>
#include <vector>

#include <yorel/yomm2/policies/core.hpp>

#include <yorel/yomm2/detail.hpp>

#include <yorel/yomm2/policies/minimal_rtti.hpp>
#include <yorel/yomm2/policies/std_rtti.hpp>
#include <yorel/yomm2/policies/vptr_vector.hpp>
#include <yorel/yomm2/policies/vptr_map.hpp>
#include <yorel/yomm2/policies/basic_indirect_vptr.hpp>
#include <yorel/yomm2/policies/basic_error_output.hpp>
#include <yorel/yomm2/policies/basic_trace_output.hpp>
#include <yorel/yomm2/policies/fast_perfect_hash.hpp>
#include <yorel/yomm2/policies/vectored_error.hpp>

#ifndef BOOST_NO_EXCEPTIONS
#include <yorel/yomm2/policies/throw_error.hpp>
#endif

namespace yorel {
namespace yomm2 {
namespace policy {

template<class Policy>
struct yOMM2_API_gcc backward_compatible_error_handler
    : vectored_error<Policy, backward_compatible_error_handler<Policy>> {
    static method_call_error_handler call_error;

    static void default_error_handler(const error_type& error_v) {
        using namespace detail;

        if (auto err = std::get_if<resolution_error>(&error_v)) {
            method_call_error old_error;
            old_error.code = err->status;
            old_error.method_name = err->method_name;
            call_error(std::move(old_error), err->arity, (type_id*)err->types);
            abort();
        }

        vectored_error<Policy>::default_error_handler(error_v);
    }

    static void default_call_error_handler(
        const method_call_error& error, std::size_t arity, type_id* ti_ptrs) {

        using namespace policy;

        if constexpr (Policy::template has_facet<error_output>) {
            const char* explanation[] = {
                "no applicable definition", "ambiguous call"};
            Policy::error_stream
                << explanation[error.code - resolution_error::no_definition]
                << " for " << error.method_name << "(";
            auto comma = "";

            for (auto ti : detail::range{ti_ptrs, ti_ptrs + arity}) {
                Policy::error_stream << comma;
                Policy::type_name(ti, Policy::error_stream);
                comma = ", ";
            }

            Policy::error_stream << ")\n";
        }

        abort();
    }
};

template<class Policy>
method_call_error_handler
    backward_compatible_error_handler<Policy>::call_error =
        backward_compatible_error_handler<Policy>::default_call_error_handler;

struct yOMM2_API_gcc release
    : basic_policy<
          release, std_rtti, fast_perfect_hash<release>, vptr_vector<release>,
          backward_compatible_error_handler<release>> {};

struct yOMM2_API_gcc debug
    : basic_policy<
          debug, std_rtti, checked_perfect_hash<debug>, vptr_vector<debug>,
          basic_error_output<debug>, basic_trace_output<debug>,
          backward_compatible_error_handler<debug>> {};

#if defined(_MSC_VER) && !defined(yOMM2_DLL)
extern template class __declspec(dllimport) basic_domain<debug_shared>;
extern template class __declspec(dllimport) vptr_vector<debug_shared>;
extern template class __declspec(dllimport)
vectored_error<debug_shared, backward_compatible_error_handler<debug_shared>>;
extern template class __declspec(dllimport) fast_perfect_hash<debug_shared>;
extern template class __declspec(dllimport) checked_perfect_hash<debug_shared>;
extern template class __declspec(dllimport)
basic_trace_output<debug_shared, detail::ostderr>;
extern template class __declspec(dllimport)
basic_error_output<debug_shared, detail::ostderr>;
extern template class __declspec(dllimport) checked_perfect_hash<debug_shared>;
extern template class __declspec(dllimport)
backward_compatible_error_handler<debug_shared>;
extern template class __declspec(dllimport) basic_policy<
    debug_shared, vptr_vector<debug_shared>, std_rtti,
    checked_perfect_hash<debug_shared>, basic_error_output<debug_shared>,
    basic_trace_output<debug_shared>,
    backward_compatible_error_handler<debug_shared>>;
#endif

#ifndef BOOST_NO_RTTI
struct yOMM2_API_gcc debug_shared
    : basic_policy<
          debug_shared, std_rtti, checked_perfect_hash<debug_shared>,
          vptr_vector<debug_shared>, basic_error_output<debug_shared>,
          basic_trace_output<debug_shared>,
          backward_compatible_error_handler<debug_shared>> {};

struct yOMM2_API_gcc release_shared : debug_shared {
    template<class Class>
    static auto dynamic_vptr(const Class& arg) -> const std::uintptr_t* {
        auto index = dynamic_type(arg);
        index = fast_perfect_hash<debug_shared>::hash_type_id(index);
        return vptrs[index];
    }
};
#endif

#ifdef NDEBUG
using default_static = policy::release;
#else
using default_static = policy::debug;
#endif

} // namespace policy

#if defined(YOMM2_SHARED)
#ifdef NDEBUG
using default_policy = policy::release_shared;
#else
using default_policy = policy::debug_shared;
#endif
#else
using default_policy = policy::default_static;
#endif

} // namespace yomm2
} // namespace yorel

#endif
