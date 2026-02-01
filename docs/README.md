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
  method container; see [`method_container`](/yomm2/reference/method_container.html).

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
global, but so are keywords. The [virtual_](/yomm2/reference/virtual_.html) template, used to specify
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
function, which can be set with [set_error_handler](/yomm2/reference/set_error_handler.html). A handler may throw
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

* [`method`](/yomm2/reference/method.html), a class template that contains:
  * a static function object [`fn`](/yomm2/reference/method.html), to call the method
  * nested class templates [`add_function`](/yomm2/reference/method.html) and [`add_definition`](/yomm2/reference/method.html), to add
    definitions to a method
  * [`next_type`](/yomm2/reference/method.html) and [`use_next`](/yomm2/reference/method.html), to call the next most
    specialised method
* [`use_classes`](/yomm2/reference/use_classes.html), a class template, provides the class and inheritance information.
* [`update`](/yomm2/reference/update.html), a function that calculates the method dispatch tables, using the
  method, definition, and class information.

The header itself does not define any macros, except for its include guard
(`YOREL_YOMM2_CORE_INCLUDED`).

The header consumes three macros:
* `NDEBUG`: if defined, no checks are performed during method calls. This
  delivers a performance close to normal virtual function calls.
* `YOMM2_SHARED`: if defined, the library runtime is in a shared library or DLL.
* [`YOMM2_DEFAULT_POLICY`](/yomm2/reference/policy-basic_policy.html): if defined, overrides the default policy.

The header defines the following macros:
* an include guard (`YOREL_YOMM2_CORE_INCLUDED`).
* *iff* `YOMM2_SHARED` is defined, a `yOMM2_API` macro, for internal use.

### `<yorel/yomm2/policy.hpp>`

Contains the policy namespace, and the associated mechanisms. It is included by
`<yorel/yomm2/core.hpp>`. It can also be included directly to create a new
policy, to be used as the default policy, before including the core header. See
[`YOMM2_DEFAULT_POLICY`](/yomm2/reference/policy-basic_policy.html) for more details.

### `<yorel/yomm2/symbols.hpp>`

This header defines two macros: `YOMM2_GENSYM`, which generates a new obfuscated
symbol each time that is is expanded; and `YOMM2_SYMBOL(seed)`, which generates
an obfuscated symbol (the same symbol for the same value of `seed`).

