# YOMM2

This library implements fast, open, multi-methods for C++17. It is strongly
inspired by the papers by Peter Pirkelbauer, Yuriy Solodkyy, and Bjarne
Stroustrup.
- [YOMM2](#yomm2)
  - [TL;DR](#tldr)
  - [Open Methods in a Nutshell](#open-methods-in-a-nutshell)
    - [Cross-cutting Concerns and the Expression Problem](#cross-cutting-concerns-and-the-expression-problem)
    - [Multiple Dispatch](#multiple-dispatch)
  - [Performance](#performance)
  - [Building and Installing](#building-and-installing)
  - [Going Further](#going-further)
  - [Roadmap](#roadmap)

## TL;DR

If you are familiar with the concept of open multi-methods, or if you prefer
to learn by reading code, go directly to [the
synopsis](examples/synopsis.cpp). The [reference is
here](reference/README.md)

## Open Methods in a Nutshell

### Cross-cutting Concerns and the Expression Problem

You have a matrix math library. It deals with all sort of matrices: dense,
diagonal, tri-diagonal, etc. Each matrix subtype has a corresponding class in a
hierarchy rooted in Matrix.

Now you would like to render Matrix objects as JSON strings. The representation
will vary depending on the exact type of the object; for example, if a matrix
is a DiagonalMatrix, you only need to store the diagonal - the other elements
are all zeroes.

This is an example of a ["cross-cutting
concern"](http://wiki.c2.com/?CrossCuttingConcern). How do you do it?

It turns out that OOP doesn't offer a good solution to this.

You can stick a pure virtual `to_json` function in the `Matrix` base class and
override it in the subclasses. It is an easy solution but it has severe
drawbacks. It requires you to change the Matrix class and its subclasses, and
recompile the library. And now all the applications that use it will contain
the `to_json` functions even if they don't need them, because of the way
virtual functions are implemented.

Or you may resort on a "type switch": have the application test for each
category and generate the JSON accordingly. This is tedious, error prone and,
above all, not extensible. Adding a new matrix subclass requires updating all
the type switches. The Visitor pattern also suffers from this flaw.

Wouldn't it be nice if you could add behavior to existing types, just as easily
and unintrusively as you can extend existing class hierarchies via derivation?
What if you could solve the so-called [Expression
Problem](http://wiki.c2.com/?ExpressionProblem):

```
existing behaviors += new types
existing types += new behaviors
```

This is exactly what Open Methods are all about: solving the Expression
Problem.

Let's look at an example.


```c++


// -----------------------------------------------------------------------------
// library code

struct matrix {
    virtual ~matrix() {}
    // ...
};

struct dense_matrix    : matrix { /* ... */ };
struct diagonal_matrix : matrix { /* ... */ };

// -----------------------------------------------------------------------------
// application code

#include <yorel/yomm2/keywords.hpp>

register_classes(matrix, dense_matrix, diagonal_matrix);

declare_method(std::string, to_json, (virtual_<const matrix&>));

define_method(std::string, to_json, (const dense_matrix& m)) {
    return "json for dense matrix...";
}

define_method(std::string, to_json, (const diagonal_matrix& m)) {
    return "json for diagonal matrix...";
}

int main() {
    yorel::yomm2::update_methods();

    const matrix& a = dense_matrix();
    const matrix& b = diagonal_matrix();

    std::cout << to_json(a) << "\n"; // json for dense matrix
    std::cout << to_json(b) << "\n"; // json for diagonal matrix

    return 0;
}


```


The `declare_method` line declares an open method called `to_json`that takes
one virtual argument of type `const matrix&` and returns a std::string. The
`virtual_<>` decorator specifies that the argument must be taken into account
to select the appropriate specialization. In essence, this is the same thing
as having a `virtual std::string to_json() const` inside class Matrix -
except that the virtual function lives outside of any classes, and you can
add as many as you want without changing the classes.

NOTE: DO NOT specify argument names, i.e. `virtual_<const matrix&> arg` is not
permitted.

The following `define_method` blocks define two implementations for the
`to_json` method: one for dense matrices, and one for diagonal matrices.

`yorel::yomm2::update_methods()` must be called before any method is called,
and after dynamically loading and unloading shared libraries.

### Multiple Dispatch

Methods can have more than one virtual argument. This is handy in certain
situations, for example to implement binary operations on matrices:

```c++

// -----------------------------------------------------------------------------
// matrix * matrix

declare_method(
    shared_ptr<const matrix>,
    times,
    (virtual_<shared_ptr<const matrix>>, virtual_<shared_ptr<const matrix>>));

// catch-all matrix * matrix -> dense_matrix
define_method(
    shared_ptr<const matrix>,
    times,
    (shared_ptr<const matrix> a, shared_ptr<const matrix> b)) {
    return make_shared<dense_matrix>();
}

// diagonal_matrix * diagonal_matrix -> diagonal_matrix
define_method(
    shared_ptr<const matrix>,
    times,
    (shared_ptr<const diagonal_matrix> a, shared_ptr<const diagonal_matrix> b)) {
    return make_shared<diagonal_matrix>();
}
```

## Performance

Open methods are almost as fast as ordinary virtual member functions once you
turn on optimization (-O2). With both `clang` and `gcc`, calling a method with
one virtual argument takes 15-30% more time than calling the equivalent virtual
member function. If the body of the method does any amount of work, the
difference is unnoticeable. If the direct [intrusive
mode](reference/intrusive_modes.md) is used, the overhead drops to 2%.

## Building and Installing

Make sure that you have the following dependencies:

* a C++17 capable optimising compiler

* `cmake` version 3.20 or above

Clone the repository:

```
git clone https://github.com/jll63/yomm2.git
cd yomm2
```

Create a build directory and run cmake then make:

```
mkdir build
cd build
cmake ..
make
```

By default, YOMM2 is built as a static library. It can also be built as a shared
library by adding -DYOMM2_SHARED=1 to the `cmake` invocation.

If you want to run the tests:

```
cmake .. -DYOMM2_ENABLE_TESTS=1
make && ctest
```

YOMM2 uses several Boost libraries:

1. Preprocessor, DynamicBitset, TypeTraits: included by YOMM2 headers

2. Boost.Test: only used to run the test suite

If these libraries are already available on your machine, and they can be found
by `cmake`, they will be used. In this case, make sure that the pre-installed
libraries are at version 1.74 or above. If Boost is not found, the latest
version will be downloaded, and the Boost headers mentioned in section (1) will
be installed along YOMM2 (if you decide to `make install`).

If you also want to run the benchmarks (and in this case you really want a
release build):

```
cmake .. -DYOMM2_ENABLE_TESTS=1 -DYOMM2_ENABLE_BENCHMARKS=1 -DCMAKE_BUILD_TYPE=Release
make && tests/benchmarks # wow it's fast!
```
This will automatically download the dependency
[benchmark](https://github.com/google/benchmark), build it and finally install
it to `./extern` within the root directory of yomm2.

Finally, if you like it and you want to install it:

```
# either:
sudo make install
# or:
make install DESTDIR=/path/to/my/libs
```
Once this is done, link with `libyomm2.a` or `libyomm2.so`. For example:

```
clang++ -std=c++17 synopsis.cpp -o synopsis -lyomm2
```

A CMake package configuration is also installed. If the install location is in
`CMAKE_PREFIX_PATH`, you can use `find_package(YOMM2)` to locate YOMM2, then
`target_link_libraries(<your_target> YOMM2::yomm2)` to add the necessary include
paths and the library. See [this example](examples/cmakeyomm2).

## Going Further

The Reference is [here](reference/README.md). Since version 1.3.0, some of
the internals are documented, which make it possible to use the library
without using the macros - see [the API tutorial](tutorials/api.md).

YOMM2 has *experimental* support for writing templatized methods and
definitions - see [the templates tutorial](tutorials/templates_tutorial.md).

The library comes with a series of examples:

* [The complete `matrix` example](examples/matrix.cpp)

* [The Asteroids example used in Wikipedia's article on Multiple Dispatch](examples/asteroids.cpp)

* [Process an AST sans clumsy Visitor](examples/accept_no_visitors.cpp)

* [Adventure: a 3-method example](examples/adventure.cpp)

* [friendship: an example with namespaces, method containers and friend
  declarations](examples/containers)

I presented the library at CppCon 2018. Here are [the video recording](https://www.youtube.com/watch?v=xkxo0lah51s) and [the slides](https://jll63.github.io/yomm2/slides/).

## Roadmap

YOMM2 has been stable (in the sense of being backward-compatible) for a years,
but it is still evolving. Here are the items on which I intend to work in the
future. No promises, no time table!

* Speed up dispatch in presence of virtual inheritance.
* Intrusive mode, &agrave; la YOMM11, for faster dispatch.
* Fat pointers, carrying the method table pointer, for faster dispatch.
* Static linking of dispatch data.
* *Minimal* perfect hash tables as an option.
* Multi-threaded hash search.
* Make error handler a `std::function`.
* Get closer to Stroustrup et al's papers (version 2.0):
  * use covariant return types for disambiguation
  * move support for `shared_ptr` and `unique_ptr` to an optional header
* Go header-only.
* 
If you have ideas, comments, suggestions...get in touch! If you use YOMM2, I
would appreciate it if you take the time to send me a description of your use
case(s), and links to the project(s), if they are publicly available.