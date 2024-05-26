/***
# YOMM2

[![CI](https://github.com/jll63/yomm2/actions/workflows/main.yml/badge.svg)](https://github.com/jll63/yomm2/actions/workflows/main.yml)
[![ConanCenter package](https://repology.org/badge/version-for-repo/conancenter/yomm2.svg)](https://repology.org/project/yomm2/versions)
[![Vcpkg package](https://repology.org/badge/version-for-repo/vcpkg/yomm2.svg)](https://repology.org/project/yomm2/versions)
---

This library implements fast, open, multi-methods for C++17. It is strongly
inspired by the papers by Peter Pirkelbauer, Yuriy Solodkyy, and Bjarne
Stroustrup.

- [YOMM2](#yomm2)
  - [](#)
  - [TL;DR](#tldr)
  - [Open Methods in a Nutshell](#open-methods-in-a-nutshell)
    - [Cross-cutting Concerns and the Expression Problem](#cross-cutting-concerns-and-the-expression-problem)
    - [Multiple Dispatch](#multiple-dispatch)
  - [Performance](#performance)
  - [Installation](#installation)
  - [Going Further](#going-further)
  - [Roadmap](#roadmap)

## TL;DR

If you are familiar with the concept of open multi-methods, or if you prefer
to learn by reading code, go directly to [the
synopsis](examples/synopsis.cpp). The [documentation is
here](https://jll63.github.io/yomm2)

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

***/

//***

// -----------------------------------------------------------------------------
// library code

struct matrix {
    virtual ~matrix() {
    }
    // ...
};

struct dense_matrix : matrix { /* ... */
};
struct diagonal_matrix : matrix { /* ... */
};

// -----------------------------------------------------------------------------
// application code

#include <memory>
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
    yorel::yomm2::update();

    const matrix& a = dense_matrix();
    const matrix& b = diagonal_matrix();

    std::cout << to_json(a) << "\n"; // json for dense matrix
    std::cout << to_json(b) << "\n"; // json for diagonal matrix

    return 0;
}

//***

/***

`<yorel/yomm2/keywords.hpp>` is the library's main entry point. It declares a
set of macros, and injects a single name, ->`virtual_`, in the global
namespace. The purpose of the header is to make it look as if open methods
are part of the language.

->`register_classes` informs the library of the existence of the classes, and
their inheritance relationships. Any class that can appear in a method call
needs to be registered, even if it is not directly referenced by a method.

`declare_method` declares an open method called `to_json`, which takes one
virtual argument of type `const matrix&` and returns a std::string. The
`virtual_<>` decorator specifies that the argument must be taken into account
to select the appropriate specialization. In essence, this is the same thing
as having a `virtual std::string to_json() const` inside class Matrix -
except that the virtual function lives outside of any classes, and you can
add as many as you want without changing the classes. NOTE: DO NOT specify
argument names, i.e. `virtual_<const matrix&> arg` is _not permitted_.

`define_method` defines two implementations for the `to_json` method: one for
dense matrices, and one for diagonal matrices.

`yorel::yomm2::update()` creates the dispatch tables; it must be called
before any method is called, and after dynamically loading and unloading
shared libraries.

The example can be compiled (from the root of the repository) with:
```shell
clang++- -I include -std=c++17 tutorials/README.cpp -o example
```

### Multiple Dispatch

Methods can have more than one virtual argument. This is handy in certain
situations, for example to implement binary operations on matrices:
***/

//***

// -----------------------------------------------------------------------------
// matrix * matrix

declare_method(
    std::shared_ptr<const matrix>,
    times, (virtual_<const matrix&>, virtual_<const matrix&>));

// catch-all matrix * matrix -> dense_matrix
define_method(
    std::shared_ptr<const matrix>,
    times, (const matrix& a, const matrix& b)) {
    return std::make_shared<dense_matrix>();
}

// diagonal_matrix * diagonal_matrix -> diagonal_matrix
define_method(
    std::shared_ptr<const matrix>,
    times, (const diagonal_matrix& a, const diagonal_matrix& b)) {
    return std::make_shared<diagonal_matrix>();
}
//***

