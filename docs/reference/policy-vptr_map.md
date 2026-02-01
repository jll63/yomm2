> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>vptr_map</strong></span><br/>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by <yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub><br/>

```c++
template<class Policy>
struct vptr_map;
```

`vptr_map` is an implementation of [`external_vptr`](/yomm2/reference/policy-vptr_placement.html) that stores the
pointers to the v-tables in a `std::unordered_map` keyed by the [`type_id`](/yomm2/reference/type_id.html)s of
the classes. This makes method dispatch slower than [`vptr_vector`](/yomm2/reference/policy-vptr_vector.html) with
[`fast_perfect_hash`](/yomm2/reference/policy-fast_perfect_hash.html) (75% slower than native virtual function). However,
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
resulting [`type_id`](/yomm2/reference/type_id.html).

### publish_vptrs

```c++
template<class Policy>
template<typename ForwardIterator>
void vptr_map<Policy>::publish_vptrs(ForwardIterator first, ForwardIterator last);
```

Store the pointers to the v-tables.
