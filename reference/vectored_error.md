<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
# **vectored_error**
<sub>defined in yorel::yomm2::policy by <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub>

```
template<class Policy>
struct vectored_error;
```

---

`vectored_error` is an implementation of [`error_handler`](/reference/error_handler.md) that provides an
`error` static `std::function` member. It is initialized to a function that
prints diagnostics using the `error_output` facet, if present. Since it returns
normally, the program is aborted by the caller. `error` can be set to a
different function, which can `throw` to prevent program termination.

## Interactions with other facets

* `error_output` - for diagnostics.

## Template parameters

**Policy** - the policy containing the facet.

## Static member functions

|                                        |                   |
| -------------------------------------- | ----------------- |
| [default_error_handler](/reference/vectored_error/default_error_handler.md) | print diagnostics |

## Static member variables

|                        |                                  |
| ---------------------- | -------------------------------- |
| [error](/reference/vectored_error/error.md) | throw value contained in variant |
