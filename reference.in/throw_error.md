<sub>/ ->home / ->reference </sub>

**yorel::yomm2::throw_error**<br>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

---
```
struct throw_error;
```

`throw_error` is an implementation of ->`error_handler` that throws the error as
an exception.

If exceptions are disabled, `throw_error` is not defined.

## static member functions

|       |                       |
| ----- | --------------------- |
| error | throw variant's value |

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
