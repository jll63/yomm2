> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>checked_perfect_hash</strong></span><br/>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by <yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub><br/>
```
template<class Policy>
struct checked_perfect_hash;
```


`checked_perfect_hash` is a subclass of [`fast_perfect_hash`](/yomm2/reference/policy-fast_perfect_hash.html). It checks
that the id passed to `hash_type_index` is valid, i.e. was in the set of ids
registered via `hash_initialize`.

## Template parameters

**Policy** - the policy containing the facet.

## Static member functions

| Name                                            | Description                           |
| ----------------------------------- | ---------------------------- |
| [hash_initialize](#hash_initialize) | finds a hash function        |
| [hash_type_id](#hash_type_id)       | returns the hashed `type_id` |

### hash_initialize

```c++
template<class Policy>
template <typename ForwardIterator>
static size_t hash_initialize(ForwardIterator first, ForwardIterator last)
```

Calls
[`fast_perfect_hash::hash_initialize`](policy-fast_perfect_hash.html#hash_initialize).
Also build a reverse mapping, from hashed ids to registered ids.


#### Parameters

**first**, **last** - a range of `type_id`s

#### Return value

None.

#### Errors

* Any exception thrown by Allocator::allocate() (typically std::bad_alloc).

### hash_type_id

```c++
template<class Policy>
static type_id hash_type_id(type_id type)
```

Retrieve the registered id corresponding to the hashed id. Compare it with
`type`. If they are the same, return the hashed value. If not, report an error
via `Policy::error` if `Policy` has an `error_handler` facet; otherwise,
abort the program.

#### Parameters

**type** - a `type_id`

#### Return value

None.

#### Errors

* [`unknown_class_error`](/yomm2/reference/error.html), with `context` set to `unknown_class_error::update`.

## Interactions with other facets

* `error_handler` - to report error conditions.
* `error_output` - for diagnostics.
* `trace_output` - for trace.
