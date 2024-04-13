entry: method_call_error
entry: method_call_error_handler
entry: set_method_call_error_handler
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
struct method_call_error {
    enum type { not_implemented = 0, ambiguous = 1 } code;
    std::string_view method_name;
};
using method_call_error_handler = void (*)(
    const method_call_error &error, size_t arity, const std::type_info* const tis[]);

method_call_error_handler set_method_call_error_handler(
    method_call_error_handler handler);
```

This mechanism is **deprecated**. Please use the new [error handler
mechanism](set_error_handler.md) instead.

If a method call cannot be dispatched, an error handler is called with a
reference to a `method_call_error` structure, which contains the method's name
in an unspecified format, and a code identifying the error.

The default error handler terminates the program with `abort()`. A different
handler can be installed with `set_method_call_error_handler`, which returns the
previous handler. The handler *may not* return. It may throw an exception.
