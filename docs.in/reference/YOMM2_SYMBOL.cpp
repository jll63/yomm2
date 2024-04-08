#ifdef YOMM2_MD

entry: YOMM2_SYMBOL
headers: yorel/yomm2/symbols.hpp, yorel/yomm2/keywords.hpp
```
#define YOMM2_SYMBOL(seed) /*unspecified*/
```Macro `YOMM2_SYMBOL` expands to an obfuscated symbol, unlikely
to clash with other names in scope.


## Example

#endif

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_CODE

#include <yorel/yomm2/symbols.hpp>

BOOST_AUTO_TEST_CASE(ref_example) {
    int foo = 1;
    int YOMM2_SYMBOL(foo) = 2;
    BOOST_TEST(foo == 1);
    BOOST_TEST(YOMM2_SYMBOL(foo) == 2);
}

#endif
