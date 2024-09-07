
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_BASIC_ERROR_OUTPUT_HPP
#define YOREL_YOMM2_POLICY_BASIC_ERROR_OUTPUT_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

template<class Policy, typename Stream = detail::ostderr>
struct yOMM2_API_gcc basic_error_output : virtual error_output {
    static Stream error_stream;
};

template<class Policy, typename Stream>
Stream basic_error_output<Policy, Stream>::error_stream;

} // namespace policy
} // namespace yomm2
} // namespace yorel

#endif
