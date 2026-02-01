> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.



<span style="font-size:xx-large;">yorel::yomm2::<strong>aggregate</strong></span><br/>
<sub>defined in <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

```c++
template<typename... T> struct aggregate;
```

An instance of `aggregate<T...>` contains one `T` sub-object for each
specified `T`, just like a `std::tuple`.
`aggregate` provides a convenient way to instantiate a collection of [YOMM2
registration objects](static_object.md). Typically, the name of the variable
does not matter, and [YOMM2_GENSYM](/yomm2/reference/YOMM2_GENSYM.html) can be used to generated that single-use
identifier.
Unlike typical `std::tuple<typename... T>` implementations, `aggregate` can
handle large numbers of `T`s. For example, clang++-12 has a limit of 1024
types, which can be reached easily when writing templatized method
definitions.
## Example


```c++
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/templates.hpp>

using namespace yorel::yomm2;

struct Animal { virtual ~Animal() {} };
struct Dog : Animal {};
struct Cat : Animal {};

aggregate<
    class_declaration<types<Animal>>,
    class_declaration<types<Dog, Animal>>,
    class_declaration<types<Cat, Animal>>
> YOMM2_GENSYM;
```
