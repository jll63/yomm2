// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "yorel/yomm2/detail/chain.hpp"

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2::detail;

struct value : chain<value>::link {
    int id;
};

// std::ostream&
// operator<<(std::ostream& os, static_chain<value>::iterator iter) {
//     return os; //  << &*iter;
// }

using test_iter = chain<value>::iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(test_iter);

BOOST_AUTO_TEST_CASE(test_chain) {
    chain<value> l;
    value a, b, c, d;

    BOOST_TEST_REQUIRE(l.begin() == l.end());

    l.push_front(a);
    BOOST_TEST_REQUIRE(&*l.begin() == &a);
    BOOST_TEST_REQUIRE(&l.begin()->id == &a.id);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 1);

    static_chain<value>::iterator iter;

    l.push_front(b);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);

    // b a
    // test iterator post-increment
    iter = l.begin();
    BOOST_TEST_REQUIRE(&iter->id == &b.id);
    BOOST_TEST_REQUIRE(&iter++->id == &b.id);
    BOOST_TEST_REQUIRE(&iter++->id == &a.id);
    BOOST_TEST_REQUIRE(iter == l.end());

    // b a
    // test iterator pre-increment
    iter = l.begin();
    ++iter;
    BOOST_TEST_REQUIRE(&iter->id == &a.id);

    l.push_front(c);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 3);

    // c b a
    // test remove from front, the commonest case
    l.remove(c);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);
    iter = l.begin();
    BOOST_TEST_REQUIRE(&iter++->id == &b.id);
    BOOST_TEST_REQUIRE(&iter++->id == &a.id);

    l.push_front(c);
    l.push_front(d);

    // d c b a
    // test remove from front
    l.remove(d);
    iter = l.begin();
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 3);
    BOOST_TEST_REQUIRE(&iter++->id == &c.id);
    BOOST_TEST_REQUIRE(&iter++->id == &b.id);
    BOOST_TEST_REQUIRE(&iter++->id == &a.id);

    // c b a
    // test remove from middle
    l.remove(b);
    iter = l.begin();
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 2);
    BOOST_TEST_REQUIRE(&iter++->id == &c.id);
    BOOST_TEST_REQUIRE(&iter++->id == &a.id);

    // c a
    // test remove from end
    l.remove(a);
    iter = l.begin();
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 1);
    BOOST_TEST_REQUIRE(&iter++->id == &c.id);

    // a
    // test remove from end, one item left
    l.remove(c);
    BOOST_TEST_REQUIRE(std::distance(l.begin(), l.end()) == 0);
}

struct static_value : static_chain<static_value>::static_link {
    explicit static_value(static_chain<static_value>& reg) {
        reg.push_front(*this);
    }
};

// Check that static links and chain work regardless of static initialization
// order.

using static_test_iter = static_chain<static_value>::iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(static_test_iter);

namespace link_link_chain {
extern static_chain<static_value> reg;
static_value a(reg);
static_value b(reg);
static_chain<static_value> reg;

BOOST_AUTO_TEST_CASE(test_static_link_link_chain) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace link_link_chain

namespace link_chain_link {
extern static_chain<static_value> reg;
static_value a(reg);
static_chain<static_value> reg;
static_value b(reg);

BOOST_AUTO_TEST_CASE(test_static_link_chain_link) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace link_chain_link

namespace chain_link_link {
static_chain<static_value> reg;
static_value a(reg);
static_value b(reg);

BOOST_AUTO_TEST_CASE(test_static_chain_link_link) {
    auto iter = reg.begin(), last = reg.end();
    BOOST_TEST_REQUIRE(iter != last);
    BOOST_TEST_REQUIRE(&*iter == &b);
    BOOST_TEST_REQUIRE(++iter != last);
    BOOST_TEST_REQUIRE(&*iter == &a);
    BOOST_TEST_REQUIRE(++iter == last);
}
} // namespace chainlink_link