These macros are useful when using the core interface, which requires
instantiating static objects to register classes, methods, and definitions; and
for defining the "key" type for the [method](/yomm2/reference/method.html) template.

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
| [class_declaration](/yomm2/reference/class_declaration.html)              | class template    | declare a class and its bases                                            |
| [declare_method](/yomm2/reference/declare_method.html)                 | macro             | declare a method                                                         |
| [declare_static_method](/yomm2/reference/declare_static_method.html)          | macro             | declare a static method inside a class                                   |
| [default_policy](/yomm2/reference/policy-basic_policy.html)                 | typedef           | `debug` or `release`, depending on `NDEBUG`                              |
| [define_method](/yomm2/reference/define_method.html)                  | macro             | add a definition to a method                                             |
| [define_method_inline](/yomm2/reference/define_method_inline.html)           | macro             | add an definition to a method in a container, and make it inline         |
| [error](/yomm2/reference/error.html)                          | class             | base class of error subclasses                                           |
| [error_handler_type](/yomm2/reference/error.html)             | type              | handler function                                                         |
| [error_type](/yomm2/reference/error.html)                     | variant           | object passed to error handler                                           |
| [friend_method](/yomm2/reference/friend_method.html)                  | macro             | make a method in a container, or the entire container, a friend          |
| [generator](/yomm2/reference/generator.html)                      | class             | generate compile-time offsets, pre-calculate dispatch data               |
| [hash_search_error](/yomm2/reference/error.html)              | class             | failure to find a hash function for registered classes                   |
| [make_virtual_shared](/yomm2/reference/virtual_ptr.html)            | function template | create an object and return a `virtual_shared_ptr`                       |
| [method](/yomm2/reference/method.html)                         | class template    | implement a method                                                       |
| [method_call_error](/yomm2/reference/method_call_error.html)              | class             | information about a failed method call                                   |
| [method_call_error_handler](/yomm2/reference/method_call_error.html)      | type              | type of a function called when a method call fails                       |
| [method_class](/yomm2/reference/method_class.html)                   | macro             | get `method` class from method signature                                 |
| [method_container](/yomm2/reference/method_container.html)               | macro             | declare a method definition container                                    |
| [method_definition](/yomm2/reference/method_definition.html)              | macro             | retrieve a definition from a container                                   |
| [method_table_error](/yomm2/reference/error.html)             | class             | `virtual_ptr` static type differs from dynamic type                      |
| [policy](/yomm2/reference/policy-basic_policy.html)                         | namespace         | contains policy and facet related mechanisms                             |
| [basic_error_output](/yomm2/reference/policy-basic_error_output.html)      | class template    | generic implementation of `error_output`                                 |
| [basic_policy](/yomm2/reference/policy-basic_policy.html)            | class template    | create a policy                                                          |
| [basic_trace_output](/yomm2/reference/policy-basic_trace_output.html)      | class template    | generic implementation of `trace_output`                                 |
| [checked_perfect_hash](/yomm2/reference/policy-checked_perfect_hash.html)    | class template    | implementation of type_hash using a perfect hash, with runtime checks    |
| [debug](/yomm2/reference/policy-basic_policy.html)                   | class             | most versatile policy, with runtime checks                               |
| [deferred_static_rtti](/yomm2/reference/policy-rtti.html)    | class             | facet sub-category: do not collect type ids at static contstruction time |
| [error_handler](/yomm2/reference/policy-error_handler.html)           | class             | facet responsible for handling errors                                    |
| [error_output](/yomm2/reference/policy-error_output.html)            | class             | facet responsible for printing errors                                    |
| [external_vptr](/yomm2/reference/policy-vptr_placement.html)           | class             | sub-category of `vptr_placement`; vptrs are stored out of objects        |
| [fast_perfect_hash](/yomm2/reference/policy-fast_perfect_hash.html)       | class template    | implementation of type_hash using a fast, perfect hash                   |
| [minimal_rtti](/yomm2/reference/policy-minimal_rtti.html)            | class             | implementation of `rtti` that des not use RTTI                           |
| [release](/yomm2/reference/policy-basic_policy.html)                 | class             | fastest and most versatile policy, no runtime checks                     |
| [rtti](/yomm2/reference/policy-rtti.html)                    | class             | facet responsible fro RTTI                                               |
| [std_rtti](/yomm2/reference/policy-std_rtti.html)                | class             | implement `rtti` facet using standard RTTI                               |
| [throw_error](/yomm2/reference/policy-throw_error.html)             | class             | handle errors by throwing exceptions                                     |
| [trace_output](/yomm2/reference/policy-trace_output.html)            | class template    | facet responsible for tracing internal operations                        |
| [type_hash](/yomm2/reference/policy-type_hash.html)               | class             | facet responsible for hashing type ids                                   |
| [vectored_error](/yomm2/reference/policy-vectored_error.html)          | class template    | handle errors by calling a `std::function`                               |
| [vptr_map](/yomm2/reference/policy-vptr_map.html)                | class template    | implement facet `vptr_placement` using a `std::unordered_map`            |
| [vptr_placement](/yomm2/reference/policy-vptr_placement.html)          | class             | facet responsible for finding the vptr for an object                     |
| [vptr_vector](/yomm2/reference/policy-vptr_vector.html)             | class template    | implement facet `vptr_placement` using a `std::vector`                   |
| [register_class](/yomm2/reference/register_class.html)                 | macro             | register a class and its bases (deprecated)                              |
| [register_classes](/yomm2/reference/use_classes.html)               | macro             | register classes and their inheritance relationships                     |
| [resolution_error](/yomm2/reference/error.html)               | class             | method call does not resolve to exactly one definition                   |
| [RestrictedOutputStream](/yomm2/reference/RestrictedOutputStream.html)         | concept           | `std::ostream`-like class with just a few operations                     |
| [set_error_handler](/yomm2/reference/set_error_handler.html)              | function          | set the function called for all errors                                   |
| [set_method_call_error_handler](/yomm2/reference/method_call_error.html)  | function          | set function to call when a method call fails                            |
| [type_id](/yomm2/reference/type_id.html)                        | typedef           | alias to `std::uintptr_t`, used for storing dispatch data                |
| [unknown_class_error](/yomm2/reference/error.html)            | class             | class used in method declaration, definition, or call was not registered |
| [update](/yomm2/reference/update.html)                         | function          | set up dispatch tables                                                   |
| [update_methods](/yomm2/reference/update_methods.html)                 | function          | set up dispatch tables (deprecated, requires linking with library)       |
| [use_classes](/yomm2/reference/use_classes.html)                    | class template    | register classes and their inheritance relationships                     |
| [virtual_](/yomm2/reference/virtual_.html)                       | class template    | mark a method parameter as virtual                                       |
| [virtual_ptr](/yomm2/reference/virtual_ptr.html)                    | class template    | fat pointer for optimal method dispatch                                  |
| [virtual_shared_ptr](/yomm2/reference/virtual_ptr.html)             | class template    | `virtual_ptr` using a `std::shared_ptr`                                  |
| [YOMM2_CLASS](/yomm2/reference/register_class.html)                    | macro             | same as `register_class` (deprecated)                                    |
| [YOMM2_CLASSES](/yomm2/reference/use_classes.html)                  | macro             | same as `register_classes`                                               |
| [YOMM2_DECLARE](/yomm2/reference/declare_method.html)                  | macro             | same as `declare_method`                                                 |
| [YOMM2_DECLARE_METHOD_CONTAINER](/yomm2/reference/method_container.html) | macro             | same as `method_container`                                               |
| [YOMM2_DEFINE](/yomm2/reference/define_method.html)                   | macro             | same as `define_method`                                                  |
| [YOMM2_DEFAULT_POLICY](/yomm2/reference/policy-basic_policy.html)           | macro             | global default policy override                                           |
| [YOMM2_DEFINE_INLINE](/yomm2/reference/define_method_inline.html)            | macro             | same as `define_method_inline`                                           |
| [YOMM2_DEFINITION](/yomm2/reference/method_definition.html)               | macro             | same as `method_definition`                                              |
| [YOMM2_FRIEND](/yomm2/reference/friend_method.html)                   | macro             | same as `friend_method`                                                  |
| [YOMM2_GENSYM](/yomm2/reference/YOMM2_GENSYM.html)                   | macro             | generate a unique symbol                                                 |
| [YOMM2_METHOD_CLASS](/yomm2/reference/method_class.html)             | macro             | get `method` class from method signature                                 |
| [YOMM2_STATIC](/yomm2/reference/YOMM2_STATIC.html)                   | macro             | instantiate an anonymous static object                                   |
| [YOMM2_STATIC_DECLARE](/yomm2/reference/declare_static_method.html)           | macro             | declare a static method inside a class                                   |
| [YOMM2_SYMBOL](/yomm2/reference/YOMM2_SYMBOL.html)                   | macro             | generate an obfuscated symbol                                            |

### *Experimental* template helpers

| name              | type           | purpose                                                          |
| ----------------- | -------------- | ---------------------------------------------------------------- |
| [aggregate](/yomm2/reference/aggregate.html)       | class template | make a type by aggregating a set of types                        |
| [apply_product](/yomm2/reference/apply_product.html)   | meta-function  | apply templates to the n-fold Cartesian product of `types` lists |
| [not_defined](/yomm2/reference/not_defined.html)     | meta-function  | tell [use_definitions](/yomm2/reference/use_definitions.html) to discard a definition                   |
| [product](/yomm2/reference/product.html)         | meta-function  | form n-fold Cartesian product of `types` lists                   |
| [template_](/yomm2/reference/template_.html)       | class template | wrap a template in a type                                        |
| [templates](/yomm2/reference/templates.html)       | class template | wrap templates in a `types` list                                 |
| [types](/yomm2/reference/types.html)           | class template | sequence of types                                                |
| [use_definitions](/yomm2/reference/use_definitions.html) | class template | add batch of definitions from a generic container to methods     |
