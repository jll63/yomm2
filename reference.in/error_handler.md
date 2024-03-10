**error_handler**<br>
location: policy;yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

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
| ->throw_error    | throw the error as an exception |
| ->vectored_error | call a `std::function`          |
