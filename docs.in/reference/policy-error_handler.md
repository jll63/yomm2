entry: policy::error_handler
headers: yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

```c+
struct error_handler;
```

The `error_handler` is used by YOMM2 to handle error conditions. If it is
present, its static member `error` is called with a variant describing the
error. If the function returns normally, the program is terminated by a call to
`abort`.

## Requirements for implementations of **error_handler**

|                                   |                                  |
| --------------------------------- | -------------------------------- |
| _unspecified_ [**error**](#error) | called when an error is detected |

(all members are static)

### error

Handle the error condition. `error` can be a function or a function or a
functor, taking a single `const error_type&` argument, and returning `void`. It
can throw an exception, derived from class ->`error`, to prevent program
termination.

## Implementations of `error_handler`

|                         |                                                                        |
| ----------------------- | ---------------------------------------------------------------------- |
| ->policy-throw_error    | throw the error as an exception                                        |
| ->policy-vectored_error | implement `error` as `std::function`; default value prints diagnostics |
