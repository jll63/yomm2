#ifdef YOMM2_MD

entry: aggregate
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
template<typename... T> struct aggregate;
```

An instance of `aggregate<T...>` contains one `T` sub-object for each
specified `T`, just like a `std::tuple`.
`aggregate` provides a convenient way to instantiate a collection of [YOMM2
registration objects](static_object.md). Typically, the name of the variable
does not matter, and ->YOMM2_GENSYM can be used to generated that single-use
identifier.
Unlike typical `std::tuple<typename... T>` implementations, `aggregate` can
handle large numbers of `T`s. For example, clang++-12 has a limit of 1024
mp_list, which can be reached easily when writing templatized method
definitions.
## Example

#endif

int main() {}

#ifdef YOMM2_CODE

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/templates.hpp>

using namespace yorel::yomm2;
using boost::mp11::mp_list;

struct Animal { virtual ~Animal() {} };
struct Dog : Animal {};
struct Cat : Animal {};

aggregate<
    class_declaration<mp_list<Animal>>,
    class_declaration<mp_list<Dog, Animal>>,
    class_declaration<mp_list<Cat, Animal>>
> YOMM2_GENSYM;

#endif
