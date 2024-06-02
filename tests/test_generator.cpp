// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/generator.hpp>

#include "test_helpers.hpp"

#define BOOST_TEST_MODULE generator
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;

struct foo {};

template<typename>
struct baz {};

namespace ns1 {

struct foo {};

namespace ns11 {
struct foo {};
struct bar {};
} // namespace ns11

struct bar {};
} // namespace ns1

namespace ns2 {
struct foo {};

namespace ns21 {
struct foo {};

} // namespace ns21
} // namespace ns2

namespace ns1_longer {
struct foo {};
} // namespace ns1_longer

BOOST_AUTO_TEST_CASE(test_generator_forward_declarations) {
    using namespace detail;

    compiler<default_policy> comp;
    comp.compile();

    {
        std::ostringstream os;
        generator gen(comp, os);
        gen.forward_declarations();
        BOOST_TEST(os.str().empty());
    }

    {
        std::ostringstream os;
        generator gen(comp, os);
        gen.add<foo>();
        gen.forward_declarations();
        BOOST_TEST(os.str() == "class foo;\n");
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);
        gen.add<ns1::foo>();
        gen.forward_declarations();
        std::string_view expected = R"(
namespace ns1 {
class foo;
}
)";
        BOOST_TEST(os.str() == expected);
    }

    {
        std::ostringstream os;
        generator gen(comp, os);
        gen.add<ns1::foo>();
        gen.forward_declarations([](auto name) { return false; });
        BOOST_TEST(os.str().empty());
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);
        gen.add<ns1::foo, ns1::ns11::bar, ns1::ns11::foo>();
        gen.forward_declarations();
        std::string_view expected = R"(
namespace ns1 {
class foo;
namespace ns11 {
class bar;
class foo;
}
}
)";
        BOOST_TEST(os.str() == expected);
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);
        gen.add<ns1::foo, ns2::foo>();
        gen.forward_declarations();
        std::string_view expected = R"(
namespace ns1 {
class foo;
}
namespace ns2 {
class foo;
}
)";
        BOOST_TEST(os.str() == expected);
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);
        gen.add<ns1::foo, ns1_longer::foo>();
        gen.forward_declarations();
        std::string_view expected = R"(
namespace ns1 {
class foo;
}
namespace ns1_longer {
class foo;
}
)";
        BOOST_TEST(os.str() == expected);
    }

    {
        std::ostringstream os;
        generator gen(comp, os);
        gen.add<int>();
        gen.add<unsigned>();
        gen.forward_declarations();
        BOOST_TEST(os.str() == "");
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);
        gen.add<method<foo, void(virtual_<baz<foo>&>, std::ostream)>>();
        gen.forward_declarations();
        BOOST_TEST(os.str() == "\nclass foo;\n");
    }
}

BOOST_AUTO_TEST_CASE(test_generate_classes_ns1_ns11) {
    compiler<default_policy> comp;
    comp.compile();
    std::ostringstream os;
    os << "\n";
    generator gen(comp, os);
    gen.add<ns1::ns11::foo, ns1::ns11::bar>();
    gen.forward_declarations();
    std::string_view expected = R"(
namespace ns1 {
namespace ns11 {
class bar;
class foo;
}
}
)";
    BOOST_TEST(os.str() == expected);
}

BOOST_AUTO_TEST_CASE(test_generate_classes_ns1_ns2) {
    compiler<default_policy> comp;
    comp.compile();
    comp.compile();
    std::ostringstream os;
    os << "\n";
    generator gen(comp, os);
    gen.add<ns1::ns11::foo, ns2::ns21::foo>();
    gen.forward_declarations();
    std::string_view expected = R"(
namespace ns1 {
namespace ns11 {
class foo;
}
}
namespace ns2 {
namespace ns21 {
class foo;
}
}
)";
    BOOST_TEST(os.str() == expected);
}

struct baz_key;

void baz1_def(foo&, int) {
}
void baz2_def(foo&, foo&) {
}

BOOST_AUTO_TEST_CASE(test_generate_offsets) {
    {
        using policy = test_policy_<1>;
        YOMM2_STATIC(use_classes<foo, policy>);

        using baz1 = method<baz_key, void(virtual_<foo&>, int), policy>;
        YOMM2_STATIC(baz1::add_function<baz1_def>);

        using baz2 =
            method<baz_key, void(virtual_<foo&>, virtual_<foo&>), policy>;
        YOMM2_STATIC(baz2::add_function<baz2_def>);

        compiler<policy> comp;
        comp.compile();

        std::ostringstream os;
        os << "\n";
        generator gen(comp, os);

        gen.forward_declarations();
        gen.static_offsets();
        std::string_view expected = R"(
class baz_key;
class foo;
template<> struct ::yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, int), test_policy_<1> >> {static constexpr size_t slots[] = {0}; };
template<> struct ::yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, yorel::yomm2::virtual_<foo&>), test_policy_<1> >> {static constexpr size_t slots[] = {1, 2}; static constexpr size_t strides[] = {1}; };
} } }
)";

        BOOST_TEST(os.str() == expected);

        // auto cwd = std::filesystem::current_path();
        // auto header_path = cwd /= "test_generate_header.hpp";

        // std::filesystem::remove(header_path);

        // comp.generate_header(header_path.string());
    }
}
