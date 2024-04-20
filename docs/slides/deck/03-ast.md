# Case Study




## Abstract Syntax Tree


```C++
struct Node {
  virtual ~Node() {}
  virtual int value() const = 0;
};

struct Number : Node {
  explicit Number(int value) : val(value) { }
  int value() const override { return val; }
  int val;
};

struct Plus : Node {
  Plus(const Node& left, const Node& right) : left(left), right(right) { }
  int value() const override { return left.value() + right.value(); }
  const Node& left; const Node& right;
};

struct Times : Node {
  Times(const Node& left, const Node& right) : left(left), right(right) { }
  int value() const override { return left.value() * right.value(); }
  const Node& left; const Node& right;
};
```



## AST

<br/>

```C++
int main() {
  Number n2(2), n3(3), n4(4);
  Plus sum(n3, n4);
  Times product(n2, sum);

  const Node& expr = product;
  cout << expr.value() << "\n";

  return 0;
}
```

Output:
```
14
```



## Add an operation

<br/><br/>

```C++
cout << to_rpn(expr) << " = " << expr->value() << "\n";
//      ^^^^^^
```

Output:
```
2 3 4 * + = 14
```




## Virtual function?

```C++
struct Node {
  // as before
  virtual string to_rpn() const = 0;
};

struct Number : Node {
  // as before
  string to_rpn() const override { return to_string(val); }
};

struct Plus : Node {
  // as before
  string to_rpn() const override { return left.to_rpn() + " " + right.to_rpn() + " +"; }
};

struct Times : Node {
  // as before
  string to_rpn() const override { return left.to_rpn() + " " + right.to_rpn() + " &"; }
};
```

<p class="fragment"><em>
banana -> gorilla -> jungle<br/>
(C) Erlang creator Joe Armstrong
</em></p>



## Type switch?

```C++
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
```

* operations += types: nope




## Visitor?

```C++
struct Node {
  // as before
  struct Visitor {
    virtual void accept(const Number& expr) = 0;
    virtual void accept(const Plus& expr) = 0;
    virtual void accept(const Times& expr) = 0;
  };
  virtual void visit(Visitor& viz) const = 0;
};

struct Number : Node {
  // as before
  void visit(Visitor& viz) override { viz.accept(*this); }
};

struct Plus : Node {
  void visit(Visitor& viz) override { viz.accept(*this); }
};
// etc.
```




## Visitor...

```C++
struct RPNVisitor : Node::Visitor {
  string result;
  void accept(Number& expr) {
    result = to_string(expr.val);
  }
  void accept(Plus& expr) {
    expr.left.visit(*this);
    string l = result;
    expr.right.visit(*this);
    result = l + " " + result + " +";
  }
  void accept(Times& expr) { ... }
};

string to_rpn(Node& node) {
  RPNVisitor viz;
  node.visit(viz);
  return viz.result;
}
```


* a lot of work
* more visitors, or more complexity (non-const...)
* operations += types: nope



## Function Table?

```C++
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
```

* types += operations: yes
* operations += types: yes



## Poll

Only one vote!

<ol>
  <li>virtual function</li>
  <li>type switch</li>
  <li>visitor</li>
  <li>function table</li>
  <li class="fragment">they all stink</li>
</ol>
