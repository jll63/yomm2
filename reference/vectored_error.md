<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
## yorel::yomm2::policy::**vectored_error**
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

    template<class Policy>
    struct vectored_error;

`vectored_error` is an implementation of [`error_handler`](/reference/error_handler.md) that provides `error`
as a static `std::function` member. `error` can be set to a user-defined
function, which may throw an exception to prevent program termination.

**Template parameters**

* `Policy`: the policy containing the facet.

**Static member functions**

|                                          |                   |
| ---------------------------------------- | ----------------- |
| [`default_error_handler`](/reference/vectored_error/default_error_handler.md) | print diagnostics |

**Static member variables**

|                          |                       |
| ------------------------ | --------------------- |
| [`error`](/reference/vectored_error/error.md) | current error handler |


## Interactions with other facets

* `error_output`: for diagnostics.
