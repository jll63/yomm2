> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>error_handler</strong></span><br/>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

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
can throw an exception, derived from class [`error`](/yomm2/reference/error.html), to prevent program
termination.

## Implementations of `error_handler`

|                         |                                                                        |
| ----------------------- | ---------------------------------------------------------------------- |
| [throw_error](/yomm2/reference/policy-throw_error.html)    | throw the error as an exception                                        |
| [vectored_error](/yomm2/reference/policy-vectored_error.html) | implement `error` as `std::function`; default value prints diagnostics |
