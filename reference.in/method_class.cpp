/***
# method_class
entry: YOMM2_METHOD_CLASS
headers: yorel/yomm2/keywords.hpp

```c++
#define method_class(RETURN_TYPE, NAME, ARGS, ...)
#define YOMM2_METHOD_CLASS(RETURN_TYPE, NAME, ARGS, ...)
```

Both macros take the same arguments as ->`declare_method`, and return the
corresponding ->`method`.

## Example
***/


#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

namespace yomm2 = yorel::yomm2;
using yomm2::virtual_;

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(Animal, Dog);

declare_method(string, kick, (virtual_<Animal&>));

define_method(string, kick, (Dog& dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_method_class) {
    yomm2::update();

    Animal&& dog = Dog();
    BOOST_TEST(method_class(string, kick, (virtual_<Animal&>))::fn(dog) == "bark");
}
