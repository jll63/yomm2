#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

/***

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::fast_perfect_hash hrefs: checked_fast_perfect_hash
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct fast_perfect_hash; struct checked_fast_perfect_hash;
```

`fast_perfect_hash` is an implementation of ->`type_hash`. It finds a perfect hash function (i.e. collision-free)

***/

#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;

template<size_t> struct dummy_policy;

BOOST_AUTO_TEST_CASE(ref_fast_perfect_hash) {
    //ref_fast_perfect_hash
}
