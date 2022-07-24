<!-- target: static-object -->
// <sub>/ [home](/README.md) / [reference](README.md) </sub>

## YOMM2 static objects

YOMM2 uses instances of various classes to register user classes, methods, and
method definitions. The constructors adds the object to an intrusive forward
list, and relies on underlying plain pointers to be zero-initialized. Thus,
these objects must be defined at file scope, or as as `static` variables inside
a function or a class.

The static objects constructor do not allocate memory from the heap.
