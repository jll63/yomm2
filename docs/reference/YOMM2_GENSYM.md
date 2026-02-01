> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;"><strong>YOMM2_GENSYM</strong><br/></span><br/>
<sub>defined in <yorel/yomm2/symbols.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

`YOMM2_GENSYM` expands to a new C++ identifier each time it is called. The
symbol is based on the `__COUNTER__` preprocessor macro, and decorated in such a
way that it is unlikely to clash with user-defined symbols.

`YOMM2_GENSYM` provides a convenient way of allocating [static
objects](static_object.md) for registering classes, methods, or definitions.

### Example

```c++
int YOMM2_GENSYM;
int YOMM2_GENSYM; // not a redefinition, this is a new symbol
```
