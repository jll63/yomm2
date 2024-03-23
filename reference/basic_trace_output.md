<sub>[home](/README.md) / [reference](/reference.md)</sub><br>
# yorel::yomm2::policy::**basic_trace_output**
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

    template<class Policy, typename Stream = /*unspecified*/>
    struct basic_trace_output;

`basic_trace_output` implements the `trace_output` facet.

## Template parameters

| Name                        | Value                               |
| --------------------------- | ----------------------------------- |
| class [**Policy**](#policy) | the policy containing the facet     |
| class [**Stream**](#stream) | a model of `RestrictedOutputStream` |

### Policy

The policy containing the facet. Since `basic_trace_output` has static state,
making the policy a template parameter ensures that each policy has its own set
of static member variables.

### Stream

`Stream` can be any type that satisfies the requirements of
[`RestrictedOutputStream`](/reference/RestrictedOutputStream.md). The default value is a lightweight version of
`std::ostream` that writes to `stderr`, using low-level C functions.

## Static member variables

| Name                                     | Value                   |
| ---------------------------------------- | ----------------------- |
| bool [**trace_enabled**](#trace_enabled) | enable or disable trace |
| Stream [**trace_stream**](trace_stream)  | the stream to print to  |

### yorel::yomm2::policy::**trace_enabled**

Controls whether information is printed to `trace_stream`. The flag is
initialized by examining the `YOMM2_TRACE` environment variable. If it is set,
and its value is `1`, trace is enabled. Other values are reserved for possible
future use.

### yorel::yomm2::policy::**trace_stream**

Initialized by the default constructor of `Stream`. It is the responsibility of
the program to perform further initialization if needed - for example, open a
`std::ofstream`, before calling `update`.
