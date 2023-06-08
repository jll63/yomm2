## Open Is Good

<br/><br/>

YOMM2: Fast, Orthogonal Open (Multi) Methods

<br/><br/>

<small>
Jean-Louis Leroy - jl@leroy.nyc<br/>
</small>

# Case Study: AST

## Case Study: AST

<br/>

```C++
struct Node {
    virtual double value() = 0;
};
```

const, memory management, etc, omitted<br/>
code formatted to fit slide

## Case Study: AST

<br/>

```C++
struct Number : Node {
  Number(double value) : val(value ) { }

  double value() override {
    return val;
  }

  double val;
};
```

## Case Study: AST

<br/>

```C++
struct Plus : Node {
    Plus(Node& left, Node& right)
    : left(left), right(right) { }

  double value() override {
    return left.value() + right.value();
  }

  Node &left, &right;
};
```

## Case Study: AST

<br/>

```C++
struct Times : Node {
    Times(Node& left, Node& right)
    : left(left), right(right) { }

    double value() override {
        return left.value() * right.value();
    }

    Node &left, &right;
};
```

## Case Study: AST

<br/>

```C++
int main() {
    Node* expr =
        new Times(
            *new Number(2),
            *new Plus(*new Number(3), *new Number(4)));

    cout << expr->value() << "\n"; // 14

    return 0;
}
```

## Case Study: AST

<br/>

```C++
int main() {
    Node* expr =
        new Times(
            *new Number(2),
            *new Plus(*new Number(3), *new Number(4)));

    cout << /* RPN */
         << " = " << expr->value() << "\n";
    // 2 3 4 * + = 14

    return 0;
}
```

## AST: Add a Virtual Function?

```C++
struct Node {
    // as before
    virtual string toRPN() = 0;
};

struct Plus : Node {
    // as before
  string toRPN() override {
    return left.toRPN() + " " + right.toRPN() + " +";
  }
};

// same for Number and Times

```

banana -> gorilla -> jungle

## AST: Type Switch?

```C++
string toRPN(Node& node) {
    if (auto expr = dynamic_cast<Number*>(&node)) {
        return to_string(expr->value());
    } else if (auto expr = dynamic_cast<Plus*>(&node)) {
        return toRPN(expr->left) + " " +
            toRPN(expr->right) + " +";
    } else if (auto expr = dynamic_cast<Times*>(&node)) {
        return toRPN(expr->left) + " " +
            toRPN(expr->right) + " *";
    }
    throw runtime_error("unknown node type");
}
```
needs modification each time a new Node subtype is added

## AST: Visitor?

```C++

struct Node {
    // as before
    struct Visitor {
        virtual void accept(Number& expr) = 0;
        virtual void accept(Plus& expr) = 0;
        virtual void accept(Times& expr) = 0;
    };

    virtual void visit(Visitor& viz) = 0;
};
```

## AST: Visitor?

```C++
struct Number : Node {
  // as before
  void visit(Visitor& viz) override { viz.accept(*this); }
};

struct Plus : Node {
  void visit(Visitor& viz) override { viz.accept(*this); }
};

struct Times : Node {
    void visit(Visitor& viz) override { viz.accept(*this); }
};
```


## AST: Visitor?

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
```

## AST: Visitor?

```C++

string toRPN(Node& node) {
  RPNVisitor viz;
  node.visit(viz);
  return viz.result;
}
```

* lot of work
* now it's difficult to add new types

## AST: Function Table?

```C++
using RPNFormatter = string (*)(Node&);
unordered_map<type_index, RPNFormatter> RPNformatters;

