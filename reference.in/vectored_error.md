## yorel::yomm2::policy::**vectored_error**
headers: yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    template<class Policy>
    struct vectored_error;

`vectored_error` is an implementation of ->`error_handler` that provides `error`
as a static `std::function` member. `error` can be set to a user-defined
function, which may throw an exception to prevent program termination.

**Template parameters**

* `Policy`: the policy containing the facet.

**Static member functions**

|                                          |                   |
| ---------------------------------------- | ----------------- |
| ->`vectored_error/default_error_handler` | print diagnostics |

**Static member variables**

|                          |                       |
| ------------------------ | --------------------- |
| ->`vectored_error/error` | current error handler |


## Interactions with other facets

* `error_output`: for diagnostics.
