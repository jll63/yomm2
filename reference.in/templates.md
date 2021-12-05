<sub>/ ->home / ->reference </sub>

## yorel::yomm2::templates <small>(experimental)</small>
<sub>defined in <yorel/yomm2/core.hpp></sub>
<!-- -->
---
```
template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;
```
<!-- -->
---
`templates` wraps a sequence of templates in a ->types list of ->template_
wrappers.
