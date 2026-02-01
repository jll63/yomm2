> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

## Introduction

YOMM2 is a library that implements open multi-methods for C++17. The semantics
of methods are strongly influenced by the [Open Multi-Methods for
C++](https://www.stroustrup.com/multimethods.pdf) paper by Peter Pirkelbauer,
Yuriy Solodkyy, and Bjarne Stroustrup.

This implementation diverges from the paper on the following points:
* A "base-method" is called a "method declaration" in YOMM2, and an "overrider"
  is called a "method definition".
* YOMM2 has a mechanism (`next`) to call the next most specialised method.
* The paper allows only references for virtual parameters. YOMM2 also allows
  pointers and smart pointers.
* YOMM2 does not support repeated inheritance. Multiple and virtual inheritance
  are supported.
* Method definitions are hidden. It is possible to access definitions while
  bypassing method dispatch, but this requires the definitions to be placed in a
  method container; see ->`method_container`.

## Concepts

A *method* is a function that has one or more virtual parameters, and a
collection of method definitions.

A *method definition* is a function attached to a single method. The type of the
parameters corresponding to the virtual parameters in the method must be
compatible with the method's parameter type. The other parameters must have the
same type as in the method. The return type of the definition must be compatible
with the return type of the method.

A *virtual parameter* is a parameter that is taken into account when the method
is called.

A *virtual argument* is an argument in a method call that corresponds to a
virtual parameter in the method declaration. The method uses the *dynamic* type
of the virtual arguments to select which definition to execute. The rules for
selecting the definition are the same as for overload resolution: use the most
specialised definition, from the set of applicable definitions. However, note
that the selection happens at runtime.

## API Overview

The library is normally used via the _keyword interface_, provided by the
`<yorel/yomm2/keywords.hpp>` header. It attempts to present open methods as a
language feature. It consists of a collection of macros, which are, of course,
global, but so are keywords. The ->virtual_ template, used to specify
virtual parameters, is also aliases in the global namespace.

The library can also be used through the _core interface_, which is almost
entirely free of macros. The primary use of this interface is to support
templatized classes, methods and definitions - something that macros are
incapable of. See [the templates
tutorial](/yomm2/tutorials/templates_tutorial.html) for more details and
examples.

## Exceptions

YOMM2 is exception agnostic. The library does not throw nor catches exceptions,
but it is exception safe. Errors are reported via an indirect call to a handler
function, which can be set with ->set_error_handler. A handler may throw
exceptions.

## Headers

### `<yorel/yomm2/keywords.hpp>`

This header provides the _keyword interface_. Since version 1.3.0, this is the
recommended main header for normal usage of the library. It is used in most
examples and tutorials. The header makes it look like the library's features are
part of the language:

* It includes `<yorel/yomm2/core.hpp>`.
* It includes `<yorel/yomm2/cute.hpp>`, thus making the lowercase macros
  (`declare_method`, etc) available.
* It contains a `using ::yorel::yomm2::virtual_` directive.

### `<yorel/yomm2/core.hpp>`

This header provides the _core interface_, in the `yorel::yomm2` namespace.
Since version 1.3.0, the key mechanisms are documented; thus, it possible to use
the library without resorting on the macros. See the [API
tutorial](/yomm2/tutorials/api.html) for an introduction to the main features of
`core`.

The main constructs are:

* ->`method`, a class template that contains:
  * a static function object ->`method-fn`, to call the method
  * nested class templates ->`method-add_function` and ->`method-add_definition`, to add
    definitions to a method
  * ->`method-next_type` and ->`method-use_next`, to call the next most
    specialised method
* ->`use_classes`, a class template, provides the class and inheritance information.
* ->`update`, a function that calculates the method dispatch tables, using the
  method, definition, and class information.

The header itself does not define any macros, except for its include guard
(`YOREL_YOMM2_CORE_INCLUDED`).

The header consumes three macros:
* `NDEBUG`: if defined, no checks are performed during method calls. This
  delivers a performance close to normal virtual function calls.
* `YOMM2_SHARED`: if defined, the library runtime is in a shared library or DLL.
* ->`YOMM2_DEFAULT_POLICY`: if defined, overrides the default policy.

The header defines the following macros:
* an include guard (`YOREL_YOMM2_CORE_INCLUDED`).
* *iff* `YOMM2_SHARED` is defined, a `yOMM2_API` macro, for internal use.

### `<yorel/yomm2/policy.hpp>`

Contains the policy namespace, and the associated mechanisms. It is included by
`<yorel/yomm2/core.hpp>`. It can also be included directly to create a new
policy, to be used as the default policy, before including the core header. See
->`YOMM2_DEFAULT_POLICY` for more details.

### `<yorel/yomm2/symbols.hpp>`

This header defines two macros: `YOMM2_GENSYM`, which generates a new obfuscated
symbol each time that is is expanded; and `YOMM2_SYMBOL(seed)`, which generates
an obfuscated symbol (the same symbol for the same value of `seed`).

These macros are useful when using the core interface, which requires
instantiating static objects to register classes, methods, and definitions; and
for defining the "key" type for the ->method template.

### `<yorel/yomm2/templates.hpp>`

This header defines *experimental* meta-programming constructs intended to
facilitate the creation of templatized method and methods definitions. See the
[template tutorial](/yomm2/tutorials/templates_tutorial.html) for examples.

### `<yorel/yomm2/macros.hpp>`

This header defines the upper-case versions of the macros (`YOMM2_DECLARE` etc).

### `<yorel/yomm2/cute.hpp>`

This header defines the lower-case versions of the macros (`declare_method`
etc).

### `<yorel/yomm2.hpp>`

This was the recommended header before version 1.3.0. Includes
`<yorel/yomm2/core.hpp>` and `<yorel/yomm2/macros.hpp>`.

## Linking

YOMM2 can be used as a header-only library. This is the recommended way. The
runtime adds ~56K (~36K after stripping) when compiled with clang++-16 for the
x86-64 architecture.

YOMM2 can also be installed as a shared library, by setting the cmake variable
`YOMM2_SHARED` to `ON`. The preprocessor symbol `YOMM2_SHARED` must be defined
for the shared runtime to be used.


## Index

| Name                             | Kind              | Purpose                                                                  |
| -------------------------------- | ----------------- | ------------------------------------------------------------------------ |
| ->class_declaration              | class template    | declare a class and its bases                                            |
| ->declare_method                 | macro             | declare a method                                                         |
| ->declare_static_method          | macro             | declare a static method inside a class                                   |
| ->default_policy                 | typedef           | `debug` or `release`, depending on `NDEBUG`                              |
| ->define_method                  | macro             | add a definition to a method                                             |
| ->define_method_inline           | macro             | add an definition to a method in a container, and make it inline         |
| ->error                          | class             | base class of error subclasses                                           |
| ->error_handler_type             | type              | handler function                                                         |
| ->error_type                     | variant           | object passed to error handler                                           |
| ->friend_method                  | macro             | make a method in a container, or the entire container, a friend          |
| ->generator                      | class             | generate compile-time offsets, pre-calculate dispatch data               |
| ->hash_search_error              | class             | failure to find a hash function for registered classes                   |
| ->make_virtual_shared            | function template | create an object and return a `virtual_shared_ptr`                       |
| ->method                         | class template    | implement a method                                                       |
| ->method_call_error              | class             | information about a failed method call                                   |
| ->method_call_error_handler      | type              | type of a function called when a method call fails                       |
| ->method_class                   | macro             | get `method` class from method signature                                 |
| ->method_container               | macro             | declare a method definition container                                    |
| ->method_definition              | macro             | retrieve a definition from a container                                   |
| ->method_table_error             | class             | `virtual_ptr` static type differs from dynamic type                      |
| ->policy                         | namespace         | contains policy and facet related mechanisms                             |
| ->policy-basic_error_output      | class template    | generic implementation of `error_output`                                 |
| ->policy-basic_policy            | class template    | create a policy                                                          |
| ->policy-basic_trace_output      | class template    | generic implementation of `trace_output`                                 |
| ->policy-checked_perfect_hash    | class template    | implementation of type_hash using a perfect hash, with runtime checks    |
| ->policy-debug                   | class             | most versatile policy, with runtime checks                               |
| ->policy-deferred_static_rtti    | class             | facet sub-category: do not collect type ids at static contstruction time |
| ->policy-error_handler           | class             | facet responsible for handling errors                                    |
| ->policy-error_output            | class             | facet responsible for printing errors                                    |
| ->policy-external_vptr           | class             | sub-category of `vptr_placement`; vptrs are stored out of objects        |
| ->policy-fast_perfect_hash       | class template    | implementation of type_hash using a fast, perfect hash                   |
| ->policy-minimal_rtti            | class             | implementation of `rtti` that des not use RTTI                           |
| ->policy-release                 | class             | fastest and most versatile policy, no runtime checks                     |
| ->policy-rtti                    | class             | facet responsible fro RTTI                                               |
| ->policy-std_rtti                | class             | implement `rtti` facet using standard RTTI                               |
| ->policy-throw_error             | class             | handle errors by throwing exceptions                                     |
| ->policy-trace_output            | class template    | facet responsible for tracing internal operations                        |
| ->policy-type_hash               | class             | facet responsible for hashing type ids                                   |
| ->policy-vectored_error          | class template    | handle errors by calling a `std::function`                               |
| ->policy-vptr_map                | class template    | implement facet `vptr_placement` using a `std::unordered_map`            |
| ->policy-vptr_placement          | class             | facet responsible for finding the vptr for an object                     |
| ->policy-vptr_vector             | class template    | implement facet `vptr_placement` using a `std::vector`                   |
| ->register_class                 | macro             | register a class and its bases (deprecated)                              |
| ->register_classes               | macro             | register classes and their inheritance relationships                     |
| ->resolution_error               | class             | method call does not resolve to exactly one definition                   |
| ->RestrictedOutputStream         | concept           | `std::ostream`-like class with just a few operations                     |
| ->set_error_handler              | function          | set the function called for all errors                                   |
| ->set_method_call_error_handler  | function          | set function to call when a method call fails                            |
| ->type_id                        | typedef           | alias to `std::uintptr_t`, used for storing dispatch data                |
| ->unknown_class_error            | class             | class used in method declaration, definition, or call was not registered |
| ->update                         | function          | set up dispatch tables                                                   |
| ->update_methods                 | function          | set up dispatch tables (deprecated, requires linking with library)       |
| ->use_classes                    | class template    | register classes and their inheritance relationships                     |
| ->virtual_                       | class template    | mark a method parameter as virtual                                       |
| ->virtual_ptr                    | class template    | fat pointer for optimal method dispatch                                  |
| ->virtual_shared_ptr             | class template    | `virtual_ptr` using a `std::shared_ptr`                                  |
| ->YOMM2_CLASS                    | macro             | same as `register_class` (deprecated)                                    |
| ->YOMM2_CLASSES                  | macro             | same as `register_classes`                                               |
| ->YOMM2_DECLARE                  | macro             | same as `declare_method`                                                 |
| ->YOMM2_DECLARE_METHOD_CONTAINER | macro             | same as `method_container`                                               |
| ->YOMM2_DEFINE                   | macro             | same as `define_method`                                                  |
| ->YOMM2_DEFAULT_POLICY           | macro             | global default policy override                                           |
| ->YOMM2_DEFINE_INLINE            | macro             | same as `define_method_inline`                                           |
| ->YOMM2_DEFINITION               | macro             | same as `method_definition`                                              |
| ->YOMM2_FRIEND                   | macro             | same as `friend_method`                                                  |
| ->YOMM2_GENSYM                   | macro             | generate a unique symbol                                                 |
| ->YOMM2_METHOD_CLASS             | macro             | get `method` class from method signature                                 |
| ->YOMM2_STATIC                   | macro             | instantiate an anonymous static object                                   |
| ->YOMM2_STATIC_DECLARE           | macro             | declare a static method inside a class                                   |
| ->YOMM2_SYMBOL                   | macro             | generate an obfuscated symbol                                            |

### *Experimental* template helpers

| name              | type           | purpose                                                          |
| ----------------- | -------------- | ---------------------------------------------------------------- |
| ->aggregate       | class template | make a type by aggregating a set of types                        |
| ->apply_product   | meta-function  | apply templates to the n-fold Cartesian product of `types` lists |
| ->not_defined     | meta-function  | tell ->use_definitions to discard a definition                   |
| ->product         | meta-function  | form n-fold Cartesian product of `types` lists                   |
| ->template_       | class template | wrap a template in a type                                        |
| ->templates       | class template | wrap templates in a `types` list                                 |
| ->types           | class template | sequence of types                                                |
| ->use_definitions | class template | add batch of definitions from a generic container to methods     |
