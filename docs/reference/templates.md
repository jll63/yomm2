
**yorel::yomm2::templates** <small>(experimental)</small><br>
<sub>defined in <yorel/yomm2/core.hpp></sub>
<!-- -->```
template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;
```
<!-- -[`templates`](/yomm2/reference/templates.html) wraps a sequence of templates in a [types](/yomm2/reference/types.html) list of [template_](/yomm2/reference/template_.html)
wrappers.
