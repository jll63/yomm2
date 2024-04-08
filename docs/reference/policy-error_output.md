
## yorel::yomm2::policy::**error_output**
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    struct error_output;

The `error_output` facet is used to print diagnostics about error conditions.

**Requirements for implementations of `error_output`**

|                                       |                              |
| ------------------------------------- | ---------------------------- |
| `static /*unspeficied*/ error_stream` | a [`RestrictedOutputStream`](/yomm2/reference/RestrictedOutputStream.html) |

**Implementations of `error_output`**

|                             |                                           |
| --------------------------- | ----------------------------------------- |
| [basic_error_output](/yomm2/reference/policy-basic_error_output.html) | print to a `Stream` local to the `Policy` |
