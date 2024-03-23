## yorel::yomm2::policy::**basic_error_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    template<class Policy, typename Stream = /*unspecified*/>
    struct basic_error_output;

`basic_error_output` is an implementation of `error_output` that provides a
`error_stream` static member local to `Policy`.

`Stream` can be any type that satisfies the requirements of
`RestrictedOutputStream` - e.g., a `std::ostringstream`. The default value is a
lightweight version of `std::ostream` that writes to `stderr`, using low-level C
functions.

**Template parameters**

* `Stream`: a model of ->`RestrictedOutputStream`.

**Static member variables**

|                                     |      |
| ----------------------------------- | ---- |
| ->`basic_error_output/error_stream` | TODO |
