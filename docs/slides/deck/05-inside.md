
# Inside YOMM2

<!-- .slide: class="title"  -->
<!-- .slide: class="center" -->



## Inside YOMM2

* pure C++17 (no extra tooling)

* "constant" time dispatch (proportional to #vargs)

* uses tables of function pointers

* object -> dispatch data?
  * perfect integer hash of `&typeid(obj)`



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

inline double
pay(yomm2::detail::remove_virtual<virtual_<Employee&>> a0) {
    return yomm2::method<
        YoMm2_S_pay, double(virtual_<Employee&>),
        yomm2::default_policy>::
        fn(std::forward<
            yomm2::detail::remove_virtual<virtual_<Employee&>>>(
            a0));
};

yomm2::method<
    YoMm2_S_pay, double(virtual_<Employee&>),
    yomm2::default_policy>
pay_yOMM2_selector_(
    yomm2::detail::remove_virtual<virtual_<Employee&>> a0);
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
using _yOMM2_method = _yOMM2_select<void(Employee&)>::type;
using _yOMM2_return_t = _yOMM2_method::return_type;
_yOMM2_method::function_pointer_type next;
struct _yOMM2_spec {
    static YoMm2_gS_10::_yOMM2_method::return_type
    yOMM2_body(Employee&);
};
_yOMM2_method::add_function<_yOMM2_spec::yOMM2_body>
    YoMm2_gS_11(&next, typeid(_yOMM2_spec).name()); } }
YoMm2_gS_10::_yOMM2_method::return_type
YoMm2_gS_10::_yOMM2_spec::yOMM2_body(Employee&) {
    return 3000;
}
```



## update

* uses class and method info registered by static ctors

* builds a representation of class hierarchies

* builds dispatch tables

* finds a perfect (not minimal) hash function for the `type_info`s
  * H(x) = (M * x) >> S



## Dispatching a uni-method

* pretty much like virtual member functions

* method table contains a pointer to the effective function

* only it is not at a fixed offset in the method table



## Dispatching a uni-method

during `update`

```C++
method<pay>::slots_strides[] = { 1 };

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
mtbls[ H(&typeid(bill)) ]           // mtable for type
  [ method<pay>::slots_strides[0] ] // pointer to fun
(bill)                              // call
```



## Assembler

```C++
double call_pay(Employee& e) { return pay(e); }
```

```asm
mov	  rax, qword ptr [rdi]                  ; vptr
mov	  rdx, qword ptr [rip + hash_mult]      ; M
imul	rdx, qword ptr [rax - 8]              ; M * &typeid(e)
movzx	ecx, byte ptr [rip + hash_shift]      ; S
shr	  rdx, cl                               ; >> S
mov	  rax, qword ptr [rip + vptrs]          ; vptrs
mov	  rax, qword ptr [rax + 8*rdx]          ; vptr
mov	  rcx, qword ptr [rip + slots_strides]  ; slot
jmp	  qword ptr [rax + 8*rcx]
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

* uses a multi-dimensional dispatch table

* size can grow very quickly

* table must be "compressed", devoid of redundancies

* in fact the "uncompressed" table never exists

* works in terms of class _groups_, not classes



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
method<approve>::slots_strides = { 0, 4, 0 };

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
approve(bill, ticket, 6)
```
=>
```C++
std::uintptr_t* slots_strides = method<approve>::.slots_strides;

mtbls[ H(&typeid(bill)) ]        // method table for bill
  [ slots_strides[0] ]           // ptr to cell in 1st column
  [ mtbls [ H(&typeid(ticket)) ] // method table for ticket
    [ slots_strides[2] ]         // column
    * slots_strides[1]           // stride
  ]                              // pointer to function
(bill, ticket, 6)                // call
```




## YOMM2 vs other systems

* Pirkelbauer - Solodkyi - Stroustrup (PSS)
* Cmm
* Loki / Modern C++



## YOMM2 vs PSS

* Solodkyi's  papers on open methods etc.:
    * [Open Multi-Methods for C++](http://www.stroustrup.com/multimethods.pdf)
    * [Design and Evaluation of C++ Open Multi-Methods](https://parasol.tamu.edu/~yuriys/papers/OMM10.pdf)
    * [Simplifying the Analysis of C++ Programs](http://oaktrust.library.tamu.edu/bitstream/handle/1969.1/151376/SOLODKYY-DISSERTATION-2013.pdf)
* PSS attempts harder to resolve ambiguities
* YOMM2 overrides not visible as overloads, cannot specialize multiple methods
* YOMM2 supports smart pointers, `next`
