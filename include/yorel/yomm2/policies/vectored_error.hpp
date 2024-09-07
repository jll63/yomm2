
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_VECTORED_ERROR_HPP
#define YOREL_YOMM2_POLICY_VECTORED_ERROR_HPP

#include <yorel/yomm2/policies/core.hpp>
#include <yorel/yomm2/detail/range.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

template<class Policy, typename DefaultHandlerProvider = void>
struct yOMM2_API_gcc vectored_error : virtual error_handler {
    static error_handler_type error;

    static void default_error_handler(const error_type& error_v) {
        using namespace detail;
        using namespace policy;

        if constexpr (Policy::template has_facet<error_output>) {
            if (auto error = std::get_if<resolution_error>(&error_v)) {
                const char* explanation[] = {
                    "no applicable definition", "ambiguous call"};
                Policy::error_stream
                    << explanation
                           [error->status - resolution_error::no_definition]
                    << " for " << error->method_name << "(";
                auto comma = "";

                for (auto ti :
                     range{error->types, error->types + error->arity}) {
                    Policy::error_stream << comma;
                    Policy::type_name(ti, Policy::error_stream);
                    comma = ", ";
                }

                Policy::error_stream << ")\n";
            } else if (
                auto error = std::get_if<unknown_class_error>(&error_v)) {
                Policy::error_stream << "unknown class ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<method_table_error>(&error_v)) {
                Policy::error_stream << "invalid method table for ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<hash_search_error>(&error_v)) {
                Policy::error_stream << "could not find hash factors after "
                                     << error->attempts << "s using "
                                     << error->buckets << " buckets\n";
            }
        }
    }
};

template<class Policy, typename DefaultHandlerProvider>
error_handler_type vectored_error<Policy, DefaultHandlerProvider>::error =
    std::conditional_t<
        std::is_same_v<DefaultHandlerProvider, void>, vectored_error<Policy>,
        DefaultHandlerProvider>::default_error_handler;

} // namespace policy
} // namespace yomm2
} // namespace yorel

#endif
