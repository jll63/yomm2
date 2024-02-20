<sub>/ [home](/reference//README.md) / [reference](/reference//reference/README.md) </sub>

**yorel::yomm2::policy::type_hash**<br>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

---
```
struct type_hash {};
```
---

The `type_hash` facet projects a sparse range of [`type_id`](/reference/type_id.md)s to a dense range.

### Requirements for implementations of `type_hash`

An implementation of `type_hash` must provide the following static functions:

|                                     |                             |
| ----------------------------------- | --------------------------- |
| [hash_initialize](#hash_initialize) | implementation dependent    |
| [hash_type_id](#hash_type_id)       | return the hashed `type_id` |

### Implementations of `type_hash`

|                                              |                                                               |
| -------------------------------------------- | ------------------------------------------------------------- |
| [fast_perfect_hash](/reference/fast_perfect_hash.md)                          | use a fast, perfect, but not minimal integer hash             |
| [checked_perfect_hash](fast_perfect_hash.md) | like `fast_perfect_hash`, also check for unregistered classes |


### hash_initialize

```c++
template<typename ForwardIterator>
static size_t hash_initialize(ForwardIterator first, ForwardIterator last);
```

Called by another facet in the policy if it requires hashing. [`vptr_vector`](/reference/vptr_vector.md) in
the default policy does that.

The function takes a range of [`type_id`](/reference/type_id.md)s, and finds a hash function for that
specific set of values.

### hash_type_id

```c++
static type_id hash_type_id(type_id type);
```

Called during method dispatch. Returns the corresponding value in the dense
range. `type` must be one of the `type_id`s passed to `hash_initialize`.
