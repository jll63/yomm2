<sub>/ [home](/README.md) / [reference](/reference/README.md) </sub>

**yorel::yomm2::policy::vptr_vector**<br>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

---
```
template<class Policy>
struct vptr_vector : virtual external_vptr { ... };
```
---

`vptr_vector` is an implementation of [`external_vptr`](vptr_placement.md) that stores the
pointers to the v-tables in a `std::vector`. If the policy contains a
[`type_hash`](/reference/type_hash.md) facet, it is used to convert the [`type_id`](/reference/type_id.md) to an index in the
vector; otherwise, the `type_id` is used as the index.

The default policy uses [`std_rtti`](/reference/std_rtti.md), [`fast_perfect_hash`](/reference/fast_perfect_hash.md) and `vptr_vector`
to implement efficient method dispatch. Calling a method with a single virtual
parameter takes only ~33% more time than calling a native virtual function call.

## Template parameters

**Policy** - the policy containing the facet.

## static member functions
|                                   |                                                    |
| --------------------------------- | -------------------------------------------------- |
| [dynamic_vptr](#dynamic_vptr)     | return the address of the v-table for an object    |
| [register_vptrs](#register_vptrs) | store the vptrs, initialize `type_hash` if present |

### dynamic_vptr

```c++
template<class Policy>
template<class Class>
const std::uintptr_t* vptr_vector<Policy>::dynamic_vptr(const Class& object);
```

Return a pointer to the v-table for `object`.

Call `Policy::dynamic_type` for `object`. If `Policy` contains a `type_hash`
facet, use it to convert the resulting `type_id` to an index; otherwise, use the
`type_id` as the index.

### register_vptrs

```c++
template<class Policy>
template<typename ForwardIterator>
void vptr_vector<Policy>::register_vptrs(ForwardIterator first, ForwardIterator last);
```

If `Policy` contains a `type_hash` facet, call its `hash_initialize`
function.

Store the pointers to the v-tables in a vector, indexed by the (possibly hashed)
`type_id`s.
