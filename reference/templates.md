<sub>/ [home](/README.md) / [reference](/reference/README.md) </sub>

**yorel::yomm2::templates** <small>(experimental)</small><br>
<sub>defined in <yorel/yomm2/core.hpp></sub>
<!-- -->
---
```
template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;
```
<!-- -->
---
`templates` wraps a sequence of templates in a [types](types.md) list of [template_](template_.md)
wrappers.
