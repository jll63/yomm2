
<sub>/ [home](/README.md) / [reference](README.md) </sub>
## yorel::yomm2::product <small>(experimental)</small>
<sub>defined in <yorel/yomm2/templates.hpp></sub>
<!-- -->
---
```
template<typename... TypeLists>
using product = /*unspecified*/;
```
<!-- -->
---

`product` takes a list of [types](types.md) lists, and evaluates to a `types` list
consisting of the n-fold Cartesian product of the input lists.

## example

```c++

#include <type_traits>
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/templates.hpp>

using namespace yorel::yomm2;

struct a;
struct b;
struct x;
struct y;
struct z;

static_assert(
    std::is_same_v<
        product< 
            types<a, b>, 
            types<x, y, z> 
        >,
        types<
            types<a, x>, types<a, y>, types<a, z>,
            types<b, x>, types<b, y>, types<b, z>
        >
    >);


```
