yorel::yomm2::**method_call_error**<br/>yorel::yomm2::**method_call_error_handler**<br/>yorel::yomm2::**set_method_call_error_handler**
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>
```
struct method_call_error {
    enum type { not_implemented = 0, ambiguous = 1 } code;
    std::string_view method_name;
};
using method_call_error_handler = void (*)(
    const method_call_error &error, size_t arity, const std::type_info* const tis[]);

method_call_error_handler set_method_call_error_handler(
    method_call_error_handler handler);
```This mechanism is **deprecated**. Please use the new [error handler
mechanism](set_error_handler.md) instead.

If a method call cannot be dispatched, an error handler is called with a
reference to a `method_call_error` structure, which contains the method's name
in an unspecified format, and a code identifying the error.

The default error handler terminates the program with `abort()`. A different
handler can be installed with `set_method_call_error_handler`, which returns the
previous handler. The handler *may not* return. It may throw an exception.
