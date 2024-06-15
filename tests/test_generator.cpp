// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <regex>
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

    {
        std::ostringstream os;
        generator gen;
        gen.open(os);
        gen.forward_declarations();
        BOOST_TEST(os.str().empty());
    }

    {
        std::ostringstream os;
        generator gen;
        gen.open(os);
        gen.add<foo>();
        gen.forward_declarations();
        BOOST_TEST(os.str() == "class foo;\n");
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen;
        gen.open(os);
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
        os << "\n";
        generator gen;
        gen.open(os);
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
        generator gen;
        gen.open(os);
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
        generator gen;
        gen.open(os);
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
        generator gen;
        gen.open(os);
        gen.add<int>();
        gen.add<unsigned>();
        gen.forward_declarations();
        BOOST_TEST(os.str() == "");
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen;
        gen.open(os);
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
    generator gen;
    gen.open(os);
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
    generator gen;
    gen.open(os);
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
        generator gen;
        gen.open(os);
        gen.add(comp);

        gen.forward_declarations();
        gen.static_offsets(comp);
        std::string_view expected = R"(
class baz_key;
class foo;
template<> struct yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, int), test_policy_<1>>> {static constexpr size_t slots[] = {0}; };
template<> struct yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, yorel::yomm2::virtual_<foo&>), test_policy_<1>>> {static constexpr size_t slots[] = {1, 2}; static constexpr size_t strides[] = {1}; };
)";
        auto actual =
            std::regex_replace(os.str(), std::basic_regex("> +>"), ">>");
        // depending on platform, 'demangle' adds spaces between closing angle
        // brackets, or not.
        BOOST_TEST(actual == expected);
    }
}

BOOST_AUTO_TEST_CASE(test_generator_write_only_if_changed) {
    using namespace detail;

    namespace fs = std::filesystem;
    auto path = fs::temp_directory_path() / "yomm2_generator_test.hpp";
    auto temp = path;
    temp += generator::temp_ext;
    fs::remove(path);
    fs::remove(temp);

    generator gen;
    gen.add<foo>();
    gen.open(path);
    gen.forward_declarations();
    gen.close();

    BOOST_TEST(fs::exists(path));

    auto initial_time = fs::last_write_time(path);

    sleep(1);
    gen.open(path);
    gen.forward_declarations();
    gen.close();
    BOOST_TEST((fs::last_write_time(path) == initial_time));

    sleep(1);
    gen.add<ns1::bar>();
    gen.open(path);
    gen.forward_declarations();
    gen.close();
    BOOST_TEST((fs::last_write_time(path) > initial_time));

    fs::remove(path);
    fs::remove(temp);
}