/***
## Performance

Open methods are almost as fast as ordinary virtual member functions once you
turn on optimization (-O2). With both clang and gcc, dispatching a call to a
method with one virtual argument takes 15-30% more time than calling the
equivalent virtual member function (unless the call goes through a virtual base,
which requires a dynamic cast). It does not involve branching or looping, only a
few memory reads (which the CPU can be parallelize), a multiplication, a bit
shift, a final memory read, then an indirect call. If the body of the method
does any amount of work, the difference is unnoticeable.

[`virtual_ptr`](https://jll63.github.io/yomm2/reference/virtual_ptr.md), a fat
pointer class, can be used to make method dispatch even faster - three
instructions and two memory reads.

[Examples](ce/README.md) are available on Compiler Explorer.

## Installation

YOMM2 is available on both major package managers. This is the easiest way of
integrating it in your project, along with its dependencies. See [the vcpkg
example](examples/vcpkg) and [the Conan2 example](examples/conan).

YOMM2 can also be built and installed from the sources, using straight `cmake`.

First clone the repository:

```
git clone https://github.com/jll63/yomm2.git
```

Run cmake:

```
cmake -S yomm2 -B build.yomm2
cmake --build build.yomm2
```

If you want to run the tests, specify it when running `cmake`:

```
cmake -S yomm2 -B build.yomm2 -DYOMM2_ENABLE_TESTS=1
cmake --build build.yomm2
ctest --test-dir build.yomm2
```

YOMM2 uses the following Boost libraries:
* Preprocessor, DynamicBitset: included by YOMM2 headers
* Boost.Test: only used to run the test suite

If these libraries are already available on your machine, and they can be found
by `cmake`, they will be used. In this case, make sure that the pre-installed
libraries are at version 1.74 or above. You can also tell the cmake script to
attempt to download and build the dependencies by setting
`YOMM2_DOWNLOAD_DEPENDENCIES` to `ON`.

If you want to run the benchmarks (and in this case you really want a release
build):

```
cmake -S yomm2 -B build.yomm2 -DYOMM2_ENABLE_TESTS=1 -DYOMM2_ENABLE_BENCHMARKS=1 -DCMAKE_BUILD_TYPE=Release
./build.yomm2/tests/benchmarks
```
The benchmarks use the [Google benchmark](https://github.com/google/benchmark)
library. Again, if it is not found, and `YOMM2_DOWNLOAD_DEPENDENCIES` is `ON`,
it will be built from its source.

If you like YOMM2, and you want to install it, either system-wide:

```
sudo cmake --install build.yomm2
```

...or to a specific directory:

```
DESTDIR=/path/to/my/libs cmake --install build.yomm2
```

This will install the headers and a CMake package configuration. By default,
YOMM2 is installed as a headers only library. The examples can be compiled like
this (after installation):

```
clang++ -std=c++17 -O3 examples/synopsis.cpp -o synopsis
```

Or directly from the repository (i.e. without installing):

```
clang++ -std=c++17 -O3 -Iinclude examples/synopsis.cpp -o synopsis
```

The YOMM2 runtime - responsible for building the dispatch tables - adds ~75K to
the image, or ~64K after stripping.

The runtime can also be built and installed as a shared library, by adding
-DYOMM2_SHARED=1 to the `cmake` command line.

A CMake package configuration is also installed. If the install location is in
`CMAKE_PREFIX_PATH`, you can use `find_package(YOMM2)` to locate YOMM2, then
`target_link_libraries(<your_target> YOMM2::yomm2)` to add the necessary include
paths and the library. See [this example](examples/cmakeyomm2).

Make sure to add the install location to `CMAKE_PREFIX_PATH` so that you can use
`find_package(YOMM2)` from your including project. For linking, the use
`target_link_library(<your_target> YOMM2::yomm2)`. This will automatically add
the necessary include directories, so this should be all you need to do to link
to yomm2.

## Going Further

The documentation is [here](https://jll63.github.io/yomm2). Since version 1.3.0,
some of the internals are documented, which make it possible to use the library
without using macros - see [the API
tutorial](https://jll63.github.io/yomm2/tutorials/api.html).

YOMM2 has *experimental* support for writing templatized methods and definitions
- see [the templates
  tutorial](https://jll63.github.io/yomm2/tutorials/templates_tutorial.html).

The library comes with a series of examples:

* [The complete `matrix` example](examples/matrix.cpp)

* [The Asteroids example used in Wikipedia's article on Multiple
  Dispatch](examples/asteroids.cpp)

* [Process an AST sans clumsy Visitor](examples/accept_no_visitors.cpp)

* [Adventure: a 3-method example](examples/adventure.cpp)

* [friendship: an example with namespaces, method containers and friend
  declarations](examples/containers)

I presented the library at CppCon 2018. Here are [the video
recording](https://www.youtube.com/watch?v=xkxo0lah51s) and [the
slides](https://jll63.github.io/yomm2/slides/).

## Roadmap

YOMM2 has been stable (in the sense of being backward-compatible) for many
years, but it is still evolving. Here are the items on which I intend to work in
the future. No promises, no time table.

* Dispatch on `std::any`.
* Static offsets (i.e. set at compile time).
* Static linking of dispatch data.
* *Minimal* perfect hash tables as an option.
* Multi-threaded hash search.
* Get closer to Stroustrup et al's papers (version 2.0):
  * use compatible return types for disambiguation
  * move support for `std::shared_ptr` and `unique_ptr` to an optional header

If you have ideas, comments, suggestions...get in touch! If you use YOMM2, I
would appreciate it if you take the time to send me a description of your use
case(s), and links to the project(s), if they are publicly available.
***/
