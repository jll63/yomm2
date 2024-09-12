
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_BASIC_INDIRECT_VPTR_HPP
#define YOREL_YOMM2_POLICY_BASIC_INDIRECT_VPTR_HPP

#include <yorel/yomm2/policies/core.hpp>

#include <vector>

namespace yorel {
namespace yomm2 {
namespace policies {

template<class Policy>
struct yOMM2_API_gcc basic_indirect_vptr : virtual indirect_vptr {
    static std::vector<std::uintptr_t const* const*> indirect_vptrs;
};

template<class Policy>
std::vector<std::uintptr_t const* const*>
    basic_indirect_vptr<Policy>::indirect_vptrs;

} // namespace policies
} // namespace yomm2
} // namespace yorel

#endif
