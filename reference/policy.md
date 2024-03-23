<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
# yorel::yomm2::policy::**basic_policy**


<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    template<class Policy, class... Facets>
    struct basic_policy : virtual Facets... { ... };

Creates a new policy class with the specified facets.

Policies provide a catalog of class and method definitions, supplied by the
user, and dispatch data, filled by `update`, and used during method calls. They
also act as customization points.

Templates `use_classes` and `method`, and macros `register_classes` and
`declare_method` accept a policy class as their first argument. If it is not
provided, `yorel::yomm2::default_policy` is used.

Class registrations and methods are scoped in a policy. A method can only
reference classes registered in the same policy. If a class is used as a virtual
parameter in methods using different policies, it must be registered with each
of them.

A policy has a collection of _facets_, implemented as classes inherited by the
policy via inheritance.  Facets control how vptrs are fetched, errors are
handled, etc. Facets fall into categories, and sometimes sub-categories. A
policy can have at most one facet per category. Some facet categories may be
absent from a policy; in which case, the corresponding functionality is not
available.

YOMM2 supports the following facet categories, and provides at least one
implementation for each category. They are summed up in the following table.

(facet sub-categories are in italics)

| Facet category           | Responsibility                    | Stock implementations                                         |
| ------------------------ | --------------------------------- | ------------------------------------------------------------- |
| [vptr_placement](/reference/vptr_placement.md)         | fetch vptr for virtual argument   |                                                               |
| *[external_vptr](/reference/vptr_placement.md)*        | store vptr outside the object     | [vptr_vector](/reference/vptr_vector.md)<...> (D), (R), [vptr_map](/reference/vptr_map.md)<...>                  |
| [rtti](/reference/rtti.md)                   | provide type information          | [std_rtti](/reference/std_rtti.md) (D), (R), [final_only_rtti](/reference/final_only_rtti.md)                        |
| ->*deferred_static_rtti* | as `rtti`, but avoid static ctors |                                                               |
| [type_hash](/reference/type_hash.md)              | map type info to integer index    | [fast_perfect_hash](/reference/fast_perfect_hash.md)<...> (R), [checked_perfect_hash](/reference/checked_perfect_hash.md)<...> (D) |
| [error_handler](/reference/error_handler.md)          | report errors                     | [vectored_error](/reference/vectored_error.md)<...>, [throw_error](/reference/throw_error.md)                          |
| [error_output](/reference/error_output.md)           | print diagnostics                 | [basic_error_output](/reference/basic_error_output.md)<...> (D)                                 |
| [trace_output](/reference/trace_output.md)           | trace                             | [basic_trace_output](/reference/basic_trace_output.md)<...> (D)                                 |

(D) denotes facets used in the default policy for debug variants, (R) for release
variants.

Several facets are [CRTP](https://en.cppreference.com/w/cpp/language/crtp) class
templates, taking the policy as the first template argument. Some facets contain
static, global data; parameterizing the facet by the policy ensures that each
policy gets its own global data. Some facets also need to access other facets in
the same policy.

### Template parameters

**Policy** - The policy class to inject a class and method catalog into (via
[CRTP](https://en.cppreference.com/w/cpp/language/crtp).

**Facets...** - The policy's facets.

### Member static data

|                             |                                                                        |
| --------------------------- | ---------------------------------------------------------------------- |
| [has_facet](#has_facet)     | `true` if policy contains a facet derived from `Category`  (constexpr) |
| [static_vptr](#static_vptr) | remove facet derived from `Category`                                   |

### Member types

|                     |                                                    |
| ------------------- | -------------------------------------------------- |
| [rebind](#rebind)   | return a new `basic_policy`, rebinding CRT facets  |
| [replace](#replace) | replace facet derived from `Category` with `Facet` |
| [remove](#remove)   | remove facet derived from `Category`               |

### has_facet

```
template<class Category> static constexpr bool has_facet = ...;
```

Evaluates to `true` if the policy contains a facet class derived from
`Category`.

### static_vptr

``````
template<class Class> static std::uintptr_t* static_vptr;
``````

Contains a pointer to the virtual table for `Class`. Valid only after `update`
has been called for the policy.

### rebind

```
template<class NewPolicy>
using rebind = basic_policy<NewPolicy, NewFacets...>;
```

Create a new policy with its own static data, separate from the original policy.
Inherit the facet implementations of `Policy`, rebound to `NewPolicy`
(i.e. the `Policy` argument of CRTP facets is replaced with `NewPolicy`;
non-template facets are copied as is).

### replace

```
template<class Category, class Facet>
using replace = basic_policy<Policy, ...>;
```

Create a new policy with the same static data as the original policy, having the
same facets as `Policy`, except for the facet derived from `Category`, whihc is
replaced with `Facet`.

### remove

```
template<class Category, class Facet>
using remove = basic_policy<Policy, ...>;
```

Create a new policy with the same static data as the original policy, having
the same facets as `Policy`, minus the facet derived from `Category`.

## Example

Given a `rtti` facet implementation, `custom_rtti`, that maps types to a dense
range of integer values. We want to create a policy that is the same as the
default policy in every aspect, except that it uses custom RTTI. In addition, we
don't need to hash the integer type id, so we remove the `type_hash` facet:

```c++
struct custom_policy : default_policy
  ::rebind<custom_policy>
  ::replace<policy::rtti, custom_rtti>
  ::remove<policy::type_hash> {
};
```
