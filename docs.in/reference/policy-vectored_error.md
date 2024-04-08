# yorel::yomm2::policy::**vectored_error**
headers: yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

```c++
template<class Policy>
struct vectored_error;
```

`vectored_error` implements ->`policy-error_handler` using a static
`std::function`.

## Template parameters

| Name                        | Value                           |
| --------------------------- | ------------------------------- |
| class [**Policy**](#policy) | the policy containing the facet |

### Policy

The policy containing the facet. Since `vectored_error` has a static member
variable, making the policy a template parameter ensures that each policy has
its own copy.

**Static member variables**

|                                        |                       |
| -------------------------------------- | --------------------- |
| error_handler_type [**error**](#error) | current error handler |

### error

```c++
static error_handler_type error;
```

A `std::function` (see ->`error_handler_type`), initialized to
`default_error_handler`.

The function may throw an exception (unless they have been disabled), thus
preventing program termination.

**Static member functions**

|                                                   |                   |
| ------------------------------------------------- | ----------------- |
| [`default_error_handler`](#default_error_handler) | print diagnostics |

### default_error_handler

```c++
static void default_error_handler(const error_type& error_v)
```

If ->`policy-error_output` is available in `Policy`, use it to print a
description of `error`. Return normally, causing the program to be aborted.


## Interactions with other facets

* ->`policy-error_output`: for diagnostics.
