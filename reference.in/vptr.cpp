#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_MD

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::vptr
entry: yorel::yomm2::policy::external_vptr
entry: yorel::yomm2::policy::external_vptr_vector
entry: yorel::yomm2::policy::external_vptr_map
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct vptr;

struct external_vptr;

template<class Policy>
struct external_vptr_vector;

template<class Policy>
struct external_vptr_map;
```
---
A `vptr` facet is provides a static function that returns a pointer to the
dispatch data for a virtual argument's dynamic class.

YOMM2 implements method dispatch in a way similar to native virtual function
dispatch: for each virtual argument, fetch a pointer to the dispatch data (the
v-table), and use it to select a pointer to a function. Method v-tables contain
pointers to functions for unary methods, and, for multi-methods, pointers to,
and coordinates in, a multi-dimensional table of pointers to functions.

The `vptr` facet is used during method call to fetch the vptr for virtual
arguments corresponding to the `virtual_` parameters in the method declaration.
It is also used by the constructor of `virtual_ptr` to obtain a vptr on the
basis of an object's dynamic type.

`virtual_ptr::final`, and the related convenience functions, assume that the
static and dynamic types of their argument are the same. The vptr is obtained
statically from the policy's `static_vptr<Class>` member. It is conceivable to
organize an entire program around the "final" constructs; thus, the `vptr` facet
is optional.

`external_vptr` is a sub-category of `facet`. If present, the runtime calls its
static functions to allow it to initialize its data structures.

#endif



BOOST_AUTO_TEST_CASE(ref_vptr) {}
