#ifdef YOMM2_MD
experimental: yorel::yomm2::template_
headers: <yorel/yomm2/templates.hpp>

```c++
template<template<typename...> typename Template>
struct template_ {
    template<typename... Ts>
    using fn = /*unspecified*/;
};
```

`template_` wraps a template in a type, making it possible to appear in
->types lists. Nested template `fn<Ts...>` evaluates to the instantiation of
the template with the specified types.

## Example

#endif

// clang-format off

#ifdef YOMM2_CODE

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>
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
