

<sub>/ [home](/reference//README.md) / [reference](/reference//reference/README.md) </sub>

**yorel::yomm2::policy::fast_perfect_hash**<br>

<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

---
```
struct fast_perfect_hash;
```

`fast_perfect_hash` is an implementation of [`type_hash`](/reference/type_hash.md). It uses the following
hash function:

```
H(x) = (x * M) >> S
```

The output values are in the interval `[0, 1 << (N - S)[`, where `N` is the
bit size of `type_id`.

The `M` and `S` coefficients are determined during initialization so that, for
the given set of type ids, the hash function is _perfect_, i.e. collision-free.
It is not _minimal_: there may be gaps in the output interval.

`hash_type_id` must not be called with ids not present in the input set passed
to `hash_initialize`. This happens when the class of a virtual argument was not
registered via [`use_classes`](/reference/use_classes.md) or [`register_classes`](/reference/use_classes.md). `ref_perfect_hash` will
silently return an index in the output range, to be used to fetch the v-table
for the class from a vector. From there, two things can happen. In the lucky
scenario, the index will correspond to a gap, a wasted entry in the v-table
vector, which will contain a null pointer. The program will segfault. Such
mistake is fairly easy to troubleshoot. In the unlucky scenario, there will be a
collision - the hash function is perfect only for the input ids - and the wrong
method will be called. The program will carry on, crashing far from the root
cause of the error, and, in the worst case, not at all.

To help detect such registration errors, it is highly recommended to run debug
builds of the program with [`checked_perfect_hash`](/reference/checked_perfect_hash.md), which detects missing class
registrations.

`default_policy` uses `fast_perfect_hash` in release builds, and
`checked_perfect_hash` in debug builds.

## Template parameters

**Policy** - the policy containing the facet.

## static member functions
|                                     |                              |
| ----------------------------------- | ---------------------------- |
| [hash_initialize](#hash_initialize) | finds a hash function        |
| [hash_type_id](#hash_type_id)       | returns the hashed `type_id` |

### hash_initialize

```c++
template<class Policy>
template <typename ForwardIterator>
static size_t hash_initialize(ForwardIterator first, ForwardIterator last)
```

Finds the `M` and `S` parameters of the hash function. `ForwardIterator` is an
iterator that satisfies the requirements of forward iterators; dereferencing it
yields a `type_id`. Sets `type_hash_last` to the maximum hash value, plus one.

#### Parameters

**first**, **last** - a range of `type_id`s

#### Return value

None.

#### Errors

None.

### hash_type_id

```c++
template<class Policy>
static type_id hash_type_id(type_id type)
```

Returns the hashed value of `type`, which must be one of the values passed to
`hash_initialize`.

#### Parameters

**type** - a `type_id`

#### Return value

None.

#### Errors

None.

### Example


```c++
#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;

struct fast_hash_policy
    : basic_policy<fast_hash_policy, fast_perfect_hash<fast_hash_policy>> {
};

BOOST_AUTO_TEST_CASE(ref_fast_perfect_hash) {
  std::vector<type_id> ids = {42, 1963, 2001};
  fast_hash_policy::hash_initialize(ids.begin(), ids.end());

  for (auto id : ids) {
    BOOST_TEST_MESSAGE(id << " -> " << fast_hash_policy::hash_type_id(id));
    // Output:
    // 42 -> 0
    // 1963 -> 3
    // 602701 -> 2

    for (auto other : ids) {
      BOOST_TEST((fast_hash_policy::hash_type_id(id) ==
                  fast_hash_policy::hash_type_id(other)) == (id == other));
    }
  }

  // Calling hash_type_id with an unregistered type_id returns bogus result:
  BOOST_TEST_MESSAGE(666 << " -> " << fast_hash_policy::hash_type_id(666));
  // 666 -> 3 !!!
}
```
