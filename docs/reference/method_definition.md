yorel::yomm2::**method_definition**


<sub>defined in <yorel/yomm2/cute.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>
```
#define method_definition(CONTAINER, RETURN_TYPE, FUNCTION_PARAMETER_LIST)
```Retrieve a method definition with a given return type and signature from a
container.

The resulting method can be used as a normal function reference. It can be
called, and its address can be taken. In particular, this makes it possible for
a method definition to call a base method as part of its implementation, in the
same manner as an ordinary virtual function can call a specific base function
by prefixing its name with a base class name.

Note that the preferred way of calling the overriden method is via `next`. In
normal circumstances, a method definition cannot assume which "super" or "base"
function is the best choice, since the set of methods pertaining to the same
declaration is open.

## synonym
YOMM2_DEFINITION, defined in <yorel/yomm2/macros.hpp>, also provided by <yorel/yomm2.hpp>.

## Example
See the [containers](https://github.com/jll63/yomm2/examples/containers) example.
