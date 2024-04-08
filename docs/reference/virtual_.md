yorel::yomm2::**virtual_**


<sub>defined in <yorel/yomm2/core.hpp>, also provided by<yorel/yomm2/keywords.hpp>, <yorel/yomm2.hpp></sub>
```
template<class C>
struct virtual_;
```
Mark a method parameter as virtual.

`type` must be a reference, a rvalue reference, a pointer or a
`std::shared_ptr` to a polymorphic type, possibly qualified with `const`.

## Examples


```c++
#include <yorel/yomm2/keywords.hpp>

struct Animal {
    virtual ~Animal() {}
};

declare_method(void, kick, (virtual_<Animal*>));
declare_method(void, kick, (virtual_<Animal&>));
declare_method(void, kick, (virtual_<Animal&&>));
declare_method(void, kick, (virtual_<std::shared_ptr<Animal>>));
declare_method(void, kick, (virtual_<const std::shared_ptr<Animal>&>));
declare_method(void, kick, (virtual_<const Animal*>));
declare_method(void, kick, (virtual_<const Animal&>));
declare_method(void, kick, (virtual_<std::shared_ptr<const Animal>>));
declare_method(void, kick, (virtual_<const std::shared_ptr<const Animal>&>));
```