string toRPN(Node& node) {
  return RPNformatters[typeid(node)](node);
}
```

## AST: Function Table?

```C++
namespace { struct Init {
  Init() {
    RPNformatters[typeid(Number)] = [](Node& node) {
     return to_string(static_cast<Number&>(node).val); };
    RPNformatters[typeid(Plus)] = [](Node& node) {
     auto expr = static_cast<Plus&>(node);
     return toRPN(expr.left) + " " + toRPN(expr.right)
       + " +"; };
    // same for Time
  }
};
Init init;
} }
```

not bad, actually

# The Expression Problem

## The Expression Problem

<br/><br/>

### existing operations += new types

### existing types += new operations

## Multi-Layer Architectures

<br/><br/>

### presentation
<hr/>
### domain
<hr/>
### persistence

## Multi-Layer Architectures

<br/>

* presentation: PersonDlg, CriminalCaseDlg

* domain: Person, CriminalCase

* persistence: persist to database, to json...

* cross-cutting concerns

# Open Methods

## Open Methods

<br/><br/>

* free virtual functions

* existing types += new operations


## AST

```C++
#include <yorel/yomm2/keywords.hpp>

register_classes(Node, Plus, Times, Number);

declare_method(string, toRPN, (virtual_<Node&>));

define_method(string, toRPN, (Number& expr)) {
  return to_string(expr.val);
}

define_method(string, toRPN, (Plus& expr)) {
  return toRPN(expr.left) + " " + toRPN(expr.right) + " +";
}

// same for Times
```
## AST

```C++
int main() {
  yorel::yomm2::update_methods();

  cout << toRPN(expr) << " = " << expr->value() << "\n";
  // 2 3 4 * + = 14
}
```

## AST: what about value?

* `value` in the node hierarchy => interpreter

* AST classes should _only_ represent the tree

```C++
declare_method(int, value, (virtual_<Node&>));

define_method(int, value, (Number& expr)) {
  return expr.val;
}

define_method(int, value, (Plus& expr)) {
  return value(expr.left) + value(expr.right);
}
```

## Multiple Dispatch

Sometimes useful.

```text
add(Matrix, Matrix)                 -> Matrix
                                       add all elements
add(DiagonalMatrix, DiagonalMatrix) -> DiagonalMatrix
                                       just add diagonals

