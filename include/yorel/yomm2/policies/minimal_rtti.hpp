
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_MINIMAL_RTTI_HPP
#define YOREL_YOMM2_POLICY_MINIMAL_RTTI_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

struct minimal_rtti : virtual rtti {
    template<typename T>
    static auto static_type() -> type_id {
        static char id;
        return reinterpret_cast<type_id>(&id);
    }
};

} // namespace policy
} // namespace yomm2
} // namespace yorel

#endif
