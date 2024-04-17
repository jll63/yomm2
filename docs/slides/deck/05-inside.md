
# Inside yomm2

<!-- .slide: class="title"  -->
<!-- .slide: class="center" -->



## Inside YOMM2

* purely in C++17 (no extra tooling)

* constant time dispatch

* uses tables of function pointers

* object -> dispatch data?
  * perfect integer hash of `&type_info`



## A payroll application

* _Role_
    * Employee
        * Manager
    * Founder
* _Expense_
    * Cab, Jet
    * _Public_
        * Bus, Train



## the `pay` Uni- Method

```C++
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
    yomm2::default_policy>
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
        yomm2::default_policy>::
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



## update

uses the class and method info registered by static ctors

* build representation of class hierarchies

* calculate the hash and dispatch tables

* find a perfect (not minimal) hash function for the `type_info`s
  * H(x) = (M * x) >> S



## Dispatching a uni-method

* pretty much like virtual member functions

* method table contains a pointer to the effective function

* only it is not at a fixed offset in the method table



## Dispatching a uni-method

during `update`

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



## Dispatching a uni-method

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



## `approve` multi-method

```C++
declare_method(bool, approve,
  (virtual_<Role&>, virtual_<Expense&>, double));

define_method(bool, approve, (Role& r, Expense& e, double amount)) {
  return false;
}

define_method(bool, approve, (Employee& r, Public& e, double amount)) {
  return true;
}

define_method(bool, approve, (Manager& r, Taxi& e, double amount)) {
  return true;
}

define_method(bool, approve, (Founder& r, Expense& e, double amount)) {
  return true;
}
```



## Dispatching a multi-method

* it's a little more complicated

* use a multi-dimensional dispatch table

* size can grow very quickly

* the table must be "compressed", devoid of redundancies

* in fact the "uncompressed" table never exists

* work in terms of class _groups_, not classes



## Dispatching a multi-method

|          | Expense+Jet  | Public+Bus+Train     | Cab |
|----------|:------------:|:--------------------:|:---:|
| Role     | R,X          | R,X                  | R,X |
| Employee | R,X          | E,P                  | R,X |
| Manager  | R,X          | E,P                  | M,C |
| Founder  | F,X          | F,X                  | F,X |

(column major)



## Building the dispatch table

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
