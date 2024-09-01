// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "yorel/yomm2/detail/list.hpp"

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using yorel::yomm2::detail::static_list;

struct value : static_list<value>::static_link {};

using test_iter = static_list<value>::iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(test_iter);

BOOST_AUTO_TEST_CASE(test_list) {
    static static_list<value> l;
    static value a, b, c, d;

    BOOST_TEST_REQUIRE(l.begin() == l.end());
    BOOST_TEST_REQUIRE(l.empty());

    l.push_back(a);
    // a
    BOOST_TEST_REQUIRE(&*l.begin() == &a);
    BOOST_TEST_REQUIRE(!l.empty());
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 1);

    static_list<value>::iterator iter;

    l.push_back(b);
    // a b
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);

    // test iterator post-increment
    iter = l.begin();
    BOOST_TEST_REQUIRE(iter != l.end());
    BOOST_TEST(&*iter++ == &a);
    BOOST_TEST_REQUIRE(iter != l.end());
    BOOST_TEST(&*iter++ == &b);
    BOOST_TEST(iter == l.end());

    // test iterator pre-increment
    iter = l.begin();
    ++iter;
    BOOST_TEST_REQUIRE(&*iter == &b);

    l.push_back(c);
    // a b c
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 3);

    // test remove from back, the most common case
    l.remove(c);
    // a b

    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);
    iter = l.begin();
    BOOST_TEST_REQUIRE(&*iter++ == &a);
    BOOST_TEST_REQUIRE(&*iter++ == &b);

    l.push_back(c);
    // a b c
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 3);

    l.push_back(d);
    // a b c d
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 4);

    // test remove from front
    l.remove(a);
    // b c d

    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 3);
    iter = l.begin();
    BOOST_TEST_REQUIRE(&*iter++ == &b);
    BOOST_TEST_REQUIRE(&*iter++ == &c);
    BOOST_TEST_REQUIRE(&*iter++ == &d);

    // test remove from middle
    l.remove(b);
    // c d

    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);
    iter = l.begin();
    BOOST_TEST_REQUIRE(&*iter++ == &c);
    BOOST_TEST_REQUIRE(&*iter++ == &d);

    // test remove from end
    l.remove(c);
    // d

    iter = l.begin();
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 1);
    BOOST_TEST_REQUIRE(&*iter++ == &d);

    // test remove from end, one item left
    l.remove(d);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 0);
}

struct static_value : static_list<static_value>::static_link {
    explicit static_value(static_list<static_value>& reg) {
        reg.push_back(*this);
    }
};

// Check that static links and list work regardless of static initialization
// order.

using static_test_iter = static_list<static_value>::iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(static_test_iter);

namespace link_link_list {
extern static_list<static_value> reg;
static_value a(reg);
static_value b(reg);
static_list<static_value> reg;

BOOST_AUTO_TEST_CASE(test_static_link_link_list) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace link_link_list

namespace link_list_link {
extern static_list<static_value> reg;
static_value a(reg);
static_list<static_value> reg;
static_value b(reg);

BOOST_AUTO_TEST_CASE(test_static_link_list_link) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace link_list_link

namespace list_link_link {
static_list<static_value> reg;
static_value a(reg);
static_value b(reg);

BOOST_AUTO_TEST_CASE(test_static_list_link_link) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace list_link_link
