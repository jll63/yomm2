yorel::yomm2::policy::**type_hash**
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>
refs: type_hash

```
struct type_hash {};
```

The `type_hash` facet projects a sparse range of [`type_id`](/yomm2/reference/type_id.html)s to a dense range.

### Requirements for implementations of `type_hash`

An implementation of `type_hash` must provide the following static functions:

|                                     |                             |
| ----------------------------------- | --------------------------- |
| [hash_initialize](#hash_initialize) | implementation dependent    |
| [hash_type_id](#hash_type_id)       | return the hashed `type_id` |

### Implementations of `type_hash`

|                               |                                                               |
| ----------------------------- | ------------------------------------------------------------- |
| [fast_perfect_hash](/yomm2/reference/policy-fast_perfect_hash.html)    | use a fast, perfect, but not minimal integer hash             |
| [checked_perfect_hash](/yomm2/reference/policy-checked_perfect_hash.html) | like `fast_perfect_hash`, also check for unregistered classes |


### hash_initialize

```c++
template<typename ForwardIterator>
static size_t hash_initialize(ForwardIterator first, ForwardIterator last);
```

Called by another facet in the policy if it requires hashing.
[`vptr_vector`](/yomm2/reference/policy-vptr_vector.html) in the default policy does that.

The function takes a range of [`type_id`](/yomm2/reference/type_id.html)s, and finds a hash function for that
specific set of values.

### hash_type_id

```c++
static type_id hash_type_id(type_id type);
```

Called during method dispatch. Returns the corresponding value in the dense
range. `type` must be one of the `type_id`s passed to `hash_initialize`.
