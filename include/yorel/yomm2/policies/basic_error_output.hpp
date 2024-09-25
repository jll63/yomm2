
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_BASIC_ERROR_OUTPUT_HPP
#define YOREL_YOMM2_POLICY_BASIC_ERROR_OUTPUT_HPP

#include <yorel/yomm2/policies/core.hpp>
#include <yorel/yomm2/detail/ostdstream.hpp>

namespace yorel {
namespace yomm2 {
namespace policies {

template<class Policy, typename Stream = detail::ostderr>
struct basic_error_output : virtual error_output {
    static Stream error_stream;
};

template<class Policy, typename Stream>
Stream basic_error_output<Policy, Stream>::error_stream;

} // namespace policies
} // namespace yomm2
} // namespace yorel

#endif
