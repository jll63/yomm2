# Benchmarking YOMM2

The Google benchmark for YOMM2 - and, before that, YOMM11 - has been very simple
until now: call the same virtual function on the same object; call the same,
equivalent uni-method on the object; and compare. Do the same for a
dual-dispatch virtual function call and a call to the equivalent, 2-arguments,
method.

This is simplistic, because the data accessed by these calls is copied to the L1
cache during the first iteration, and read from it in the subsequent ones. Since
open methods require more memory reads than virtual functions, the situation
with the L1 cache gives an unfair advantage to YOMM2.

Version 1.3.0 introduced support for mixing open methods and templates without
too much difficulty. This makes it possible to generate class hierarchies and
methods that for them. I was thus able to rewrite the benchmarks in a more
realistic manner.

Before getting into the benchmark results, I will first explain why YOMM2
methods are so fast, but still slower than ordinary virtual functions.

## A Peek Inside Method Calls

```c++
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/symbols.hpp>

using namespace yorel::yomm2;

struct Dog {
    virtual ~Dog() {}
};

use_classes<Dog> YOMM2_GENSYM;

struct kick_;
using kick = method<kick_, const char*(virtual_<Dog&>)>;

const char* bark(Dog&) {
    return "bark";
}

kick::add_function<bark> YOMM2_GENSYM;
```

Here is the equivalent method, using the keyword interface:

```c++
register_classes(Dog);

declare_method(const char*, kick, (virtual_<Dog&>));

define_method(const char*, kick, (Dog&)) {
    return "bark";
}
```

YOMM2 generates code for `kick::fn(obj)` that is essentially equivalent to:

```c++
auto call_kick_manual(Dog& obj) {
    const auto hash_table = policy::global_context::context.hash_table;
    const auto mult = policy::global_context::context.hash.mult;
    const auto shift = policy::global_context::context.hash.shift;
    const auto index = kick::fn.slots_strides.i;
    const auto vptr = *(void***) &obj; // typeid 1
	// movq	    context+24(%rip), %r8
	// movq	    context+32(%rip), %rdx
	// movb	    context+40(%rip), %cl
	// movslq	method<kick_, char const* (virtual_<Dog&>)>::fn+96(%rip), %rsi
	// movq	    (%rdi), %rax

    const auto h1 = key * std::uintptr_t(vptr[-1]/* typeid 2*/);
	// imulq	-8(%rax), %rdx

    const auto h2 = h1 << shift;
	// shrq	    %cl, %rdx

    const auto method_table = hash_table[h2].pw;
	// movq	    (%r8,%rdx,8), %rax

    const auto fptr = method_table[index].pw;
    return ((const char*(*)(Dog&)) fptr)(obj);
	// jmpq	    *(%rax,%rsi,8)                  # TAILCALL
}
```

This amounts to 9 instructions, and 7 memory reads. In contrast, calling a
virtual function takes only 2 instructions, and 2 memory reads. It seems that
calling a uni-method should be significantly slower than calling the equivalent
virtual function. Note, however, that there are no `if`s and no loops in this
code. That alone makes YOMM2 much faster than other implementations.

When we look at the nine instructions more closely, though, we can see that
first five are independent; thus they can be executed in parallel. Moreover, the
first three memory reads are from consecutive locations. Reading the first
brings the other two in the cache, so the first three accesses actually count as
one. Moreover, these three reads are the same regardless of the method that is
called. Thus, in a program that makes intensive use of YOMM2, these three values
are likely to be in the L1 cache permanently.

This brings the cost of calling a uni-method essentially to 5 sequential (groups
of instructions and 5 (or even 4) memory reads.

Intuitively, one would expect a uni-method dispatch to take around 2.5 the time
it takes to dispatch a virtual function.

Let's now look at the benchmark results.

## The Benchmark

The benchmark now generates a (configurable) number of hierarchies. They all
look the same, namely, they are the hierarchy used in the synopsis example,
except that they are now templates that take various policies (like, use
ordinary or virtual inheritance), and a `std::integral_constant`. Template
meta-programming is then used to generate the desired number of hierarchies.

The new benchmarks also allocates a number of instances of each hierarchy. The
amount of memory allocated to each object is randomized, to prevent the
instances from being laid in memory according to a regular pattern.

Each benchmark test case runs a loop that picks a hierarchy at random, then two
objects from it at random, then perform a call to a virtual function or a
method. A special test case does not make the call; it is used to measure (and
remove) the overhead of the benchmark apparatus.

## The Results

## Analysing the Impact of Size

## Analysing Cache Misses

## Conclusion

When I began working on the successor of YOMM11 (which requires instrumenting
the classes), I would have gladly signed up for a 200-300% overhead compared to
virtual functions - for functions that have an empty body, mind you. This is not
the most common application of polymorphism, whether it is backed by virtual
functions or open methods, but it does occur, for example, in windowing or
application frameworks (like MFC, the Microsoft Foundation Classes).

I am still puzzled why YOMM2 performs so well. If you have a clue, or
suggestions about improving the benchmark, please let me know.

