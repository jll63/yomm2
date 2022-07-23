<!-- target: YOMM2_DEFINE_INLINE -->
<sub>/ [home](/README.md) / [reference](README.md) </sub>
## define_method_inline
<sub>defined in <yorel/yomm2/cute.hpp>, also provided by <yorel/yomm2/keywords.hpp>

---
```
#define define_method_inline(container, return-type, name, (function-parameter-list)) /*unspecified*/
```
---
Add an definition to a method, inside a container, and make it inline.

Like [define_method](define_method.md), but the definition has the `inline` storage class, and
thus can be placed in a header file and is a potential candidate for inlining.

Note that inlining is only supported for definitions placed inside a container.
Inlining implementations defined outside of a container would make no sense, as
there would be no way of referencing them.

See the documentation of [method_container](method_container.md) for more information on method
containers.

See the documentation of [declare_method](declare_method.md) for information on handling types that
contain commas.

## synonym
YOMM2_METHOD_INLINE, defined in <yorel/yomm2/macros.hpp>, also provided by <yorel/yomm2.hpp>.
