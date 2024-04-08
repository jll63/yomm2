yorel::yomm2::**define_method_inline**

<sub>defined in <yorel/yomm2/cute.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>
```
#define define_method_inline(container, return-type, name, (function-parameter-list)) /*unspecified*/
```Add an definition to a method, inside a container, and make it inline.

Like [define_method](/yomm2/reference/define_method.html), but the definition has the `inline` storage class, and
thus can be placed in a header file and is a potential candidate for inlining.

Note that inlining is only supported for definitions placed inside a container.
Inlining implementations defined outside of a container would make no sense, as
there would be no way of referencing them.

See the documentation of [method_container](/yomm2/reference/method_container.html) for more information on method
containers.

See the documentation of [declare_method](/yomm2/reference/declare_method.html) for information on handling types that
contain commas.

## synonym
YOMM2_METHOD_INLINE, defined in <yorel/yomm2/macros.hpp>, also provided by <yorel/yomm2.hpp>.
