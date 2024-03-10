# **vectored_error**
location: policy;yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp
```
template<class Policy>
struct vectored_error;
```

---

`vectored_error` is an implementation of ->`error_handler` that provides an
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
| ->vectored_error/default_error_handler | print diagnostics |

## Static member variables

|                        |                                  |
| ---------------------- | -------------------------------- |
| ->vectored_error/error | throw value contained in variant |
