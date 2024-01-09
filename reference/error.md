

<sub>/ [home](/reference//README.md) / [reference](/reference//reference/README.md) </sub>

**yorel::yomm2::policy::vptr**<br>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>

---
```
template<class Policy>
struct facet;
```
---
If the facet is present, it provides one static member: `error`, either a
function or a functor that takes a [`error_type`](/reference/set_error_handler.md) variant. It is called in the
following situations:

* While building dispatch tables (`update`), a class that has not been
  registered is used as a virtual parameter in a method declaration or
  definition.
* During a method call, no definition is available for a combination of virtual
  arguments, or more than one is available and none is more specialized than all
  the others.
* A facet encounters an error.

The function is allowed to throw an exception, `exit`, or anything it sees fit.
If it returns, the program is terminated with `abort`.

YOMM2 provides three implementations of the facet: `vectored_error_handler`. It
calls the function specified in the `error` static member variable of the facet
- a `std::function<void(const error_type& error)>` -, passing it the  error
variant.


