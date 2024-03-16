<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
## yorel::yomm2::policy::<big>**basic_error_output**</big>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    template<class Policy, typename Stream = /*unspecified*/
    struct basic_error_output;

`basic_error_output` is an implementation of `error_output` that provides one
static data member, `stream`. xxx

**Template parameters**

* `Stream`: a type that implements the subset of `std::ostream` required by
  the facet. The default value is a lightweight version of `std::stream` that
  writes to `stderr`, using low-level C functions.

**Static member variables**

|                                     |      |
| ----------------------------------- | ---- |
| [`error_stream`](/reference/basic_error_output/error_stream.md) | TODO |
