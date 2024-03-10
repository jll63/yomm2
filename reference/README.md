<sub>trailexxxr</sub><br>
<!-- target:reference -->
<sub>/ [home](/README.md) </sub>
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
# Reference
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Introduction
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
YOMM2 is a library that implements open multi-methods for C++17. The semantics
<sub>trailexxxr</sub><br>
of methods are strongly influenced by the [Open Multi-Methods for
<sub>trailexxxr</sub><br>
C++](https://www.stroustrup.com/multimethods.pdf) paper by Peter Pirkelbauer,
<sub>trailexxxr</sub><br>
Yuriy Solodkyy, and Bjarne Stroustrup.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This implementation diverges from the paper on the following points:
<sub>trailexxxr</sub><br>
* A "base-method" is called a "method declaration" in YOMM2, and an "overrider"
<sub>trailexxxr</sub><br>
  is called a "method definition". This is because
<sub>trailexxxr</sub><br>
* YOMM2 has a mechanism (`next`) to call the next most specialised method.
<sub>trailexxxr</sub><br>
* The paper allows only references for virtual parameters. YOMM2 also allows
<sub>trailexxxr</sub><br>
  pointers and smart pointers.
<sub>trailexxxr</sub><br>
* YOMM2 does not support repeated inheritance. Multiple and virtual inheritance
<sub>trailexxxr</sub><br>
  are supported.
<sub>trailexxxr</sub><br>
* Method definitions are hidden. It is possible to access definitions while
<sub>trailexxxr</sub><br>
  bypassing method dispatch, but this requires the definitions to be placed in a
<sub>trailexxxr</sub><br>
  method container; see [the documentation](method_container.md) of
<sub>trailexxxr</sub><br>
  `method_container`.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Concepts
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
A *method* is a function that has one or more virtual parameters, and a
<sub>trailexxxr</sub><br>
collection of method definitions.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
A *method definition* is a function attached to a single method. The type of the
<sub>trailexxxr</sub><br>
parameters corresponding to the virtual parameters in the method must be
<sub>trailexxxr</sub><br>
compatible with the method's parameter type. The other parameters must have the
<sub>trailexxxr</sub><br>
same type as in the method. The return type of the definition must be compatible
<sub>trailexxxr</sub><br>
with the return type of the method.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
A *virtual parameter* is a parameter that is taken into account when the method
<sub>trailexxxr</sub><br>
is called.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
A *virtual argument* is an argument in a method call that corresponds to a
<sub>trailexxxr</sub><br>
virtual parameter in the method declaration. The method uses the *dynamic* type
<sub>trailexxxr</sub><br>
of the virtual arguments to select which definition to execute. The rules for
<sub>trailexxxr</sub><br>
selecting the definition are the same as for overload resolution: use the most
<sub>trailexxxr</sub><br>
specialised definition, from the set of applicable definitions. However, note
<sub>trailexxxr</sub><br>
that the selection happens at runtime.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## API Overview
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The library is normally used via the _keyword interface_, provided by the
<sub>trailexxxr</sub><br>
`<yorel/yomm2/keywords.hpp>` header. It attempts to present open methods as a
<sub>trailexxxr</sub><br>
language feature. It consists of a collection of macros, which are, of course,
<sub>trailexxxr</sub><br>
global, but so are keywords. The [virtual_](/reference/virtual_.md) template, used to specify
<sub>trailexxxr</sub><br>
virtual parameters, is also aliases in the global namespace.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The library can also be used through the _core interface_, which is almost
<sub>trailexxxr</sub><br>
entirely free of macros. The primary use of this interface is to support
<sub>trailexxxr</sub><br>
templatized classes, methods and definitions - something that macros are
<sub>trailexxxr</sub><br>
incapable of. See [the templates tutorial](tutorials/templates_tutorial.md) for
<sub>trailexxxr</sub><br>
more details and examples.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Exceptions
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
YOMM2 is exception agnostic. The library does not throw nor catches exceptions,
<sub>trailexxxr</sub><br>
but it is exception safe. Errors are reported via an indirect call to a handler
<sub>trailexxxr</sub><br>
function, which can be set with [set_error_handler](/reference/set_error_handler.md). A handler may throw
<sub>trailexxxr</sub><br>
exceptions.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Headers
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/keywords.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header provides the _keyword interface_. Since version 1.3.0, this is the
<sub>trailexxxr</sub><br>
recommended main header for normal usage of the library. It is used in most
<sub>trailexxxr</sub><br>
examples and tutorials. The header makes it look like the library's features are
<sub>trailexxxr</sub><br>
part of the language:
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
* It includes `<yorel/yomm2/core.hpp>`.
<sub>trailexxxr</sub><br>
* It includes `<yorel/yomm2/cute.hpp>`, thus making the lowercase macros
<sub>trailexxxr</sub><br>
  (`declare_method`, etc) available.
<sub>trailexxxr</sub><br>
* It contains a `using ::yorel::yomm2::virtual_` directive.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/core.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header provides the _core interface_, in the `yorel::yomm2` namespace.
<sub>trailexxxr</sub><br>
Since version 1.3.0, the key mechanisms are documented; thus, it possible to use
<sub>trailexxxr</sub><br>
the library without resorting on the macros. See the [API
<sub>trailexxxr</sub><br>
tutorial](../tutorials/api.md) for an introduction to the main features of
<sub>trailexxxr</sub><br>
`core`.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The main constructs are:
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
* [method](/reference/method.md), a class template that contains:
<sub>trailexxxr</sub><br>
  * a static function object [fn](method.md#fn), to call the method
<sub>trailexxxr</sub><br>
  * nested class templates [add_function](method.md#add_function) and
<sub>trailexxxr</sub><br>
    [add_definition](method.md#add_definition), to add definitions to a method
<sub>trailexxxr</sub><br>
  * [next_type](method.md#next_type) and [use_next](method.md#use_next), to call
<sub>trailexxxr</sub><br>
    the next most specialised method
<sub>trailexxxr</sub><br>
* [use_classes](/reference/use_classes.md), a class template, provides the class and inheritance information.
<sub>trailexxxr</sub><br>
* [update_methods](/reference/update_methods.md), a function that calculates the method dispatch tables, using
<sub>trailexxxr</sub><br>
  the method, definition, and class information.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The header itself does not define any macros, except for its include guard
<sub>trailexxxr</sub><br>
(`YOREL_YOMM2_CORE_INCLUDED`).
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The header consumes two macros:
<sub>trailexxxr</sub><br>
* `NDEBUG`: if defined, no checks are performed during method calls. This
<sub>trailexxxr</sub><br>
  delivers a performance close to normal virtual function calls.
<sub>trailexxxr</sub><br>
* `YOMM2_ENABLE_TRACE`: enables tracing. This feature is not documented at the
<sub>trailexxxr</sub><br>
  moment.
<sub>trailexxxr</sub><br>
* `YOMM2_SHARED`: if defined, the library runtime is in a shared library or DLL.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
The header defines the following macros:
<sub>trailexxxr</sub><br>
* an include guard (`YOREL_YOMM2_CORE_INCLUDED`).
<sub>trailexxxr</sub><br>
* *iff* `YOMM2_SHARED` is defined, a `yOMM2_API` macro, for internal use.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/symbols.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header defines two macros: `YOMM2_GENSYM`, which generates a new obfuscated
<sub>trailexxxr</sub><br>
symbol each time that is is expanded; and `YOMM2_SYMBOL(seed)`, which generates
<sub>trailexxxr</sub><br>
an obfuscated symbol (the same symbol for the same value of `seed`).
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
These macros are useful when using the core interface, which requires
<sub>trailexxxr</sub><br>
instantiating static objects to register classes, methods, and definitions; and
<sub>trailexxxr</sub><br>
for defining the "key" type for the [method](/reference/method.md) template.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/templates.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header defines *experimental* meta-programming constructs intended to
<sub>trailexxxr</sub><br>
facilitate the creation of templatized method and methods definitions. See the
<sub>trailexxxr</sub><br>
[template tutorial](../tutorials/templates_tutorial.md) for examples.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/macros.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header defines the upper-case versions of the macros (`YOMM2_DECLARE` etc).
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2/cute.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This header defines the lower-case versions of the macros (`declare_method`
<sub>trailexxxr</sub><br>
etc).
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### `<yorel/yomm2.hpp>`
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
This was the recommended header before version 1.3.0. Includes
<sub>trailexxxr</sub><br>
`<yorel/yomm2/core.hpp>` and `<yorel/yomm2/macros.hpp>`.
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Libraries
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
40488
<sub>trailexxxr</sub><br>
99424
<sub>trailexxxr</sub><br>
~58K
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
## Index
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
| name                             | type              | purpose                                                                  |
<sub>trailexxxr</sub><br>
| -------------------------------- | ----------------- | ------------------------------------------------------------------------ |
<sub>trailexxxr</sub><br>
| [class_declaration](/reference/class_declaration.md)              | class template    | declares a class and its bases                                           |
<sub>trailexxxr</sub><br>
| [declare_method](/reference/declare_method.md)                 | macro             | declares a method                                                        |
<sub>trailexxxr</sub><br>
| [declare_static_method](/reference/declare_static_method.md)          | macro             | declares a static method inside a class                                  |
<sub>trailexxxr</sub><br>
| [define_method](/reference/define_method.md)                  | macro             | adds a definition to a method                                            |
<sub>trailexxxr</sub><br>
| [define_method_inline](/reference/define_method_inline.md)           | macro             | adds an definition to a method in a container, and make it inline        |
<sub>trailexxxr</sub><br>
| [error_handler_type](/reference/set_error_handler.md)             | type              | handler function                                                         |
<sub>trailexxxr</sub><br>
| [error_type](/reference/set_error_handler.md)                     | variant           | object passed to error handler                                           |
<sub>trailexxxr</sub><br>
| [friend_method](/reference/friend_method.md)                  | macro             | makes a method in a container, or the entire container, a friend         |
<sub>trailexxxr</sub><br>
| [hash_search_error](/reference/set_error_handler.md)              | class             | failure to find a hash function for registered classes                   |
<sub>trailexxxr</sub><br>
| [make_virtual_shared](/reference/virtual_ptr.md)            | function template | creates an object and return a `virtual_shared_ptr`                      |
<sub>trailexxxr</sub><br>
| [method](/reference/method.md)                         | class template    | implements a method                                                      |
<sub>trailexxxr</sub><br>
| [method_call_error](/reference/method_call_error.md)              | class             | information about a failed method call                                   |
<sub>trailexxxr</sub><br>
| [method_call_error_handler](/reference/method_call_error.md)      | type              | type of a function called when a method call fails                       |
<sub>trailexxxr</sub><br>
| [method_container](/reference/method_container.md)               | macro             | declares a method definition container                                   |
<sub>trailexxxr</sub><br>
| [method_definition](/reference/method_definition.md)              | macro             | retrieves a definition from a container                                  |
<sub>trailexxxr</sub><br>
| [register_class](/reference/register_class.md)                 | macro             | registers a class and its bases (deprecated)                             |
<sub>trailexxxr</sub><br>
| [register_classes](/reference/use_classes.md)               | macro             | registers classes and their inheritance relationships                    |
<sub>trailexxxr</sub><br>
| [resolution_error](/reference/set_error_handler.md)               | class             | method call does not resolve to exactly one definition                   |
<sub>trailexxxr</sub><br>
| [set_error_handler](/reference/set_error_handler.md)              | function          | sets the function called for all errors                                  |
<sub>trailexxxr</sub><br>
| [set_method_call_error_handler](/reference/method_call_error.md)  | function          | sets function to call when a method call fails                           |
<sub>trailexxxr</sub><br>
| [unknown_class_error](/reference/set_error_handler.md)            | class             | class used in method declaration, definition, or call was not registered |
<sub>trailexxxr</sub><br>
| [update](/reference/update.md)                         | function          | sets up dispatch tables                                                  |
<sub>trailexxxr</sub><br>
| [update_methods](/reference/update_methods.md)                 | function          | sets up dispatch tables (deprecated, requires linking with library)      |
<sub>trailexxxr</sub><br>
| [use_classes](/reference/use_classes.md)                    | class template    | registers classes and their inheritance relationships                    |
<sub>trailexxxr</sub><br>
| [virtual_](/reference/virtual_.md)                       | class template    | marks a method parameter as virtual                                      |
<sub>trailexxxr</sub><br>
| [virtual_ptr](/reference/virtual_ptr.md)                    | class template    | fat pointer for optimal method dispatch                                  |
<sub>trailexxxr</sub><br>
| [virtual_shared_ptr](/reference/virtual_ptr.md)             | class template    | `virtual_ptr` using a `std::shared_ptr`                                  |
<sub>trailexxxr</sub><br>
| [YOMM2_CLASS](/reference/register_class.md)                    | macro             | same as `register_class` (deprecated)                                    |
<sub>trailexxxr</sub><br>
| [YOMM2_CLASSES](/reference/use_classes.md)                  | macro             | same as `register_classes`                                               |
<sub>trailexxxr</sub><br>
| [YOMM2_DECLARE](/reference/declare_method.md)                  | macro             | same as `declare_method`                                                 |
<sub>trailexxxr</sub><br>
| [YOMM2_DECLARE_METHOD_CONTAINER](/reference/method_container.md) | macro             | same as `method_container`                                               |
<sub>trailexxxr</sub><br>
| [YOMM2_DEFINE](/reference/define_method.md)                   | macro             | same as `define_method`                                                  |
<sub>trailexxxr</sub><br>
| [YOMM2_DEFINE_INLINE](/reference/define_method_inline.md)            | macro             | same as `define_method_inline`                                           |
<sub>trailexxxr</sub><br>
| [YOMM2_DEFINITION](/reference/method_definition.md)               | macro             | same as `method_definition`                                              |
<sub>trailexxxr</sub><br>
| [YOMM2_FRIEND](/reference/friend_method.md)                   | macro             | same as `friend_method`                                                  |
<sub>trailexxxr</sub><br>
| [YOMM2_GENSYM](/reference/YOMM2_GENSYM.md)                   | macro             | generates a unique symbol                                                |
<sub>trailexxxr</sub><br>
| [YOMM2_STATIC_DECLARE](/reference/declare_static_method.md)           | macro             | declares a static method inside a class                                  |
<sub>trailexxxr</sub><br>
| [YOMM2_SYMBOL](/reference/YOMM2_SYMBOL.md)                   | macro             | generates an obfuscated symbol                                           |
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
### *Experimental* template helpers
<sub>trailexxxr</sub><br>

<sub>trailexxxr</sub><br>
| name              | type           | purpose                                                          |
<sub>trailexxxr</sub><br>
| ----------------- | -------------- | ---------------------------------------------------------------- |
<sub>trailexxxr</sub><br>
| [apply_product](/reference/apply_product.md)   | meta-function  | apply templates to the n-fold Cartesian product of `types` lists |
<sub>trailexxxr</sub><br>
| [not_defined](/reference/not_defined.md)     | meta-function  | tell [use_definitions](/reference/use_definitions.md) to discard a definition                   |
<sub>trailexxxr</sub><br>
| [product](/reference/product.md)         | meta-function  | form n-fold Cartesian product of `types` lists                   |
<sub>trailexxxr</sub><br>
| [template_](/reference/template_.md)       | class template | wrap a template in a type                                        |
<sub>trailexxxr</sub><br>
| [templates](/reference/templates.md)       | class template | wrap templates in a `types` list                                 |
<sub>trailexxxr</sub><br>
| [types](/reference/types.md)           | class template | sequence of types                                                |
<sub>trailexxxr</sub><br>
| [use_definitions](/reference/use_definitions.md) | class template | add batch of definitions from a generic container to methods     |
