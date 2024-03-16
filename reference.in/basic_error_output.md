## yorel::yomm2::policy::**basic_error_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    template<class Policy, typename Stream = /*unspecified*/>
    struct basic_error_output;

`basic_error_output` is an implementation of `error_output` that provides one
static data member, `stream`.

**Template parameters**

* `Stream`: a type that implements the subset of `std::ostream` required by
  the facet. The default value is a lightweight version of `std::stream` that
  writes to `stderr`, using low-level C functions.

**Static member variables**

|                                     |                       |
| ----------------------------------- | --------------------- |
| ->`basic_error_output/error_stream` | current error handler |
