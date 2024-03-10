<sub>[home](/README.md) / [reference](/reference.md) / [vectored_error](/reference/vectored_error.md)</sub><br>

# vectored_error<Policy>::**default_error_handler**
<sub>defined in yorel::yomm2::policy by <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub>

---
```
static void default_error_handler(const error_type& error);
```

If ->error_output is available in `Policy`, use it to print a description of
`error`. Return normally, causing the program to be aborted by the caller.

**Parameters**

**error** - a variant containing the error.

**Return value**

(none)
