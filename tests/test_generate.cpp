// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#include <yorel/yomm2.hpp>
#include "test_helpers.hpp"

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;

struct at_file_scope {};

namespace ns1 {

namespace ns11 {
struct in_ns_11 {};
} // namespace ns11

namespace ns12 {
struct in_ns_12 {};
} // namespace ns12
} // namespace ns1

BOOST_AUTO_TEST_CASE(test_generate_forward_declarations) {
    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(use_classes<at_file_scope, policy>);
        compiler<policy> comp;
        comp.compile();
        std::ostringstream os;
        comp.generate_forward_declarations(os);
        BOOST_TEST(os.str() == "struct at_file_scope;\n");
    }
}
