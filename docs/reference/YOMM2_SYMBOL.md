> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.



<span style="font-size:xx-large;"><strong>YOMM2_SYMBOL</strong><br/></span><br/>
<sub>defined in <yorel/yomm2/symbols.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

```c++
#define YOMM2_SYMBOL(seed) /*unspecified*/
```

Macro `YOMM2_SYMBOL` expands to an obfuscated symbol, unlikely
to clash with other names in scope.

## Example


```c++
#include <yorel/yomm2/symbols.hpp>

BOOST_AUTO_TEST_CASE(ref_example) {
    int foo = 1;
    int YOMM2_SYMBOL(foo) = 2;
    BOOST_TEST(foo == 1);
    BOOST_TEST(YOMM2_SYMBOL(foo) == 2);
}
```
