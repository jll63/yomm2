> [!IMPORTANT]
> YOMM2 has been superseded by Boost.OpenMethod. See README for details.


<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>vptr_vector</strong></span><br/>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by <yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub><br/>

```c++
template<class Policy>
struct vptr_vector : virtual external_vptr { ... };
```

`vptr_vector` is an implementation of [`external_vptr](/yomm2/reference/policy-vptr_placement.html) that stores the
pointers to the v-tables in a `std::vector`. If the policy contains a
[`type_hash`](/yomm2/reference/policy-type_hash.html) facet, it is used to convert the [`type_id`](/yomm2/reference/type_id.html) to an index in the
vector; otherwise, the `type_id` is used as the index.

The default policy uses [`std_rtti`](/yomm2/reference/policy-std_rtti.html), [`fast_perfect_hash`](/yomm2/reference/policy-fast_perfect_hash.html) and
`vptr_vector` to implement efficient method dispatch. Calling a method with a
single virtual parameter takes only ~33% more time than calling a native virtual
function call.

## Template parameters

**Policy** - the policy containing the facet.

## Static member functions

|                                 |                                                    |
| ------------------------------- | -------------------------------------------------- |
| [dynamic_vptr](#dynamic_vptr)   | return the address of the v-table for an object    |
| [publish_vptrs](#publish_vptrs) | store the vptrs, initialize `type_hash` if present |

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

### publish_vptrs

```c++
template<class Policy>
template<typename ForwardIterator>
void vptr_vector<Policy>::publish_vptrs(ForwardIterator first, ForwardIterator last);
```

If `Policy` contains a `type_hash` facet, call its `hash_initialize`
function.

Store the pointers to the v-tables in a vector, indexed by the (possibly hashed)
`type_id`s.
