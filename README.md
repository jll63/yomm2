# YOMM2

This is a complete rewrite of YOMM11, which is now deprecated. This library is
much better, see [here](yomm11-yomm2.md) to find out more.

## TL;DR

If you are familiar with the concept of open multi-methods, or prefer to learn
by reading code, go directly to [the synopsis](examples/synopsis.cpp)

## Open Methods in a Nutshell

### Cutting Concerns

You have a matrix math library. It deals with all sort of matrices: dense,
diagonal, tri-diagonal, etc. Each matrix subtype has a corresponding class in a
hierarchy rooted in Matrix.

Now you would like to render Matrix objects as JSON strings. The
representation will vary depending on the exact type of the object; for
example, if the matrix is a DiagonalMatrix, you only need to store the
diagonal - the other elements are all zeroes.

This is an example of a "cutting concern". How do you do it?

It turns out that OOP doesn't offer a good solution to this.

You can stick a virtual `to_json` function in the `Matrix` base class and
override it in the subclasses. It is an easy solution but it has severe
drawbacks. It requires you to change the Matrix class and its subclasses, and
recompile the library. And now all the applications that use it will contain
the `to_json` functions even if they don't need it, because of the way virtual
functions are implemented.

Or you may resort on a "type switch": have the application test for each
category and generate the JSON accordingly. This is tedious, error prone and,
above all, not extensible. Adding a new matrix subclass requires updating all
the type switches. The Visitor pattern also suffers from this problem.

Open methods provide a simple, elegant and efficient solution to this
problem. Let's look at an example:

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

#include <yorel/yomm2/cute.hpp>

using yorel::yomm2::virtual_;

register_class(matrix);
register_class(dense_matrix, matrix);
register_class(diagonal_matrix, matrix);

declare_method(string, to_json, (virtual_<const matrix&>));

begin_method(string, to_json, (const dense_matrix& m)) {
    return "json for dense matrix...";
} end_method;

begin_method(string, to_json, (const diagonal_matrix& m)) {
    return "json for diagonal matrix...";
} end_method;

int main() {
    yorel::yomm2::update_methods();

    shared_ptr<const matrix> a = make_shared<dense_matrix>();
    shared_ptr<const matrix> b = make_shared<diagonal_matrix>();

    cout << to_json(*a) << "\n"; // json for dense matrix
    cout << to_json(*b) << "\n"; // json for diagonal matrix

    return 0;
}
```

The `declare_method` line declares an open method called `to_json`that takes
one virtual argument of type `const matrix&` and returns a string. The
`virtual_<>` decorator specifies that the argument must be taken into account
to select the appropriate specialization. In essence, this is the same thing as
having a `virtual string to_json() const` inside class Matrix - except
that the virtual function lives outside of any classes, and you can add as many
as you want without modifying the classes.

NOTE: DO NOT specify argument names, i.e. `virtual_<const matrix&> arg` is not
permitted.

The following two `begin_method ... end_method` blocks define two
implementations for the `to_json` method: one for dense matrices, and one for
diagonal matrices.

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
begin_method(
    shared_ptr<const matrix>,
    times,
    (shared_ptr<const matrix> a, shared_ptr<const matrix> b)) {
    return make_shared<dense_matrix>();
} end_method;

// diagonal_matrix * diagonal_matrix -> diagonal_matrix
begin_method(
    shared_ptr<const matrix>,
    times,
    (shared_ptr<const diagonal_matrix> a, shared_ptr<const diagonal_matrix> b)) {
    return make_shared<diagonal_matrix>();
} end_method;
```

## Going Further

The Reference is [here](REFERENCE.md).

The library comes with a series of examples:

* [The complete `matrix` example](examples/matrix.cpp)

* [Process an AST sans clumsy Visitor](examples/accept_no_visitors.cpp)
