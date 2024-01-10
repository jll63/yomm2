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
If the facet is present, it provides one static member: `error`, either a
function or a functor that takes a ->`error_type` variant. It is called in the
following situations:

* While building dispatch tables (`update`), a class that has not been
  registered is used as a virtual parameter in a method declaration or
  definition.
* During a method call, no definition is available for a combination of virtual
  arguments, or more than one is available and none is more specialized than all
  the others.
* A facet encounters an error.

The function is allowed to throw an exception, `exit`, or anything it sees fit.
If it returns, the program is terminated with `abort`.

YOMM2 provides three implementations of the facet: `vectored_error_handler`. It
calls the function specified in the `error` static member variable of the facet
- a `std::function<void(const error_type& error)>` -, passing it the  error
variant.

#endif

BOOST_AUTO_TEST_CASE(ref_error) {}
