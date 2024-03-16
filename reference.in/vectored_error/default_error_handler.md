## yorel::yomm2::policy::vectored_error@<Policy>::**default_error_handler**
headers:yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

    static void default_error_handler(const error_type& error);

If ->`error_output` is available in `Policy`, use it to print a description of
`error`. Return normally, causing the program to be aborted.

**Parameters**

* error - a variant containing the error.

**Return value**

(none)
