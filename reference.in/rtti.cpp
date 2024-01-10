#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_MD

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::vptr
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
template<class Policy>
struct facet;
```
---

#endif

#ifdef YOMM2_MD

## See also
The [custom RTTI tutorial](custom_rtti_tutorial.md) for a full explanation
of these facets.
#endif

BOOST_AUTO_TEST_CASE(ref_rtti) {}
