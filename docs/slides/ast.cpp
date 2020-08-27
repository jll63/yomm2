// clang++-5.0 -std=c++17 -I ~/dev/yomm2/include -o ast ast.cpp ~/dev/yomm2/src/yomm2.cpp && ./ast

#include <iostream>
#include <utility>
#include <unordered_map>
#include <typeindex>

using namespace std;

class Number;
class Plus;
class Times;

struct Node {
    virtual double value() = 0;
    virtual string toRPN() = 0;

    struct Visitor {
        virtual void accept(Number& expr) = 0;
        virtual void accept(Plus& expr) = 0;
        virtual void accept(Times& expr) = 0;
    };

    virtual void visit(Visitor& viz) = 0;
};

struct Number : Node {
  Number(double value) : val(value ) { }

  double value() override {
    return val;
  }

  string toRPN() override {
    return to_string(val);
  }

  void visit(Visitor& viz) override {
    viz.accept(*this);
  }

  double val;
};

struct Plus : Node {
    Plus(Node& left, Node& right)
    : left(left), right(right) { }

  double value() override {
    return left.value() + right.value();
  }

  string toRPN() override {
    return left.toRPN() + " " + right.toRPN() + " +";
  }

  void visit(Visitor& viz) override {
    viz.accept(*this);
  }

  Node &left, &right;
};

struct Times : Node {
    Times(Node& left, Node& right)
    : left(left), right(right) { }

    double value() override {
        return left.value() * right.value();
    }

    string toRPN() override {
        return left.toRPN() + " " + right.toRPN() + " &";
    }

    void visit(Visitor& viz) override {
        viz.accept(*this);
    }

    Node &left, &right;
};

namespace typeswitch {

string toRPN(Node& node) {
    if (auto expr = dynamic_cast<Number*>(&node)) {
        return to_string(expr->value());
    } else if (auto expr = dynamic_cast<Plus*>(&node)) {
        return toRPN(expr->left) + " " + toRPN(expr->right) + " +";
    } else if (auto expr = dynamic_cast<Times*>(&node)) {
        return toRPN(expr->left) + " " + toRPN(expr->right) + " *";
    }
    throw runtime_error("unknown node type");
}
}

namespace visitor {

struct RPNVisitor : Node::Visitor {
  void accept(Number& expr) {
    result = to_string(expr.val);
  }
  void accept(Plus& expr) {
    expr.left.visit(*this);
    string l = result;
    expr.right.visit(*this);
    result = l + " " + result + " +";
  }
  void accept(Times& expr) {
    expr.left.visit(*this);
    string l = result;
    expr.right.visit(*this);
    result = l + " " + result + " *";
  }
  string result;
};

string toRPN(Node& node) {
  RPNVisitor viz;
  node.visit(viz);
  return viz.result;
}

}

namespace funtable {

using RPNFormatter = string (*)(Node&);
unordered_map<type_index, RPNFormatter> RPNformatters;

string toRPN(Node& node) {
  return RPNformatters[typeid(node)](node);
}

namespace { struct Init {
    Init() {
        RPNformatters[typeid(Number)] = [](Node& node) {
          return to_string(static_cast<Number&>(node).val); };
        RPNformatters[typeid(Plus)] = [](Node& node) {
          auto expr = static_cast<Plus&>(node);
          return toRPN(expr.left) + " " + toRPN(expr.right) + " +"; };
        RPNformatters[typeid(Times)] = [](Node& node) {
          auto expr = static_cast<Times&>(node);
          return toRPN(expr.left) + " " + toRPN(expr.right) + " *"; };
    }
};
Init init;
} }

#include <yorel/yomm2/cute.hpp>

using yorel::yomm2::virtual_;

namespace openmethods {

register_class(Node);
register_class(Plus, Node);
register_class(Times, Node);
register_class(Number, Node);

declare_method(string, toRPN, (virtual_<const Node&>));

define_method(string, toRPN, (const Number& expr)) {
  return std::to_string(expr.val);
}

define_method(string, toRPN, (const Plus& expr)) {
  return toRPN(expr.left) + " " + toRPN(expr.right) + " +";
}

define_method(string, toRPN, (const Times& expr)) {
  return toRPN(expr.left) + " " + toRPN(expr.right) + " *";
}

declare_method(int, value, (virtual_<const Node&>));

define_method(int, value, (const Number&& expr)) {
  return expr.val;
}

define_method(int, value, (const Plus& expr)) {
  return value(expr.left) + value(expr.right);
}

define_method(int, value, (const Times& expr)) {
  return value(expr.left) * value(expr.right);
}
}

int main() {
    Node* expr =
        new Times(*new Number(2),
                  *new Plus(*new Number(3), *new Number(4)));

    cout << expr->value() << "\n";
    cout << typeswitch::toRPN(*expr) << " = " << expr->value() << "\n";
    cout << visitor::toRPN(*expr) << " = " << expr->value() << "\n";
    cout << funtable::toRPN(*expr) << " = " << expr->value() << "\n";

    yorel::yomm2::update_methods();
    cout << openmethods::toRPN(*expr) << " = " << expr->value() << "\n";


    return 0;
}
