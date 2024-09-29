// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <memory>

#include "test_generator_domain.hpp"

#include <yorel/yomm2/decode.hpp>

using namespace yorel::yomm2;

static_assert(detail::has_static_offsets<method_class(
                  kick, (virtual_<Animal&>, std::ostream&), void)>::value);
static_assert(detail::has_static_offsets<method_class(
                  meet, (virtual_<Animal&>, virtual_<Animal&>, std::ostream&),
                  void)>::value);
static_assert(
    detail::has_static_offsets<method_class(
        identify, (virtual_<Property&>, std::ostream&), void)>::value);

#define BOOST_TEST_MODULE test_generator
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

using namespace yorel::yomm2;

BOOST_AUTO_TEST_CASE(test_generator) {
#include "test_generator_tables.hpp"

    std::ostringstream os;

    auto animal = std::make_unique<Animal>();
    auto cat = std::make_unique<DomesticCat>("Alice");
    auto dog = std::make_unique<DomesticDog>("Bob");

    kick(*cat, os);
    BOOST_TEST(os.str() == "hiss");

    os.str("");
    kick(*dog, os);
    BOOST_TEST(os.str() == "bark");

    try {
        kick(*animal, os);
        BOOST_FAIL("should have thrown");
    } catch (const resolution_error& e) {
    }

    os.str("");
    meet(*dog, *cat, os);
    BOOST_TEST(os.str() == "chase");

    try {
        meet(*animal, *animal, os);
        BOOST_FAIL("should have thrown");
    } catch (const resolution_error& e) {
    }

    os.str("");
    identify(*cat, os);
    BOOST_TEST(os.str() == "Alice's cat");

    os.str("");
    identify(*dog, os);
    BOOST_TEST(os.str() == "Bob's dog");
}
