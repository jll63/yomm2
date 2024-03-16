<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
## yorel::yomm2::policy::**error_handler**
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

    struct error_handler;

The `error_handler` facet provides a static callable member `error`.

When YOMM2 detects an error condition, it checks whether the policy contains a
`error_handler`; if yes, it calls `error`, then `abort`. The function can
prevent program termination by throwing an exception.

**Requirements for implementations of `error_handler`**

|                                   |                                  |
| --------------------------------- | -------------------------------- |
| `static error_handler_type error` | called when an error is detected |

**Implementations of `error_handler`**

|                  |                                                                        |
| ---------------- | ---------------------------------------------------------------------- |
| [throw_error](/reference/throw_error.md)    | throw the error as an exception                                        |
| [vectored_error](/reference/vectored_error.md) | implement `error` as `std::function`; default value prints diagnostics |
