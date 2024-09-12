// clang-format off

#include <functional>
#include <iostream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

using namespace std;

class Number;
class Plus;
class Times;

struct Node {
  virtual ~Node() {}
  virtual int value() const = 0;
  virtual string to_rpn() const = 0;

  struct Visitor {
    virtual void accept(const Number& expr) = 0;
    virtual void accept(const Plus& expr) = 0;
    virtual void accept(const Times& expr) = 0;
  };

  virtual void visit(Visitor& viz) const = 0;
};

struct Number : Node {
  explicit Number(int value) : val(value) { }
  int value() const override { return val; }
  string to_rpn() const override { return to_string(val); }
  void visit(Visitor& viz) const override { viz.accept(*this); }
  int val;
};

struct Plus : Node {
  Plus(const Node& left, const Node& right) : left(left), right(right) { }
  int value() const override { return left.value() + right.value(); }
  string to_rpn() const override { return left.to_rpn() + " " + right.to_rpn() + " +"; }
  void visit(Visitor& viz) const override { viz.accept(*this); }
  const Node& left; const Node& right;
};

struct Times : Node {
  Times(const Node& left, const Node& right) : left(left), right(right) { }
  int value() const override { return left.value() * right.value(); }
  string to_rpn() const override { return left.to_rpn() + " " + right.to_rpn() + " &"; }
  void visit(Visitor& viz) const override { viz.accept(*this); }
  const Node& left; const Node& right;
};

namespace typeswitch {

string to_rpn(const Node& node) {
  if (auto expr = dynamic_cast<const Number*>(&node)) {
    return to_string(expr->value());
  } else if (auto expr = dynamic_cast<const Plus*>(&node)) {
    return to_rpn(expr->left) + " " + to_rpn(expr->right) + " +";
  } else if (auto expr = dynamic_cast<const Times*>(&node)) {
    return to_rpn(expr->left) + " " + to_rpn(expr->right) + " *";
  }
  throw runtime_error("unknown node type");
}
}

namespace visitor {

struct RPNVisitor : Node::Visitor {
  void accept(const Number& expr) {
    result = to_string(expr.val);
  }
  void accept(const Plus& expr) {
    expr.left.visit(*this);
    string l = result;
    expr.right.visit(*this);
    result = l + " " + result + " +";
  }
  void accept(const Times& expr) {
    expr.left.visit(*this);
    string l = result;
    expr.right.visit(*this);
    result = l + " " + result + " *";
  }
  string result;
};

string to_rpn(const Node& node) {
  RPNVisitor viz;
  node.visit(viz);
  return viz.result;
}

}

namespace funtable {

unordered_map<type_index, string (*)(const Node&)> RPNformatters;

string to_rpn(const Node& node) {
  return RPNformatters[typeid(node)](node);
}

struct Init {
  Init() {
    RPNformatters[typeid(Number)] = [](const Node& node) {
      return to_string(static_cast<const Number&>(node).val); };
    RPNformatters[typeid(Plus)] = [](const Node& node) {
      auto expr = static_cast<const Plus&>(node);
      return to_rpn(expr.left) + " " + to_rpn(expr.right) + " +"; };
    RPNformatters[typeid(Times)] = [](const Node& node) {
      auto expr = static_cast<const Times&>(node);
      return to_rpn(expr.left) + " " + to_rpn(expr.right) + " *"; };
  }
} init;
}

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

namespace openmethods {

register_classes(Node, Number, Plus, Times);

declare_method(string, to_rpn, (virtual_<const Node&>));

define_method(string, to_rpn, (const Number& expr)) {
  return std::to_string(expr.val);
}

define_method(string, to_rpn, (const Plus& expr)) {
  return to_rpn(expr.left) + " " + to_rpn(expr.right) + " +";
}

define_method(string, to_rpn, (const Times& expr)) {
  return to_rpn(expr.left) + " " + to_rpn(expr.right) + " *";
}

declare_method(int, value, (virtual_<const Node&>));

define_method(int, value, (const Number& expr)) {
  return expr.val;
}

define_method(int, value, (const Plus& expr)) {
  return value(expr.left) + value(expr.right);
}

define_method(int, value, (const Times& expr)) {
  return value(expr.left) * value(expr.right);
}
}

namespace virtual_ptr_demo {

using namespace yorel::yomm2;

declare_method(int, value, (virtual_ptr<const Node>));

int call_via_vptr(virtual_ptr<const Node> node) {
  return value(node);
}

define_method(int, value, (virtual_ptr<const Plus> expr)) {
  return value(expr->left) + value(expr->right);
}

auto make_node_ptr(const Node& node) {
  return virtual_ptr(node);
}

auto make_final_node_ptr(const Plus& node) {
  return final_virtual_ptr(node);
}

}

namespace core_api {

using namespace yorel::yomm2;

use_classes<Node, Number, Plus, Times> use_animal_classes;

struct value_id;
using value = method<value_id, int(virtual_<const Node&>)>;

int number_value(const Number& node) {
  return node.val;
}
value::add_function<number_value> add_number_value;

template<class NodeClass, class Op>
struct binary_value {
  static int fn(const NodeClass& expr) {
    return Op()(value::fn(expr.left), value::fn(expr.right));
  }
};

YOMM2_STATIC(value::add_definition<binary_value<Plus, std::plus<int>>>);
YOMM2_STATIC(value::add_definition<binary_value<Times, std::multiplies<int>>>);

}

int main() {
  Number n2(2), n3(3), n4(4);
  Plus sum(n3, n4);
  Times product(n2, sum);

  const Node& expr = product;
  cout << expr.value() << "\n";

  cout << expr.value() << "\n";
  cout << typeswitch::to_rpn(expr) << " = " << expr.value() << "\n";
  cout << visitor::to_rpn(expr) << " = " << expr.value() << "\n";
  cout << funtable::to_rpn(expr) << " = " << expr.value() << "\n";

  yorel::yomm2::update();
  cout << openmethods::to_rpn(expr) << " = " << expr.value() << "\n";

  cout << core_api::value::fn(expr) << "\n";

  return 0;
}
