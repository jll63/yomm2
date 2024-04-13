**yorel::yomm2::templates** <small>(experimental)</small><br>
<sub>defined in <yorel/yomm2/core.hpp></sub><br/>

```c++
template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;
```
`templates` wraps a sequence of templates in a [types](/yomm2/reference/types.html) list of [template_](/yomm2/reference/template_.html)
wrappers.
