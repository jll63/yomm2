#ifdef YOMM2_MD

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::virtual_
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp, yorel/yomm2.hpp

---
```
template<class C>
struct virtual_;
```
---

Mark a method parameter as virtual.

`type` must be a reference, a rvalue reference, a pointer or a
`std::shared_ptr` to a polymorphic type, possibly qualified with `const`.

## examples

#endif

int main() {}

#ifdef YOMM2_CODE

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

#endif
