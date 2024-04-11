entry: policy,policy::basic_policy
entry: policy::debug,policy::release
entry:policy::debug_shared,policy::release_shared
entry: default_policy
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
namespace policy {

template<class Policy, class... Facets>
struct basic_policy : virtual Facets...;

struct debug;
struct release;
struct debug_shared;
struct release_shared;

} // namespace policy

using default_policy = policy::/*build variant dependant*/;
```

`basic_policy` creates a new policy class with the specified facets. It is
defined in namespace `yorel::yomm2::policy`, along with several stock policies.
`default_policy`, defined in the main `yorel::yomm2` namespace, is an alias to
one of the stock policies, depending on the build variant.

### Template parameters

**Policy**: the policy class to inject a class and method catalog into (via
[CRTP](https://en.cppreference.com/w/cpp/language/crtp).

**Facets...**: the policy's facets.

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
same facets as `Policy`, except for the facet derived from `Category`, which is
replaced with `Facet`.

### remove

```
template<class Category, class Facet>
using remove = basic_policy<Policy, ...>;
```

Create a new policy with the same static data as the original policy, having
the same facets as `Policy`, minus the facet derived from `Category`.

## Discussion

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

| Facet category                  | Responsibility                    | Stock implementations                                                            |
| ------------------------------- | --------------------------------- | -------------------------------------------------------------------------------- |
| ->policy-vptr_placement         | fetch vptr for virtual argument   |                                                                                  |
| *->policy-external_vptr*        | store vptr outside the object     | ->policy-vptr_vector (D) (R), ->policy-vptr_map                                  |
| ->policy-rtti                   | provide type information          | ->policy-std_rtti (D) (R), ->policy-minimal_rtti                                  |
| *->policy-deferred_static_rtti* | as `rtti`, but avoid static ctors |                                                                                  |
| ->policy-type_hash              | map type info to integer index    | ->policy-fast_perfect_hash (R), ->policy-checked_perfect_hash (D)                |
| ->policy-error_handler          | report errors                     | ->policy-vectored_error, ->policy-throw_error, backward_compatible_error_handler |
| ->policy-error_output           | print diagnostics                 | ->policy-basic_error_output (D)                                                  |
| ->policy-trace_output           | trace                             | ->policy-basic_trace_output (D)                                                  |

(D) denotes facets used in the default policy for debug variants, (R) for release
variants.

Several facets are [CRTP](https://en.cppreference.com/w/cpp/language/crtp) class
templates, taking the policy as the first template argument. Some facets contain
static, global data; parameterizing the facet by the policy ensures that each
policy gets its own global data. Some facets also need to access other facets in
the same policy.

The stock policies consist of the following facets:

* **debug**: for error detection and trace. Consists of std_rtti,
  checked_perfect_hash, vptr_vector, basic_error_output, basic_trace_output and
  backward_compatible_error_handler.

* **release**: for maximum performance. Consists of fast_perfect_hash,
  vptr_vector, std_rtti and backward_compatible_error_handler.

* **debug_shared**: consists of vptr_vector, std_rtti, checked_perfect_hash,
  basic_error_output, basic_trace_output and backward_compatible_error_handler.
  The member function and variables are declared as external in the headers, and
  explicitly instantiated in the shared library.

* **release_shared**: same as debug_shared, but checkeds are bypassed.


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
