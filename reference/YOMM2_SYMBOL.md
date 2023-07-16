

<sub>/ [home](/README.md) / [reference](/reference/README.md) </sub>

**YOMM2_SYMBOL**<br>
<sub>defined in <yorel/yomm2/symbols.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

---
```
#define YOMM2_SYMBOL(seed) /*unspecified*/
```
---
Macro `YOMM2_SYMBOL` expands to an obfuscated symbol, unlikely
to clash with other names in scope.


## example


```c++
#include <yorel/yomm2/symbols.hpp>

BOOST_AUTO_TEST_CASE(reference_example) {
    int foo = 1;
    int YOMM2_SYMBOL(foo) = 2;
    BOOST_TEST(foo == 1);
    BOOST_TEST(YOMM2_SYMBOL(foo) == 2);
}
```
