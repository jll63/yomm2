# yorel::yomm2::policy::**basic_error_output**
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

    template<class Policy, typename Stream = /*unspecified*/>
    struct basic_error_output;

`basic_error_output` implements the `error_output` facet.

## Template parameters

| Name                        | Value                               |
| --------------------------- | ----------------------------------- |
| class [**Policy**](#policy) | the policy containing the facet     |
| class [**Stream**](#stream) | a model of `RestrictedOutputStream` |

### Policy

The policy containing the facet. Since `basic_error_output` has static state,
making the policy a template parameter ensures that each policy has its own set
of static member variables.

### Stream

`Stream` can be any type that satisfies the requirements of
->`RestrictedOutputStream`. The default value is a lightweight version of
`std::ostream` that writes to `stderr`, using low-level C functions.

## Static member variables

| Name                                     | Value                   |
| ---------------------------------------- | ----------------------- |
| Stream [**error_stream**](error_stream)  | the stream to print to  |

### yorel::yomm2::policy::**error_stream**

Initialized by the default constructor of `Stream`. It is the responsibility of
the program to perform further initialization if needed - for example, open a
`std::ofstream`, before calling `update`.
