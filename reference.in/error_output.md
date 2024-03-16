# yorel::yomm2::policy::**error_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    struct error_output;

The `error_output` facet provides one static data member, `stream`, which
must support a small subset of the protocol of `std::stream`. The following
insertion operators are required:

    Stream& operator<<(Stream& os, const std::string_view& view);
    Stream& operator<<(Stream& os, const void* value);
    Stream& operator<<(Stream& os, size_t value);

(where `Stream` is the type of the `stream` data member)

When an error is encountered, diagnostics are written to `stream` using these
operators.

**Requirements for implementations of `error_output`**

|                                 |                              |
| ------------------------------- | ---------------------------- |
| `static /*unspeficied*/ stream` | a `std::ostream`-like object |

**Implementations of `error_output`**

|                                                               |                    |
| ------------------------------------------------------------- | ------------------ |
| [`basic_error_output<Policy, Stream>`](basic_error_output.md) | provide a `stream` |
