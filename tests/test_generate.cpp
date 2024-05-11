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
struct key;

namespace ns1 {
struct foo {};
namespace ns11 {
struct foo {};
struct bar {};
} // namespace ns11
struct bar {};
} // namespace ns1

namespace ns2 {
namespace ns21 {
struct foo {};
} // namespace ns21
} // namespace ns2

void baz1_def(foo&) {}
void baz2_def(foo&, foo&) {}

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

    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(
            use_classes<
                ns1::foo, ns1::bar, ns1::ns11::foo, ns2::ns21::foo, policy>);
        compiler<policy> comp;
        comp.compile();
        std::ostringstream os;
        os << "\n";
        comp.generate_forward_declarations(os);
        std::string_view expected = R"(
namespace ns1 {
struct bar;
struct foo;
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

    {
        using policy = test_policy_<__COUNTER__>;
        YOMM2_STATIC(use_classes<foo, policy>);

        using baz1 = method<key, void(virtual_<foo&>), policy>;
        YOMM2_STATIC(baz1::add_function<baz1_def>);

        using baz2 = method<key, void(virtual_<foo&>, virtual_<foo&>), policy>;
        YOMM2_STATIC(baz2::add_function<baz2_def>);

        compiler<policy> comp;
        comp.compile();

        std::ostringstream os;
        os << "\n";
        comp.generate_header(os);
        std::string_view expected = R"(
struct foo;
struct key;
template<> struct static_offsets<yorel::yomm2::method<key, void (yorel::yomm2::virtual_<foo&>), test_policy_<9> >> {static constexpr size_t slots[] = {0}; };
template<> struct static_offsets<yorel::yomm2::method<key, void (yorel::yomm2::virtual_<foo&>, yorel::yomm2::virtual_<foo&>), test_policy_<9> >> {static constexpr size_t slots[] = {1, 2}; static constexpr size_t strides[] = {1}; };
} } }
)";
        BOOST_TEST(os.str() == expected);
    }
}
