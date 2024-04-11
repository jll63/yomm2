# RuntimeClass

This concept specifies the operations available on the objects in the range
passed to ->`publish_vptrs` function of facet ->`policy-vptr_vector`, and the
`hash_initialize` function of facet ->`policy-type_hash`.


## Member functions

### type_id_begin

```c++
ForwardIterator type_id_begin() const
```

Return the beginning of a range of `type_id`s for the class.

### type_id_end

```c++
ForwardIterator type_id_end() const
```
Return the end of a range of `type_id`s for the class.

### vptr

```c++
const std::uintptr_t* vptr() const
```

Return a pointer to the v-table for the class. The pointer changes every time
`update` is called.

### indirect_vptr

```c++
const std::uintptr_t* const* indirect_vptr() const
```

Return a pointer to a pointer to the v-table. This pointer to the pointer is
stable across calls to `update`, although the pointer itself changes.
