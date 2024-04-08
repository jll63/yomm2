# YOMM2 static objects

hrefs: static_object

// YOMM2 uses instances of various classes to register user classes, methods, and
method definitions. The constructors adds the object to an intrusive forward
list, and relies on underlying plain pointers to be zero-initialized. Thus,
these objects must be defined at file scope, or as as `static` variables inside
a function or a class.

The static objects constructor do not allocate memory from the heap.

## See also

The ->`YOMM2_STATIC` macro is a convenient way of creating static registration
objects with an obfuscated name.
