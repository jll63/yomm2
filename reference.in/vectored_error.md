<sub>/ ->home / ->reference </sub>

**yorel::yomm2::vectored_error**<br>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

---

```
template<class Policy>
struct vectored_error;
```

`vectored_error` is an implementation of ->`error_handler` that provides an
`error` static `std::function` member. Its default value prints diagnostics
using the `error_output` facet, if present. Since it returns normally, the
program is aborted by the caller. `error` can be set to a different function,
which can `throw` to prevent program termination.

## Interactions with other facets

* `error_output` - for diagnostics.

## Template parameters

**Policy** - the policy containing the facet.

## Members

| static member variables             |                                  |
| ----------------------------------- | -------------------------------- |
| error                               | throw value contained in variant |
| static member functions             |                                  |
| default_error_handler               | print diagnostics                |

### error

```c++
static void error(const error_type& error_variant);
```

Extract the value of `error_variant`, and throw it as an exception.

#### Parameters

**error_variant** - A variant containing an instance of a subclass of `error`.

#### Return value

None.

#### Errors

None.
