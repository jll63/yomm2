#define BOOST_TEST_MODULE fast_perfect_hash
#include <boost/test/included/unit_test.hpp>

/***

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::fast_perfect_hash hrefs: checked_fast_perfect_hash
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct fast_perfect_hash; struct checked_fast_perfect_hash;
```

`fast_perfect_hash` is an implementation of ->`type_hash`. It uses a very fast
hash function:

```
H(x) = (x * M) >> N
```

The `M` and `N` coefficients are determined during initialization so that, for
the given set of type ids, the hash function is _perfect_, i.e. collision-free.
It is not _minimal_: there may be gaps in the interval formed by applying the
hash function to all the known type ids.

***/

//***

#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;

struct test : basic_policy<test, fast_perfect_hash<test>> {}; // dummy policy

BOOST_AUTO_TEST_CASE(ref_fast_perfect_hash) {
  std::vector<type_id> ids = {42, 1963, 2001};
  test::hash_initialize(ids.begin(), ids.end());

  for (auto id : ids) {
    BOOST_TEST_MESSAGE(id << " -> " << test::hash_type_id(id));
    // 42 -> 0
    // 1963 -> 3
    // 602701 -> 2

    for (auto other : ids) {
      BOOST_TEST((test::hash_type_id(id) == test::hash_type_id(other)) ==
                 (id == other));
    }
  }
}
//***

/***
Problem

***/

//***

BOOST_AUTO_TEST_CASE(ref_fast_perfect_hash_problem) {
  BOOST_TEST_MESSAGE(666 << " -> " << test::hash_type_id(666));
  // 666 -> 3 !!!
}

//***

/***
solution

***/

//***

struct checked_policy
    : basic_policy<checked_policy, checked_perfect_hash<checked_policy>,
                   throw_error_handler> {};

BOOST_AUTO_TEST_CASE(ref_check_fast_perfect_hash) {
  std::vector<type_id> ids = {42, 1963, 602701};
  checked_policy::hash_initialize(ids.begin(), ids.end());
  bool caught = false;

  try {
    checked_policy::hash_type_id(666);
  } catch (unknown_class_error &error) {
    caught = true;
  }

  BOOST_TEST(caught);
}
//***
