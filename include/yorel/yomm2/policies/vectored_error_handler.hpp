
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_VECTORED_ERROR_HPP
#define YOREL_YOMM2_POLICY_VECTORED_ERROR_HPP

#include <yorel/yomm2/policies/core.hpp>

#include <functional>
#include <variant>

namespace yorel {
namespace yomm2 {
namespace policies {

template<class Policy>
struct yOMM2_API_gcc vectored_error_handler : virtual error_handler {

    using error_variant = std::variant<
        error, resolution_error, unknown_class_error, hash_search_error,
        method_table_error, static_slot_error, static_stride_error>;

    using error_handler_type = std::function<void(const error_variant& error)>;
    static error_handler_type error_handler;

    template<class Error>
    static auto error(const Error& error) {
        error_handler(error_variant(error));
    }

    static auto set_error_handler(error_handler_type handler) {
        auto prev = error_handler;
        error_handler = handler;

        return error_handler;
    }

    static auto default_error_handler(const error_variant& error_v) {
        using namespace detail;
        using namespace policies;

        if constexpr (Policy::template has_facet<error_output>) {
            if (auto error = std::get_if<resolution_error>(&error_v)) {
                const char* explanation[] = {
                    "no applicable definition", "ambiguous call"};
                Policy::error_stream
                    << explanation
                           [error->status - resolution_error::no_definition]
                    << " for ";
                Policy::type_name(error->method, Policy::error_stream);
                Policy::error_stream << "(";
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

template<class Policy>
typename vectored_error_handler<Policy>::error_handler_type
    vectored_error_handler<Policy>::error_handler =
        vectored_error_handler<Policy>::default_error_handler;

} // namespace policies
} // namespace yomm2
} // namespace yorel

#endif
