<sub>['[home](/README.md)', '[reference](/reference.md)']</sub><br>
**error_handler**<br>
<sub>defined in yorel::yomm2::policy by <yorel/yomm2/keywords.hpp>, also provided by <yorel/yomm2/core.hpp></sub>


---
```
struct error_handler;
```

The `error_handler` facet provides a static member named `error`, either a
function or a functor.

When YOMM2 detects an error condition, it checks whether the policy contains a
`error_handler`; if yes, it calls `error`, then `abort`. The function
can prevent program termination by throwing an exception.

### Requirements for implementations of `error_handler`

|                          |                   |
| ------------------------ | ----------------- |
| [error](#register_vptrs) | handle ther error |

### Implementations of `error_handler`

|                  |                                 |
| ---------------- | ------------------------------- |
| [throw_error](/reference/throw_error.md)    | throw the error as an exception |
| [vectored_error](/reference/vectored_error.md) | call a `std::function`          |
