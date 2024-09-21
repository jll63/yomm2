/***
macro: method_class
headers: yorel/yomm2.hpp
macro: YOMM2_METHOD_CLASS
headers: yorel/yomm2/macros.hpp

```c++
#define method_class(NAME, ARGS [, POLICY], RETURN_TYPE)
#define YOMM2_METHOD_CLASS(NAME, ARGS [, POLICY], RETURN_TYPE)
```

Both macros take the same arguments as ->`declare_method`, and return the
corresponding core type ->`method`.

## Example
***/

#define BOOST_TEST_MODULE checked_perfect_hash
#include <boost/test/included/unit_test.hpp>

#include <string>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(Animal, Dog);

declare_method(kick, (virtual_<Animal&>), std::string);

define_method(kick, (Dog & dog), std::string) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_method_class) {
    yorel::yomm2::initialize();

    Animal&& dog = Dog();
    using X = YOMM2_METHOD_CLASS(kick, (virtual_<Animal&>), std::string);
    auto reply = YOMM2_METHOD_CLASS(kick, (virtual_<Animal&>), std::string)::fn(dog);
    BOOST_TEST(reply == "bark");
}
