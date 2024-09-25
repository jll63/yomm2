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

#include <yorel/yomm2/policies/minimal_rtti.hpp>
#include <yorel/yomm2/policies/std_rtti.hpp>
#include <yorel/yomm2/policies/vptr_vector.hpp>
#include <yorel/yomm2/policies/vptr_map.hpp>
#include <yorel/yomm2/policies/basic_indirect_vptr.hpp>
#include <yorel/yomm2/policies/basic_error_output.hpp>
#include <yorel/yomm2/policies/basic_trace_output.hpp>
#include <yorel/yomm2/policies/fast_perfect_hash.hpp>
#include <yorel/yomm2/policies/vectored_error_handler.hpp>

#ifndef BOOST_NO_EXCEPTIONS
#include <yorel/yomm2/policies/throw_error.hpp>
#endif

namespace yorel {
namespace yomm2 {
namespace policies {

struct release
    : basic_policy<
          release, std_rtti, fast_perfect_hash<release>, vptr_vector<release>,
          vectored_error_handler<release>> {};

struct debug
    : basic_policy<
          debug, std_rtti, checked_perfect_hash<debug>, vptr_vector<debug>,
          basic_error_output<debug>, basic_trace_output<debug>,
          vectored_error_handler<debug>> {};

} // namespace policies


#ifdef NDEBUG
using default_policy = policies::release;
#else
using default_policy = policies::debug;
#endif

} // namespace yomm2
} // namespace yorel

#endif
