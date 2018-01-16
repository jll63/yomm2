# From Yomm11 to Yomm2

While porting Yomm11 to the D language, I had a few fresh ideas, some of which
are relevant to the C++ library. In addition, my colleague David Goffredo
educated me on the power of the preprocessor's variadic macros. This prompted
me to completely rewrite yomm11. Here are the improvements:

## Only one mode: orthogonal _and_ fast

Yomm11 method dispatch is very fast, provided that you instrument all the
classes that partake in method calls. This means adding macro calls in several
places, and sometimes adding constructors. The purpose is to add a pointer to a
method table, which plays a role very similar to the virtual function table
created by the compiler.

As an alternative, yomm11 supports an orthogonal mode: the method table pointer
is stored in a global map indexed by the type_index of the virtual
arguments. No modification to existing classes is required, but method dispatch
is much slower (roughly by a factor of 10).

Yomm2 does away with class instrumentation. Classes need to be registered but
need not be modified. The method pointer is acquired from a collision free
global map indexed by a perfect integer hash of the address of the type_info
object found in the argument's virtual table. See the implementation notes for
more details. As a result, calling a method with one virtual argument is only
10-15% slower than calling the equivalent virtual member function.

## Method entry point is an ordinary function

In yomm11, methods are called via a function object created by the
`MULTI_METHOD` macro. Consequently, methods cannot be overloaded, because this
would create two objects with the same name.

In Yomm2, declaring a method creates a plain inline function. Thus it is
possible to create several methods that have the same name and differ only by
their signature. It is also possible to take a pointer to a method.

## Support for smart pointers

Yomm2 allows virtual arguments of `std::shared_ptr` type, in addition to
ordinary pointers and references. It is possible to add support for other
pointer types by specializing `yorel::yomm2::virtual_traits`.

## Method definitions need not live in the same namespace as the declaration

Yomm11 uses a template (declared by `MULTI_METHOD`) to contain the method's
specialization. They are now created in hidden namespaces.
