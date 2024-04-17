<!-- .slide: class="title"  -->
<!-- .slide: class="center" -->

# Open Methods



## Open Methods

<br/><br/>

* free virtual functions
  * i.e. virtual functions that exist outside of a class
* existing types += new operations



## Open Methods

```c++
struct Node {
  virtual string to_rpn(/*const Node* this*/) const = 0;
};
```

```c++
declare_method(string, to_rpn, (virtual_<const Node&>));
```
<small>Common Lisp: defgeneric, Clojure: defmulti</small>


<br/>

```c++
struct Plus : Node {
  string to_rpn(/*const Node* this*/) const override {
    return left.to_rpn() + " " + right.to_rpn() + " +";
  }
};
```

```c++
define_method(string, to_rpn, (const Plus& expr)) {
  return to_rpn(expr.left) + " " + to_rpn(expr.right) + " +";
}
```
<small>Common Lisp, Clojure: defmethod</small>




## AST

```C++
#include <yorel/yomm2/keywords.hpp>

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

int main() {
  yorel::yomm2::update();
  cout << to_rpn(expr) << " = " << expr.value() << "\n";
  return 0;
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




## Performance

* 15-30% slower than equivalent native virtual function call (but see `virtual_ptr`)

* [Optimizing Away C++ Virtual Functions May Be
  Pointless](https://www.youtube.com/watch?v=i5MAXAxp_Tw)  - Shachar Shemesh -
  CppCon 2023

```asm
	mov	  rax, qword ptr [rdi]
	mov	  rdx, qword ptr [rip+fast_perfect_hash<release>::mult]

	imul	rdx, qword ptr [rax-8]
	mov	  cl, byte ptr [rip+fast_perfect_hash<release>::shift]

	shr	  rdx, cl
	mov	  rax, qword ptr [rip+vptr_vector<release>::vptrs]

	mov	  rax, qword ptr [rax+8*rdx]
	mov	  rcx, qword ptr [rip+method<value, int (virtual_<Node const&>)::fn+80]

	jmp	  qword ptr [rax+8*rcx]
```



## Multiple Dispatch

sometimes useful

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


* works just like selecting from set of overloads (but at runtime!)

* ambiguities can arise




## Poll

### Is this OOP?

<br/>

Only one vote!

1. Yes
2. No



## Poll

### What do you prefer?

<br/>

Only one vote!

1. virtual function, type switch, visitor, function table
2. open methods
