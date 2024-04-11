entry: policy::type_hash
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp
refs: type_hash

```
struct type_hash {};
```

The `type_hash` facet projects a sparse range of ->`type_id`s to a dense range.

### Requirements for implementations of `type_hash`

An implementation of `type_hash` must provide the following static functions:

|                                     |                             |
| ----------------------------------- | --------------------------- |
| [hash_initialize](#hash_initialize) | implementation dependent    |
| [hash_type_id](#hash_type_id)       | return the hashed `type_id` |

### Implementations of `type_hash`

|                               |                                                               |
| ----------------------------- | ------------------------------------------------------------- |
| ->policy-fast_perfect_hash    | use a fast, perfect, but not minimal integer hash             |
| ->policy-checked_perfect_hash | like `fast_perfect_hash`, also check for unregistered classes |


### hash_initialize

```c++
template<typename ForwardIterator>
static size_t hash_initialize(ForwardIterator first, ForwardIterator last);
```

Called by another facet in the policy if it requires hashing.
->`policy-vptr_vector` in the default policy does that.

The function takes a range of ->`type_id`s, and finds a hash function for that
specific set of values.

### hash_type_id

```c++
static type_id hash_type_id(type_id type);
```

Called during method dispatch. Returns the corresponding value in the dense
range. `type` must be one of the `type_id`s passed to `hash_initialize`.