fight(Human, Creature, Axe)    -> not agile enough to wield
fight(Warrior, Creature, Axe)  -> chop it into pieces
fight(Warrior, Dragon, Axe)    -> die a honorable death
fight(Human, Dragon, Hands)    -> congratulations! you have just
                                  vanquished a dragon with your
                                  bare hands! (unbelievable,
                                  isn't it?)
```

## Syntax

`virtual_<>` denotes parameters taken into consideration when selecting the appropriate specialization

```C++
declare_method(
  string, fight,
  (virtual_<Character&>, virtual_<Creature&>, virtual_<Device&>));

define_method(string, fight, (Human& x, Creature& y, Axe& z)) {
  return "not agile enough to wield";
}
```

## Selecting the right specialization

<br/>

* works just like selecting from set of overloads (but at runtime!)

* or partial template specialization

* ambiguities can arise

## `next`

calls the next most specific specialization

```c++
register_classes(Animal, Dog, Bulldog);

declare_method(std::string, kick, (virtual_<Animal&>));

define_method(string, kick, (Dog& dog)) {
  return "bark";
}

define_method(string, kick, (Bulldog& dog)) {
    return next(dog) + " and bite";
}
```

# Is This OOP?

## Is This OOP?

* Simula, Smalltalk, C++/Java/D/...:<br/>message-receiver

* CLOS: rules

* algorithms retake the front stage

* no unnecessary breach of encapsulation

# Inside yomm2

* purely in C++17 (no extra tooling)

* constant time dispatch

* uses tables of function pointers

* object -> dispatch data?
  * perfect integer hash of &type_info

## A Payroll Application

* _Role_
    * Employee
        * Manager
    * Founder
* _Expense_
    * Cab, Jet
    * _Public_
        * Bus, Train

## the `pay` Uni- Method

```C++`
declare_method(double, pay, (virtual_<Employee&>));

define_method(double, pay, (Employee&)) {
  return 3000;
}

define_method(double, pay, (Manager& manager)) {
  return next(manager) + 2000;
}
```

## declare_method

```C++
declare_method(double, pay, (virtual_<Employee&>));
```


```C++
struct YoMm2_S_pay;
yomm2::method<
    YoMm2_S_pay, double(virtual_<const Employee&>),
    yomm2::policy::default_policy>
pay_yOMM2_selector_(
    yomm2::detail::remove_virtual<virtual_<const Employee&>> a0);
```

## declare_method

```C++
declare_method(double, pay, (virtual_<Employee&>));
```


```C++
inline double
pay(yomm2::detail::remove_virtual<virtual_<const Employee&>> a0) {
    return yomm2::method<
        YoMm2_S_pay, double(virtual_<const Employee&>),
        yomm2::policy::default_policy>::
        fn(std::forward<
            yomm2::detail::remove_virtual<virtual_<const Employee&>>>(
            a0));
};
```

## define_method

```C++
define_method(double, pay, (Employee&)) { return 3000; }
```

```C++
namespace { namespace YoMm2_gS_10 {
template<typename T> struct _yOMM2_select;
template<typename... A> struct _yOMM2_select<void(A...)> {
    using type = decltype(pay_yOMM2_selector_(std::declval<A>()...));
};
using _yOMM2_method = _yOMM2_select<void(const Employee&)>::type;
using _yOMM2_return_t = _yOMM2_method::return_type;
_yOMM2_method::function_pointer_type next;
struct _yOMM2_spec {
    static YoMm2_gS_10::_yOMM2_method::return_type
    yOMM2_body(const Employee&);
};
_yOMM2_method::add_function<_yOMM2_spec::yOMM2_body>
    YoMm2_gS_11(&next, typeid(_yOMM2_spec).name()); } }
```

## define_method

```C++
define_method(double, pay, (Employee&)) { return 3000; }
```

```C++
YoMm2_gS_10::_yOMM2_method::return_type
YoMm2_gS_10::_yOMM2_spec::yOMM2_body(const Employee&) {
    return 3000;
}
```

## update_methods

Uses the class and method info registered by static ctors.

* build representation of class hierarchies

* calculate the hash and dispatch tables

* find a perfect (not minimal) hash function for the `type_info`s
  * H(x) = (M * x) >> S

## Dispatching a Uni-Method

* pretty much like virtual member functions

* method table contains a pointer to the effective function

* only it is not at a fixed offset in the method table

## Dispatching a Uni-Method

during `update_methods`

```C++
method<pay>::slots_strides.i = 1;

// method table for Employee
mtbls[ H(&typeid(Employee)) ] = {
  ..., // used by approve,
  wrapper(pay(Employee&))
};

// method table for Manager
mtbls[ H(&typeid(Manager&)) ] = {
  ..., // used by approve,
  wrapper(pay(Manager&))
};
```

## Dispatching a Uni-Method

```C++
pay(bill)
```
=>
```C++
mtbls[ H(&typeid(bill)) ]          // mtable for type
  [ method<pay>::slots_strides.i ] // pointer to fun
(bill)                             // call
```

## Assembler

```C++
double call_pay(Employee& e) { return pay(e); }
```

```asm
mov	r8, qword ptr [rip + context+24]              ; hash table
mov	rdx, qword ptr [rip + context+32]             ; M
mov	cl, byte ptr [rip + context+40]               ; S
movsxd rsi, dword ptr [rip + method<pay>::fn+96]  ; slot
mov	rax, qword ptr [rdi]                          ; vptr
imul rdx, qword ptr [rax - 8]                     ; M * &typeid(e)
shr	rdx, cl                                       ; >> S
mov	rax, qword ptr [r8 + 8*rdx]                   ; method table
jmp	qword ptr [rax + 8*rsi]                       ; call wrapper
```

## `approve` Multi-Method

```C++
declare_method(bool, approve,
  (virtual_<Role&>, virtual_<Expense&>, double));

define_method(bool, approve,
  (Role& r, Expense& e, double amount))    { return false; }

define_method(bool, approve,
  (Employee& r, Public& e, double amount)) { return true; }

define_method(bool, approve,
  (Manager& r, Taxi& e, double amount))    { return true; }

define_method(bool, approve,
  (Founder& r, Expense& e, double amount)) { return true; }
```

## Dispatching a Multi-Method

* it's a little more complicated

* use a multi-dimensional dispatch table

* size can grow very quickly

* the table must be "compressed", devoid of redundancies

* in fact the "uncompressed" table never exists

* work in terms of class _groups_, not classes

## Dispatching a Multi-Method

|          | Expense+Jet  | Public+Bus+Train     | Cab |
|----------|:------------:|:--------------------:|:---:|
| Role     | R,X          | R,X                  | R,X |
| Employee | R,X          | E,P                  | R,X |
| Manager  | R,X          | E,P                  | M,C |
| Founder  | F,X          | F,X                  | F,X |

(column major)

## Building the Compressed Dispatch Table

* [Fast Algorithms for Compressed Multi-Method Dispatch, Eric Amiel, Eric Dujardin, Eric Simon, 1996](https://hal.inria.fr/inria-00073721/document)

* [Open Multi-Methods for C++11, Part 3 - Inside Yomm11: Data Structures and Algorithms, Jean-Louis Leroy, 2013](https://www.codeproject.com/Articles/859492/Open-Multi-Methods-for-Cplusplus-Part-Inside-Yomm)

## Dispatching a Multi-Method

```C++
method<approve>::.slots_strides.pw = { 0, 4, 0 };

mtbls[ H(&typeid(Employee)) ] = {
  // & of (Employee,Expense+Jet) cell
  // used by pay
};
mtbls[ H(&typeid(Manager)) ] = {
  // & of (Manager,Expense+Jet) cell
  // used by pay
};

mtbls[ H(&typeid(Expense)) ] = { 0 }; // also for Jet
mtbls[ H(&typeid(Public))  ] = { 1 }; // also for Bus, Train
mtbls[ H(&typeid(Cab))     ] = { 2 };
```

## Dispatching a Multi-Method

```C++
approve(bill, ticket, 5000)
```
=>
```C++
word* slots_strides = method<approve>::.slots_strides.pw;

mtbls[ H(&typeid(bill)) ]        // method table for bill
  [ slots_strides[0].i ]         // ptr to cell in 1st column
  [ mtbls [ H(&typeid(ticket)) ] // method table for ticket
    [ slots_strides[2].i ]       // column
    * slots_strides[1].i         // stride
  ]                              // pointer to function
(bill, ticket, 5000)             // call
```

## Benchmarks

|                     |          | gcc6 | clang6 |
|---------------------|----------|------|--------|
| normal inheritance  |          |      |        |
| virtual function    | 1-method | 16%  | 17%    |
| double dispatch     | 2-method | 25%  | 35%    |
| virtual inheritance |          |      |        |
| virtual function    | 1-method | 19%  | 17%    |
| double dispatch     | 2-method | 40%  | 33%    |

## yomm2 vs other systems

* Pirkelbauer - Solodkyi - Stroustrup (PSS)
* yomm11
* Cmm
* Loki / Modern C++

## yomm2 vs PSS

* Solodkyi's  papers on open methods etc:
    * [Open Multi-Methods for C++](http://www.stroustrup.com/multimethods.pdf)
    * [Design and Evaluation of C++ Open Multi-Methods](https://parasol.tamu.edu/~yuriys/papers/OMM10.pdf)
    * [Simplifying the Analysis of C++ Programs](http://oaktrust.library.tamu.edu/bitstream/handle/1969.1/151376/SOLODKYY-DISSERTATION-2013.pdf)
* PSS attempts harder to resolve ambiguities
* yomm2 overrides not visible as overloads, cannot specialize multiple methods
* yomm2 supports smart pointers, `next`

## yomm2 vs yomm11

* no need to instrument classes

* methods are ordinary functions

# Roadmap

* `virtual_ptr` (fat pointer)
* intrusive mode
* dispatch on `std::any`
* header-only
* tunable runtime
* allocators (surprised?)
* "static" mode

# QA Time

* github: https://github.com/jll63/yomm2
* contact: Jean-Louis Leroy - jl@leroy.nyc

<center>
<img src="qr.png" />
</center>
