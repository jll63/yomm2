#ifdef YOMM2_MD
<sub>/ ->home / ->reference </sub>

experimental: yorel::yomm2::template_
<sub>defined in <yorel/yomm2/templates.hpp></sub>
<!-- -->
---
```
template<template<typename...> typename Template>
struct template_ {
    template<typename... Ts>
    using fn = /*unspecified*/;
};
```
<!-- -->
---

`template_` wraps a template in a type, making it possible to appear in
->types lists. Nested template `fn<Ts...>` evaluates to the instantiation of
the template with the specified types.

## example

#endif

// clang-format off

#ifdef YOMM2_CODE

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/templates.hpp>

#include <type_traits>
#include <utility>

using namespace yorel::yomm2;

struct a;
struct b;

static_assert(
    std::is_same_v<
        template_<std::pair>::fn<char, int>,
        std::pair<char, int>
    >);

#endif

int main() {}
