<sub>[home](/README.md) / [reference](/reference.md) / [throw_error](/reference/throw_error.md)</sub><br>
## yorel::yomm2::policy::throw_error::**error**
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

    static void error(const error_type& error_variant);

Extract the value of `error_variant`, and throw it as an exception.

If exceptions are disabled, `throw_error` is not defined.

Can be set to a user-provided handler.

**Parameters**

* error_variant - a variant containing the error.

**Return value**

None.

**Errors**

None.
