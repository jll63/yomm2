## yorel::yomm2::policy::**trace_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

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
| `static /*unspeficied*/ trace_stream` | a ->`RestrictedOutputStream` |
| `static bool trace_enabled`           | enable trace if `true`       |



**Implementations of `trace_output`**

|                               |                                       |
| ----------------------------- | ------------------------------------- |
| ->`policy-basic_trace_output` | print to a `Stream` local to `Policy` |
