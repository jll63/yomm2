<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
## yorel::yomm2::policy::**error_output**
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    struct error_output;

The `error_output` facet is used to print diagnostics about error conditions.

**Requirements for implementations of `error_output`**

|                                       |                              |
| ------------------------------------- | ---------------------------- |
| `static /*unspeficied*/ error_stream` | a [`RestrictedOutputStream`](/reference/RestrictedOutputStream.md) |

**Implementations of `error_output`**

|                                                               |                                           |
| ------------------------------------------------------------- | ----------------------------------------- |
| [`basic_error_output<Policy, Stream>`](basic_error_output.md) | print to a `Stream` local to the `Policy` |
