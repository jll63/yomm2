#ifdef YOMM2_MD

experimental: yorel::yomm2::product
headers:yorel/yomm2/templates.hpp>
```
template<typename... TypeLists>
using product = /*unspecified*/;
```
`product` takes a list of ->mp_list lists, and evaluates to a `mp_list` list
consisting of the n-fold Cartesian product of the input lists.

## Example

#endif

// clang-format off

#ifdef YOMM2_CODE

#include <type_traits>
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/templates.hpp>

namespace yomm2 = yorel::yomm2;
using boost::mp11::mp_list;

struct a;
struct b;
struct x;
struct y;
struct z;

static_assert(
    std::is_same_v<
        yomm2::product<
            mp_list<a, b>,
            mp_list<x, y, z>
        >,
        mp_list<
            mp_list<a, x>, mp_list<a, y>, mp_list<a, z>,
            mp_list<b, x>, mp_list<b, y>, mp_list<b, z>
        >
    >);

#endif

int main() {}
