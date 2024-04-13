entry: policy::vptr_map
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
template<class Policy>
struct vptr_map;
```

`vptr_map` is an implementation of ->`policy-external_vptr` that stores the
pointers to the v-tables in a `std::unordered_map` keyed by the ->`type_id`s of
the classes. This makes method dispatch slower than ->`policy-vptr_vector` with
->`policy-fast_perfect_hash` (75% slower than native virtual function). However,
`vptr_map` has some advantages: `fast_perfect_hash` takes more time to
initialize. It also sacrifices memory space for speed, as it uses a hash
function that is not suitable for perfect _and_ minimal hashing. Using
`virtual_ptr`s extensively can mitigate the speed disadvantage of `vptr_map`.

## Template parameters

**Policy** - the policy containing the facet.

## static member functions
|                                 |                                                 |
| ------------------------------- | ----------------------------------------------- |
| [dynamic_vptr](#dynamic_vptr)   | return the address of the v-table for an object |
| [publish_vptrs](#publish_vptrs) | store the vptrs                                 |

### dynamic_vptr

```c++
template<class Policy>
template<class Class>
const std::uintptr_t* vptr_map<Policy>::dynamic_vptr(const Class& object);
```

Return a pointer to the v-table for `object`.

Call `Policy::dynamic_type` for `object`. Return the vptr associated to the
resulting ->`type_id`.

### publish_vptrs

```c++
template<class Policy>
template<typename ForwardIterator>
void vptr_map<Policy>::publish_vptrs(ForwardIterator first, ForwardIterator last);
```

Store the pointers to the v-tables.
