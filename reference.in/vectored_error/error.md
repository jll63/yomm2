## yorel::yomm2::policy::vectored_error@<Policy>::**error**
headers: policy;yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

    static error_handler_type error;

A `std::function` containing a error handler. It is set to
->`vectored_error::default_error_handler` by ->`update`,  but it can be set to a
user-defined function, which may throw an exception to prevent program
termination.
