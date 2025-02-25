entry: generator
headers: yorel/yomm2/generator.hpp

```c++
class generator;
```

This mechanism generates C++ source files that can be included in a project to
speed up method dispatch, and to make dispatch data construction significantly
less resource intensive.

Like virtual functions, methods occupy slots in v-tables associated to classes.
Unlike virtual functions, the slots cannot be determined by looking at a single
translation unit; the entire program has to be examined before the slots
are known. By default, method dispatch reads the slots from variables set by
->`update`. The additional reads put open methods at a disadvantage, compared to
ordinary virtual functions.

`write_static_offsets` generates C++ code that enables method dispatch to use
"static" slots - i.e. slots known at compile time. Static slots should be made
visible (typically by means including the generated code) before these methods
are called. A program may contain a mixture of methods that use static slots,
and methods that do not. However, this should be consistent across translation
units; failing to ensure this is a ODR violation.

Using static slots shaves off 2*N-1 memory reads from a method call, where N is
the number of virtual parameters in a method. A 1-method call via a
->`virtual_ptr`, using static offsets, takes 2 instructions on a x64 CPU, the
same as a virtual function call, but one fewer memory read. See the example for
assembly listings.

The code generated by `write_static_offsets` requires that types used by the
method (parameter types, return type and method key) to be known. This is easy
when using static slots for specific methods - by including the generated static
offsets for the method  just before the method declaration; it is more
challenging when using static offsets for an entire program.
`write_forward_declarations` attempts to generate suitable forward declarations,
but it has limitations. See its documentation.

`encode_dispatch_data` initializes the dispatch tables for a policy, using a
compact representation of the data produced by ->`update`. It merely copies
integers and it does not allocate memory from the heap.

## Member functions

| Name                                                      | Description                                         |
| --------------------------------------------------------- | --------------------------------------------------- |
| [write_static_offsets](#write_static_offsets)             | write static slots for a method or a policy         |
| [add_forward_declaration](#add_forward_declaration)       | register types for forward declaration generation   |
| [write_forward_declarations](#write_forward_declarations) | write forward declarations for the registered types |
| [encode_dispatch_data](#write_forward_declarations)       | write data and code to initialize dispatch tables   |

## write_static_offsets

```c++
template<class Method> void write_static_offsets(std::ostream& os) const; (1)
template<class Policy> void write_static_offsets(std::ostream& os) const; (2)
```
Add the method to the policy's method list.

1) Write static slots for a single method to `os`.
2) Write static slots for all the methods in a policy to  `os`.

## add_forward_declaration

```c++
void add_forward_declaration(std::string_view decl);         (1)
void add_forward_declaration(const std::type_info& type);    (2)
template<typename T> void add_forward_declaration();         (3)
template<class Policy> void add_forward_declarations();      (4)
```
1) Add `decl` to the list of declarations.
2) Add a declaration for `type` to the list of declarations.
3) Equivalent to `add_forward_declaration(typeid(T))`.
4) Add declarations for the return, parameter and key types used by all the
   methods in `Policy`.

(2), (3) and (4) use `boost::demangle` to extract type names. The result is not
guaranteed, as it depends on the availability and the output of a ABI specific
demangling mechanism. Note that no attempt is made at extracting templates,
because it is impossible to guess the tewmplate parameter list.

## write_forward_declarations

```c++
void write_forward_declarations(std::ostream& os) const;
```

Write forward declarations for all the types extracted by
`add_forward_declaration(s)` to `os`.

## encode_dispatch_data

```c++
template<class Compiler>
static void encode_dispatch_data(
   const Compiler& compiler, std::ostream& os);                            (1)
template<class Compiler>
static void encode_dispatch_data(
   const Compiler& compiler, const std::string& policy, std::ostream& os); (2)
```

Write code to initialize the dispatch data for a policy to `os`.
`compiler` is the object returned by the ->`update` function.

(1) targets the default policy. (2) targets the specified policy.

Ther generated code consists of a data structure, named `yomm2_dispatch_data`,
and a function call. It is suitable for inclusionh in a function body - e.g.
`main`. It is assumed that `<yorel/yomm2/generator.hpp>` has been included, and
that the policy is visible.

## Example

See the
[generator](https://github.com/jll63/yomm2/tree/master/examples/generator)
example.
