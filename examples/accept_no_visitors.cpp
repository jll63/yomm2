// Copyright (c) 2018-2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#include <yorel/yomm2/cute.hpp>

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::cout;

using yorel::yomm2::virtual_;

struct Node {
    virtual ~Node() {}
};

struct Plus : Node {
    Plus(shared_ptr<const Node> left, shared_ptr<const Node> right)
    : left(left), right(right) {}

    shared_ptr<const Node> left, right;
};

struct Times : Node {
    Times(shared_ptr<const Node> left, shared_ptr<const Node> right)
    : left(left), right(right) {}

    shared_ptr<const Node> left, right;
};

struct Integer : Node {
    explicit Integer(int value) : value(value) {}
    int value;
};

// =============================================================================
// add behavior to existing classes, without changing them

register_class(Node);
register_class(Plus, Node);
register_class(Times, Node);
register_class(Integer, Node);

// -----------------------------------------------------------------------------
// evaluate

declare_method(int, value, (virtual_<const Node&>));

define_method(int, value, (const Plus& expr)) {
  return value(*expr.left) + value(*expr.right);
}

define_method(int, value, (const Times& expr)) {
  return value(*expr.left) * value(*expr.right);
}

define_method(int, value, (const Integer& expr)) {
  return expr.value;
}

// -----------------------------------------------------------------------------
// render as Forth

declare_method(string, as_forth, (virtual_<const Node&>));

define_method(string, as_forth, (const Plus& expr)) {
    return as_forth(*expr.left) + " " + as_forth(*expr.right) + " +";
}

define_method(string, as_forth, (const Times& expr)) {
    return as_forth(*expr.left) + " " + as_forth(*expr.right) + " *";
}

define_method(string, as_forth, (const Integer& expr)) {
    return std::to_string(expr.value);
}

// -----------------------------------------------------------------------------
// render as Lisp

declare_method(string, as_lisp, (virtual_<const Node&>));

define_method(string, as_lisp, (const Plus& expr)) {
    return "(plus " + as_lisp(*expr.left) + " " + as_lisp(*expr.right) + ")";
}

define_method(string, as_lisp, (const Times& expr)) {
    return "(times " + as_lisp(*expr.left) + " " + as_lisp(*expr.right) + ")";
}

define_method(string, as_lisp, (const Integer& expr)) {
    return std::to_string(expr.value);
}

// -----------------------------------------------------------------------------

int main()
{
    yorel::yomm2::update_methods();

    shared_ptr<Node> expr =
        make_shared<Times>(
            make_shared<Integer>(2),
            make_shared<Plus>(
                make_shared<Integer>(3),
                make_shared<Integer>(4)));

    cout << as_forth(*expr)
         << " = " << as_lisp(*expr)
         << " = " << value(*expr) << "\n";
    // output:
    // 2 3 4 + * = (times 2 (plus 3 4)) = 14

    return 0;
}
