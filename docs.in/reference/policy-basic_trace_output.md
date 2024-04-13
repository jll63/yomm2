entry: policy::**basic_trace_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

```c++
    template<class Policy, typename Stream = /*unspecified*/>
    struct basic_trace_output;
```

`basic_trace_output` implements the ->`policy-trace_output` facet.

## Template parameters

* **Policy**: the policy containing the facet. Since `basic_trace_output` has
  static state, making the policy a template parameter ensures that each policy
  has its own set of static member variables.

* **Stream**: - `Stream` can be any type that satisfies the requirements of
  ->`RestrictedOutputStream`. The default value is a lightweight version of
  `std::ostream` that writes to `stderr`, using low-level C functions.

## Static member variables

| Name                                     | Value                   |
| ---------------------------------------- | ----------------------- |
| bool [**trace_enabled**](#trace_enabled) | enable or disable trace |
| Stream [**trace_stream**](#trace_stream) | the stream to print to  |

### trace_enabled

Controls whether information is printed to `trace_stream`. The flag is
initialized by examining the `YOMM2_TRACE` environment variable. If it is set,
and its value is `1`, trace is enabled. Other values are reserved for possible
future use.

### trace_stream

Initialized by the default constructor of `Stream`. It is the responsibility of
the program to perform further initialization if needed - for example, open a
`std::ofstream`, before calling `update`.
