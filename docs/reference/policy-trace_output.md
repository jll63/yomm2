> [!IMPORTANT]
> YOMM2 has been superseded by Boost.OpenMethod. See README for details.


<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>trace_output</strong></span><br/>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by <yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub><br/>

    struct trace_output;

The `trace_output` facet enables the YOMM2 runtime to print information about
the data structures used at dispatch time, and how they are derived. This
includes deduction of inheritance lattices, v-table slot allocation, dispatch
table construction, hash parameters, etc. This can help troubleshoot common
errors, like missing class registrations, ambiguities in method definitions,
etc.

The format of the information is not documented, beyond that it is useful. It
may change without notice.

**Requirements for implementations of `trace_output`**

|                                       |                              |
| ------------------------------------- | ---------------------------- |
| `static /*unspeficied*/ trace_stream` | a [`RestrictedOutputStream`](/yomm2/reference/RestrictedOutputStream.html) |
| `static bool trace_enabled`           | enable trace if `true`       |



**Implementations of `trace_output`**

|                               |                                       |
| ----------------------------- | ------------------------------------- |
| [`basic_trace_output`](/yomm2/reference/policy-basic_trace_output.html) | print to a `Stream` local to `Policy` |
