#define BOOST_TEST_MODULE fast_perfect_hash
#include <boost/test/included/unit_test.hpp>

/***

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::fast_perfect_hash
hrefs: checked_fast_perfect_hash
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct fast_perfect_hash; struct checked_fast_perfect_hash;
```

`fast_perfect_hash` is an implementation of ->`type_hash`. It finds a perfect hash function (i.e. collision-free)

***/

#include <sstream>
#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;

struct test : basic_policy<test> {}; // dummy policy

BOOST_AUTO_TEST_CASE(ref_fast_perfect_hash) {
    std::vector<type_id> ids = {42, 1963, 602701};
    fast_perfect_hash<test>::hash_initialize(ids.begin(), ids.end());

    for (auto id : ids) {
        BOOST_TEST_MESSAGE(
            id << " -> " << fast_perfect_hash<test>::hash_type_id(id));
        // 42 -> 2
        // 1963 -> 0
        // 602701 -> 1

        for (auto other : ids) {
            BOOST_TEST(
                (fast_perfect_hash<test>::hash_type_id(id) ==
                 fast_perfect_hash<test>::hash_type_id(other)) ==
                (id == other));
        }
    }

    // problem:
    BOOST_TEST_MESSAGE(
        666 << " -> " << fast_perfect_hash<test>::hash_type_id(666));
    // 666 -> 0 !!!
}

struct checks : basic_policy<
                    checks, final_only_rtti, vectored_error_handler<checks>,
                    basic_error_output<checks, std::ostringstream>> {};

BOOST_AUTO_TEST_CASE(ref_check_fast_perfect_hash) {
    std::vector<type_id> ids = {42, 1963, 602701};
    checked_perfect_hash<checks>::hash_initialize(ids.begin(), ids.end());
    BOOST_TEST_MESSAGE(
        666 << " -> " << checked_perfect_hash<checks>::hash_type_id(666));
}
