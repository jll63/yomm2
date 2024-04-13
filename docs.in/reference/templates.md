experimental: yorel::yomm2::templates
headers: yorel/yomm2/core.hpp

```c++
template<template<typename...> typename... Templates>
using templates = types<template_<Templates>...>;
```
`templates` wraps a sequence of templates in a ->types list of ->template_
wrappers.
