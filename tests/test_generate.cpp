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

struct foo {};

namespace ns1 {

namespace ns11 {
struct foo {};
struct bar {};
} // namespace ns11
} // namespace ns1

namespace ns2 {
namespace ns21 {
struct foo {};
} // namespace ns21
} // namespace ns2

BOOST_AUTO_TEST_CASE(test_generate_forward_declarations) {
    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(use_classes<foo, policy>);
        compiler<policy> comp;
        comp.compile();
        std::ostringstream os;
        comp.generate_forward_declarations(os);
        BOOST_TEST(os.str() == "struct foo;\n");
    }

    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(use_classes<ns1::ns11::foo, ns1::ns11::bar, policy>);
        compiler<policy> comp;
        comp.compile();
        std::ostringstream os;
        os << "\n";
        comp.generate_forward_declarations(os);
        std::string_view expected = R"(
namespace ns1 {
namespace ns11 {
struct bar;
struct foo;
}
}
)";
        BOOST_TEST(os.str() == expected);
    }

    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(use_classes<ns1::ns11::foo, ns2::ns21::foo, policy>);
        compiler<policy> comp;
        comp.compile();
        std::ostringstream os;
        os << "\n";
        comp.generate_forward_declarations(os);
        std::string_view expected = R"(
namespace ns1 {
namespace ns11 {
struct foo;
}
}
namespace ns2 {
namespace ns21 {
struct foo;
}
}
)";
        BOOST_TEST(os.str() == expected);
    }
}
