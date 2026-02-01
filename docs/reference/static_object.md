> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

# YOMM2 static objects



// YOMM2 uses instances of various classes to register user classes, methods, and
method definitions. The constructors adds the object to an intrusive forward
list, and relies on underlying plain pointers to be zero-initialized. Thus,
these objects must be defined at file scope, or as as `static` variables inside
a function or a class.

The static objects constructor do not allocate memory from the heap.

## See also

The [`YOMM2_STATIC`](/yomm2/reference/YOMM2_STATIC.html) macro is a convenient way of creating static registration
objects with an obfuscated name.
