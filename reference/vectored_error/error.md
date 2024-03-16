<sub>[home](/README.md) / [reference](/reference.md) / [vectored_error](/reference/vectored_error.md)</sub><br>
## yorel::yomm2::policy::vectored_error&lt;Policy>::**error**
<sub>defined in <policy;yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

    static error_handler_type error;

A `std::function` containing a error handler. It is set to
[`vectored_error::default_error_handler`](/reference/vectored_error/default_error_handler.md) by [`update`](/reference/update.md),  but it can be set to a
user-defined function, which may throw an exception to prevent program
termination.
