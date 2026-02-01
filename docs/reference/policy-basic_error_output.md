> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.

<span style="font-size:xx-large;">yorel::yomm2::policy::<strong>basic_error_output</strong></span><br/>
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by <yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub><br/>

```c++
template<class Policy, typename Stream = /*unspecified*/>
struct basic_error_output;
```

`basic_error_output` implements the [`error_output`](/yomm2/reference/policy-error_output.html) facet.

## Template parameters

* **Policy**: The policy containing the facet. Since `basic_error_output` has
  static state, making the policy a template parameter ensures that each policy
  has its wn set of static member variables.

* **Stream**: `Stream` can be any type that satisfies the requirements of
  [`RestrictedOutputStream`](/yomm2/reference/RestrictedOutputStream.html). The default value is a lightweight version of
  `std::ostream` that writes to `stderr`, using low-level C functions.

## Static member variables

| Name                                     | Value                  |
| ---------------------------------------- | ---------------------- |
| Stream [**error_stream**](#error_stream) | the stream to print to |

### error_stream

Initialized by the default constructor of `Stream`. It is the responsibility of
the program to perform further initialization if needed - for example, open a
`std::ofstream`, before calling `update`.
