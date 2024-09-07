
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_BASIC_TRACE_OUTPUT_HPP
#define YOREL_YOMM2_POLICY_BASIC_TRACE_OUTPUT_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

template<class Policy, typename Stream = detail::ostderr>
struct yOMM2_API_gcc basic_trace_output : virtual trace_output {
    static Stream trace_stream;
    static bool trace_enabled;
};

template<class Policy, typename Stream>
Stream basic_trace_output<Policy, Stream>::trace_stream;

template<class Policy, typename Stream>
bool basic_trace_output<Policy, Stream>::trace_enabled([]() {
    auto env = getenv("YOMM2_TRACE");
    return env && *env++ == '1' && *env++ == 0;
}());

} // namespace policy
} // namespace yomm2
} // namespace yorel

#endif
