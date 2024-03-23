<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
# # yorel::yomm2::policy::**throw_error**
<sub>defined in yorel::yomm2::policy by <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub>


    struct throw_error;

`throw_error` implements [`error_handler`](/reference/error_handler.md) by throwing errors as exceptions.

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
