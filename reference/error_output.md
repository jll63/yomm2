<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
# yorel::yomm2::policy::**error_output**
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    struct error_output;

The `error_output` facet provides one static data member, `error_stream`, which
must support a small subset of the protocol of `std::ostream`. The following
insertion operators are required:

    Stream& operator<<(Stream& os, const std::string_view& view);
    Stream& operator<<(Stream& os, const void* value);
    Stream& operator<<(Stream& os, size_t value);

(where `Stream` is the type of the `error_stream` data member)

When an error is encountered, diagnostics are written to `error_stream` using these
operators.

**Requirements for implementations of `error_output`**

|                                       |                              |
| ------------------------------------- | ---------------------------- |
| `static /*unspeficied*/ error_stream` | a `std::ostream`-like object |

**Implementations of `error_output`**

|                                                               |                          |
| ------------------------------------------------------------- | ------------------------ |
| [`basic_error_output<Policy, Stream>`](basic_error_output.md) | provide a `error_stream` |
