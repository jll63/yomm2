entry: policy::basic_error_output
headers: yorel/yomm2/policy.hpp,yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

```c++
template<class Policy, typename Stream = /*unspecified*/>
struct basic_error_output;
```

`basic_error_output` implements the ->`policy-error_output` facet.

## Template parameters

* **Policy**: The policy containing the facet. Since `basic_error_output` has
  static state, making the policy a template parameter ensures that each policy
  has its wn set of static member variables.

* **Stream**: `Stream` can be any type that satisfies the requirements of
  ->`RestrictedOutputStream`. The default value is a lightweight version of
  `std::ostream` that writes to `stderr`, using low-level C functions.

## Static member variables

| Name                                     | Value                  |
| ---------------------------------------- | ---------------------- |
| Stream [**error_stream**](#error_stream) | the stream to print to |

### error_stream

Initialized by the default constructor of `Stream`. It is the responsibility of
the program to perform further initialization if needed - for example, open a
`std::ofstream`, before calling `update`.
