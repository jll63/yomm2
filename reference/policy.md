<sub>/ [home](/reference//README.md) / [reference](/reference//reference/README.md) </sub>

**yorel::yomm2::policy::basic_policy**<br>
**yorel::yomm2::default_policy**<br>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

---
```
template<class Policy, class... Facets>
struct basic_policy : virtual Facets... { ... };
```
---
Creates a new policy class with the specified facets.

_Policies_ are at the core of method dispatch. A policy  provides a catalog of
class and method definitions, supplied by the user, and dispatch data, filled by
`update`, and used during method calls.

Templates `use_classes` and `method`, and macros `register_classes` and
`declare_method` accept a policy class as their first argument. If it is not
provided, `yorel::yomm2::default_policy` is used.

Class registrations and methods are scoped in a policy. A method can only
reference classes registered in the same policy. If a class is used as a virtual
parameter in methods using different policies, it must be registered with each
of them.

A policy has a collection of _facets_ classes, inherited by the policy via
virtual inheritance.  Facets control how vptrs are fetched, errors are reported,
etc. Facets fall into categories, and sometimes sub-categories, expressed via
inheritance. A policy can have at most one facet per category. Some facet
categories may be absent from a policy; in which case, the corresponding
functionality is not available.

YOMM2 supports the following facet categories, and provides at least one
implementation for each category. They are summed up in the following table.

| facet category      | responsibility                  | implementations                                                              |
| ------------------- | ------------------------------- | ---------------------------------------------------------------------------- |
| vptr, external_vptr | fetch vptr for virtual argument | **external_vptr_vector\<...>** (D, R), external_vptr_map\<...>               |
| rtti, deferred_rtti | type information                | **std_rtti** (D, R), no_rtti                                                 |
| type_hash           | map type info to integer index  | **simple_perfect_hash\<...>** (R), **checked_simple_perfect_hash`<...>** (D) |
| error               | report errors                   | vectored_error_handler\<...>                                                 |
| error_output        | describe error                  | **basic_error_output\<...>** (D)                                             |
| update_output       | trace `update`                  | **basic_update_output\<...>** (D)                                            |

Facet categories in bold are required for YOMM2 to work at all. Facet
implementations in bold are used in the default policy, either in debug (D) or
release (R) builds only, or in both (D, R). A policy needs a `rtti` facet to be
useable. All others are optional.

Most facets are [CRTP](https://en.cppreference.com/w/cpp/language/crtp) class
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
