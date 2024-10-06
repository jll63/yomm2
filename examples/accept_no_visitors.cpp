// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

using std::cout;
using std::make_shared;
using std::shared_ptr;
using std::string;

struct Node {
    virtual ~Node() {
    }
};

struct Plus : Node {
    Plus(shared_ptr<const Node> left, shared_ptr<const Node> right)
        : left(left), right(right) {
    }

    shared_ptr<const Node> left, right;
};

struct Times : Node {
    Times(shared_ptr<const Node> left, shared_ptr<const Node> right)
        : left(left), right(right) {
    }

    shared_ptr<const Node> left, right;
};

struct Integer : Node {
    explicit Integer(int value) : value(value) {
    }
    int value;
};

// =============================================================================
// add behavior to existing classes, without changing them

BOOST_OPENMETHOD_CLASSES(Node, Plus, Times, Integer);

// -----------------------------------------------------------------------------
// evaluate

BOOST_OPENMETHOD(value, (virtual_<const Node&>), int);

BOOST_OPENMETHOD_OVERRIDE(value, (const Plus& expr), int) {
    return value(*expr.left) + value(*expr.right);
}

BOOST_OPENMETHOD_OVERRIDE(value, (const Times& expr), int) {
    return value(*expr.left) * value(*expr.right);
}

BOOST_OPENMETHOD_OVERRIDE(value, (const Integer& expr), int) {
    return expr.value;
}

// -----------------------------------------------------------------------------
// render as Forth

BOOST_OPENMETHOD(as_forth, (virtual_<const Node&>), string);

BOOST_OPENMETHOD_OVERRIDE(as_forth, (const Plus& expr), string) {
    return as_forth(*expr.left) + " " + as_forth(*expr.right) + " +";
}

BOOST_OPENMETHOD_OVERRIDE(as_forth, (const Times& expr), string) {
    return as_forth(*expr.left) + " " + as_forth(*expr.right) + " *";
}

BOOST_OPENMETHOD_OVERRIDE(as_forth, (const Integer& expr), string) {
    return std::to_string(expr.value);
}

// -----------------------------------------------------------------------------
// render as Lisp

BOOST_OPENMETHOD(as_lisp, (virtual_<const Node&>), string);

BOOST_OPENMETHOD_OVERRIDE(as_lisp, (const Plus& expr), string) {
    return "(plus " + as_lisp(*expr.left) + " " + as_lisp(*expr.right) + ")";
}

BOOST_OPENMETHOD_OVERRIDE(as_lisp, (const Times& expr), string) {
    return "(times " + as_lisp(*expr.left) + " " + as_lisp(*expr.right) + ")";
}

BOOST_OPENMETHOD_OVERRIDE(as_lisp, (const Integer& expr), string) {
    return std::to_string(expr.value);
}

// -----------------------------------------------------------------------------

int main() {
    boost::openmethod::initialize();

    shared_ptr<Node> expr = make_shared<Times>(
        make_shared<Integer>(2),
        make_shared<Plus>(make_shared<Integer>(3), make_shared<Integer>(4)));

    cout << as_forth(*expr) << " = " << as_lisp(*expr) << " = " << value(*expr)
         << "\n";
    // error_output:
    // 2 3 4 + * = (times 2 (plus 3 4)) = 14

    return 0;
}
