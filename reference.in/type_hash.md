<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::type_hash
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct type_hash {};
```
---

The `type_hash` facet projects a set of ->`type_id`s to a compact range of
integers.

### Requirements for implementations of `type_hash`

An implementation of `type_hash` must provide the following static function
template:

|                                               |                             |
| --------------------------------------------- | --------------------------- |
| [type_hash_initialize](#type_hash_initialize) | find a hash function        |
| [hash_type_id](#hash_type_id)                 | return the hashed `type_id` |


### type_hash_initialize
```c++
```

### hash_type_id
```c++
```
