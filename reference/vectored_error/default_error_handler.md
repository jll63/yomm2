<sub>[home](/README.md) / [reference](/reference.md) / [vectored_error](/reference/vectored_error.md)</sub><br>
## yorel::yomm2::policy::vectored_error&lt;Policy>::**default_error_handler**
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

    static void default_error_handler(const error_type& error);

If [`error_output`](/reference/error_output.md) is available in `Policy`, use it to print a description of
`error`. Return normally, causing the program to be aborted.

**Parameters**

* error - a variant containing the error.

**Return value**

(none)
