entry: policy::error_output
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    struct error_output;

The `error_output` facet is used to print diagnostics about error conditions.

**Requirements for implementations of `error_output`**

|                                       |                              |
| ------------------------------------- | ---------------------------- |
| `static /*unspeficied*/ error_stream` | a ->`RestrictedOutputStream` |

**Implementations of `error_output`**

|                             |                                           |
| --------------------------- | ----------------------------------------- |
| ->policy-basic_error_output | print to a `Stream` local to the `Policy` |
