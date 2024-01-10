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

## The `error_output` facet

If the facet is present, it provides one static data member, `stream`, which
must support a small subset of the protocol of `std::stream`. The following
insertion operators are required:

```c++
Stream& operator<<(Stream& os, const std::string_view& view);
Stream& operator<<(Stream& os, const void* value);
Stream& operator<<(Stream& os, size_t value);
```
(where `Stream` is the type of the `stream` data member)

When an error is encountered, information is written to `stream` using the
operators above.

YOMM2 provides one implementation of this facet: `basic_output<class Policy,
class Stream = unspecified>`. The default value of `Stream` is not
`std::ostream`, but a more lightweight mechanism that writes to `stderr`

#endif

BOOST_AUTO_TEST_CASE(ref_output) {}
