
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_THROW_ERROR_HPP
#define YOREL_YOMM2_POLICY_THROW_ERROR_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policies {

struct yOMM2_API_gcc throw_error : virtual error_handler {
    static void error(const error_type& error_v) {
        std::visit([](auto&& arg) { throw arg; }, error_v);
    }
};

} // namespace policies
} // namespace yomm2
} // namespace yorel

#endif
