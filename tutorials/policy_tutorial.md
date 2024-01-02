

YOMM2's customization point for method dispatch is the _policy_ class. Templates
`use_classes` and `method`, and macros `register_classes` and `declare_method`,
use the `yorel::yomm2::default_policy`; this can be overriden by specifying a
policy class as the first argument. Alternatively, the default policy used by
`register_classes` and `declare_method` can be overridden by re-defining
preprocessor symbol `YOMM2_DEFAULT_POLICY`.

Note that class registrations and methods are scoped in their policy. If a class
is used as a virtual parameter in methods using different policies, it must be
registered with each of them.

A policy consists of a collection of _facets_ classes, inherited by the policy
(virtual inheritance is used throughout the facet mechanism). Facets fall into
categories, and sometimes sub-categories. A policy can have at most one facet
per category. Some facet categories may be absent from a policy; in which case,
the corresponding functionality is not available.

YOMM2 supports the following facet categories, and provides at least one
implementation for each category. They are summed up in the following table.
Facet categories in bold are required for YOMM2 to work at all. Facet
implementations in bold are used in the default policy, either in debug (D) or
release (R) builds only, or in both.

| facet category      | responsibility                  | implementations                                                                 |
| ------------------- | ------------------------------- | ------------------------------------------------------------------------------- |
| vptr, external_vptr | fetch vptr for virtual argument | **external_vptr_vector** (D, R), external_vptr_map                              |
| **rtti**            | type information                | **std_rtti** (D, R), minimal_rtti                                               |
| type_hash           | map type info to integer index  | **simple_perfect_hash** (R), **checked_simple_perfect_hash** (D)                |
| error       | report errors                   | **backward_compatible_error_handler** (D, R), vectored_error_handler, exception |
| output              | report errors, trace            | **generic_output** (D)                                                          |

## The `vptr` facet

This facet is responsible for obtaining a pointer to the dispatch data for a
virtual argument's dynamic class.

YOMM2 implements method dispatch in a similar fashion that the compiler
implements virtual function dispatch. For each virtual argument, fetch a pointer
to the dispatch data (the v-table), and use it to select a pointer to a
function. Method v-tables contain pointers to functions for unary methods, and,
for multi-methods, pointers to, and coordinates in, a multi-dimensional table of
pointers to functions.

The `vptr` facet is used during method call to fetch the vptr for virtual
arguments corresponding to the `virtual_` parameters in the method declaration.
It is also used by the constructor of `virtual_ptr` to obtain a vptr on the
basis of an object's dynamic type.

`virtual_ptr::final`, and it associated convenience functions, assume that the
static and dynamic types of their argument are the same. The vptr is obtained
statically from the policy's `static_vptr<Class>` member. It is conceivable to
organize an entire program around the "final" constructs; thus, the `vptr` facet
is optional.

## The `rtti` and `type_hash` facets

See the [custom RTTI tutorial](custom_rtti_tutorial.md) for a full explanation
of these facets.

## The `error` facet

## The `output` facet


