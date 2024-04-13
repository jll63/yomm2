macro: define_method_inline
headers: yorel/yomm2/cute.hpp, yorel/yomm2/keywords.hpp
hrefs: YOMM2_DEFINE_INLINE

```
#define define_method_inline(container, return-type, name, (function-parameter-list)) /*unspecified*/
```

Add an definition to a method, inside a container, and make it inline.

Like ->define_method, but the definition has the `inline` storage class, and
thus can be placed in a header file and is a potential candidate for inlining.

Note that inlining is only supported for definitions placed inside a container.
Inlining implementations defined outside of a container would make no sense, as
there would be no way of referencing them.

See the documentation of ->method_container for more information on method
containers.

See the documentation of ->declare_method for information on handling types that
contain commas.

## synonym
YOMM2_METHOD_INLINE, defined in <yorel/yomm2/macros.hpp>, also provided by <yorel/yomm2.hpp>.
