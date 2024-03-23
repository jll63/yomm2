# # yorel::yomm2::policy::**throw_error**
location: policy;yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    struct throw_error;

`throw_error` implements ->`error_handler` by throwing errors as exceptions.

If exceptions are disabled, `throw_error` is not defined.

**Static member functions**

|                                   |                                 |
| --------------------------------- | ------------------------------- |
| [**error**](#error) | throw the error variant's value |

### error

```c++
static void error(const error_type& error_variant);
```

Extract the value of `error_variant`, and throw it as an exception.
