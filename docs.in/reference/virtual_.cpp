#ifdef YOMM2_MD

entry: virtual_
headers: yorel/yomm2/core.hpp, yorel/yomm2.hpp, yorel/yomm2.hpp
```
template<class C>
struct virtual_;
```
Mark a method parameter as virtual.

`type` must be a reference, a rvalue reference, a pointer or a
`std::shared_ptr` to a polymorphic type, possibly qualified with `const`.

## Examples

#endif

int main() {}

#ifdef YOMM2_CODE

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/virtual_shared_ptr.hpp>

struct Animal {
    virtual ~Animal() {}
};

declare_method(poke, (virtual_<Animal&>), void);
declare_method(poke, (virtual_<Animal&&>), void);
declare_method(poke, (virtual_<std::shared_ptr<Animal>>), void);
declare_method(poke, (virtual_<const std::shared_ptr<Animal>&>), void);
declare_method(poke, (virtual_<const Animal&>), void);
declare_method(poke, (virtual_<std::shared_ptr<const Animal>>), void);
declare_method(poke, (virtual_<const std::shared_ptr<const Animal>&>), void);

#endif
