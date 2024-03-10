<sub>/ [home](/README.md) / [reference](/reference/reference.md) / [vectored_error](/reference/vectored_error.md) / [error](/reference/vectored_error/error.md) </sub>

# vectored_error<Policy>::**error**

---
```
static error_handler_type error = ;
```

A `std::function` containing a error handler. It is set to
[`vectored_error::default_error_handler`](/reference/vectored_error/default_error_handler.md)

Can be set to a user-provided handler.
