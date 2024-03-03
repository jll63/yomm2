<sub>/ ->home / ->reference / ->vectored_error / ->vectored_error/error </sub>

# vectored_error<Policy>::**error**

---
```
static error_handler_type error = ;
```

A `std::function` containing a error handler. It is set to
->`vectored_error::default_error_handler`

Can be set to a user-provided handler.
