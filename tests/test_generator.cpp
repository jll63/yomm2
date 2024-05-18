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

BOOST_AUTO_TEST_CASE(test_generator_extract_type) {
    using namespace detail;

    {
        std::string input("<foo>");
        BOOST_TEST((extract_type(input.begin()) == input.end() - 1));
    }

    {
        std::string input("<(foo<int (*x)[10]>, foo<bar<int>>)>");
        BOOST_TEST((extract_type(input.begin()) == input.end() - 1));
    }
}

BOOST_AUTO_TEST_CASE(test_generator_extract_simple_types) {
    using namespace detail;

    {
        std::string input("foo");
        std::vector<std::string_view> actual, expected = {"foo"};
        auto out = extract_simple_types(
            input.begin(), input.end(), std::back_insert_iterator(actual));
        BOOST_TEST(actual == expected);
    }
}

// struct foo {};
// struct baz_key;
// struct file_scope_policy : test_policy_<__COUNTER__> {};

// namespace ns1 {

// struct policy : test_policy_<__COUNTER__> {};
// struct foo {};

// namespace ns11 {
// struct policy : test_policy_<__COUNTER__> {};
// struct foo {};
// struct bar {};
// } // namespace ns11

// struct bar {};
// } // namespace ns1

// namespace ns2 {
// struct policy : test_policy_<__COUNTER__> {};

// namespace ns21 {
// struct foo {};

// } // namespace ns21
// } // namespace ns2

// void baz1_def(foo&, std::ostream&) {
// }
// void baz2_def(foo&, foo&) {
// }

// BOOST_AUTO_TEST_CASE(test_generate_classes_file_scope_only) {
//     YOMM2_STATIC(use_classes<foo, file_scope_policy>);
//     compiler<file_scope_policy> comp;
//     comp.compile();
//     std::ostringstream os;
//     generator gen(comp, os);
//     gen.write_forward_declarations();
//     BOOST_TEST(os.str() == "struct foo;\n");
// }

// BOOST_AUTO_TEST_CASE(test_generate_classes_ns1_ns11) {
//     using policy = ns1::policy;
//     YOMM2_STATIC(
//         use_classes<ns1::ns11::foo, ns1::ns11::bar, ns1::policy>);
//     compiler<policy> comp;
//     comp.compile();
//     std::ostringstream os;
//     os << "\n";
//     generator gen(comp, os);
//     gen.write_forward_declarations();
//     std::string_view expected = R"(
// namespace ns1 {
// namespace ns11 {
// struct bar;
// struct foo;
// }
// }
// )";
//     BOOST_TEST(os.str() == expected);
// }

// BOOST_AUTO_TEST_CASE(test_generate_classes_ns1_ns2) {
//     using policy = ns2::policy;
//     YOMM2_STATIC(use_classes<ns1::ns11::foo, ns2::ns21::foo, policy>);
//     compiler<policy> comp;
//     comp.compile();
//     std::ostringstream os;
//     os << "\n";
//     generator gen(comp, os);
//     gen.write_forward_declarations();
//     std::string_view expected = R"(
// namespace ns1 {
// namespace ns11 {
// struct foo;
// }
// }
// namespace ns2 {
// namespace ns21 {
// struct foo;
// }
// }
// )";
//     BOOST_TEST(os.str() == expected);
// }

// BOOST_AUTO_TEST_CASE(test_generate_offsets) {
//     {
//         using policy = test_policy_<__COUNTER__>;
//         YOMM2_STATIC(use_classes<foo, policy>);

//         using baz1 =
//             method<baz_key, void(virtual_<foo&>, std::ostream&), policy>;
//         YOMM2_STATIC(baz1::add_function<baz1_def>);

//         using baz2 =
//             method<baz_key, void(virtual_<foo&>, virtual_<foo&>), policy>;
//         YOMM2_STATIC(baz2::add_function<baz2_def>);

//         compiler<policy> comp;
//         comp.compile();

//         std::ostringstream os;
//         os << "\n";
//         generator gen(comp, os);
//         gen.write_static_offsets();
//         std::string_view expected = R"(
// template<> struct ::yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, std::ostream&), test_policy_<9> >> {static constexpr size_t slots[] = {0}; };
// template<> struct ::yorel::yomm2::detail::static_offsets<yorel::yomm2::method<baz_key, void (yorel::yomm2::virtual_<foo&>, yorel::yomm2::virtual_<foo&>), test_policy_<9> >> {static constexpr size_t slots[] = {1, 2}; static constexpr size_t strides[] = {1}; };
// } } }
// )";

//         BOOST_TEST(os.str() == expected);

//         // auto cwd = std::filesystem::current_path();
//         // auto header_path = cwd /= "test_generate_header.hpp";

//         // std::filesystem::remove(header_path);

//         // comp.generate_header(header_path.string());
//     }
// }
