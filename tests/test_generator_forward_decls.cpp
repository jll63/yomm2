// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <regex>
#include <sstream>

#include <thread>
#include <chrono>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/generator.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE generator
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

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

struct generic_forward_declaration_data {
    std::string expected;
    const std::type_info** const first_type;
    const std::type_info** const last_type;
};

std::ostream&
operator<<(std::ostream& os, const generic_forward_declaration_data& data) {
    const char* sep = "";

    for (auto& type : detail::range{data.first_type, data.last_type}) {
        os << sep << boost::core::demangle(type->name());
        sep = ", ";
    }
    return os;
}

template<typename... Ts>
generic_forward_declaration_data fd(std::string expected) {
    static const std::type_info* types[] = {&typeid(Ts)...};
    return generic_forward_declaration_data(
        {expected, types, types + sizeof...(Ts)});
}

namespace bdata = boost::unit_test::data;

auto fd_dataset = bdata::make(
    fd<int>("\n"), fd<foo>("\nclass foo;\n"),
    fd<ns1::foo>(
        R"(
namespace ns1 {
class foo;
}
)"),
    fd<ns1::foo, ns2::foo>(
        R"(
namespace ns1 {
class foo;
}
namespace ns2 {
class foo;
}
)"),
    fd<ns1::foo, ns1_longer::foo>(
        R"(
namespace ns1 {
class foo;
}
namespace ns1_longer {
class foo;
}
)"),
    fd<method<foo, void(virtual_<baz<foo>&>, std::ostream)>>(
        R"(
class foo;
)"),
    fd<ns1::ns11::foo, ns1::ns11::bar>(
        R"(
namespace ns1 {
namespace ns11 {
class bar;
class foo;
}
}
)"),
    fd<ns1::ns11::foo, ns2::ns21::foo>(
        R"(
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
)"));

BOOST_DATA_TEST_CASE(test_generator_write_forward_declarations, fd_dataset) {
    std::ostringstream os;
    generator gen;

    for (auto& type : detail::range{sample.first_type, sample.last_type}) {
        gen.add_forward_declaration(*type);
    }

    os << "\n";
    gen.write_forward_declarations(os);
    BOOST_TEST(os.str() == sample.expected);
}

BOOST_AUTO_TEST_CASE(test_generator_write_forward_declarations_) {
    using namespace detail;

    {
        std::ostringstream os;
        generator gen;
        gen.write_forward_declarations(os);
        BOOST_TEST(os.str().empty());
    }

    {
        std::ostringstream os;
        os << "\n";
        generator gen;
        gen.add_forward_declaration<ns1::foo>();
        gen.add_forward_declaration<ns1::ns11::bar>();
        gen.add_forward_declaration<ns1::ns11::foo>();

        gen.write_forward_declarations(os);
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
        YOMM2_STATIC(baz1::override_fn<baz1_def>);

        using baz2 =
            method<baz_key, void(virtual_<foo&>, virtual_<foo&>), policy>;
        YOMM2_STATIC(baz2::override_fn<baz2_def>);

        update<policy>();

        {
            std::ostringstream os;
            os << "\n";
            generator gen;
            gen.write_static_offsets<baz1>(os);
            std::string_view expected = R"(
template<> struct yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, int), test_policy_<1>>> {static constexpr std::size_t slots[] = {0}; };
)";
            auto actual =
                std::regex_replace(os.str(), std::basic_regex("> +>"), ">>");
            // On some compilers, 'demangle' adds spaces between closing angle
            // brackets.
            BOOST_TEST(actual == expected);
        }

        {
            std::ostringstream os;
            os << "\n";
            generator gen;
            gen.write_forward_declarations(os);
            gen.write_static_offsets<policy>(os);
            std::string_view expected = R"(
template<> struct yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, int), test_policy_<1>>> {static constexpr std::size_t slots[] = {0}; };
template<> struct yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, yorel::yomm2::virtual_<foo&>), test_policy_<1>>> {static constexpr std::size_t slots[] = {1, 2}; static constexpr std::size_t strides[] = {1}; };
)";
            auto actual =
                std::regex_replace(os.str(), std::basic_regex("> +>"), ">>");
            // On some compilers, 'demangle' adds spaces between closing angle
            // brackets.
            BOOST_TEST(actual == expected);
        }
    }
}
