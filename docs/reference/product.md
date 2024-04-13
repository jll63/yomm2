

**yorel::yomm2::product** <small>(experimental)</small><br>
<sub>defined in <yorel/yomm2/templates.hpp>></sub><br/>
```
template<typename... TypeLists>
using product = /*unspecified*/;
```
`product` takes a list of [types](/yomm2/reference/types.html) lists, and evaluates to a `types` list
consisting of the n-fold Cartesian product of the input lists.

## Example


```c++
#include <type_traits>
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/templates.hpp>

namespace yomm2 = yorel::yomm2;

struct a;
struct b;
struct x;
struct y;
struct z;

static_assert(
    std::is_same_v<
        yomm2::product<
            yomm2::types<a, b>,
            yomm2::types<x, y, z>
        >,
        yomm2::types<
            yomm2::types<a, x>, yomm2::types<a, y>, yomm2::types<a, z>,
            yomm2::types<b, x>, yomm2::types<b, y>, yomm2::types<b, z>
        >
    >);
```
