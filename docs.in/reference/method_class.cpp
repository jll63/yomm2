/***
macro: method_class
headers: yorel/yomm2.hpp
macro: YOMM2_METHOD_CLASS
headers: yorel/yomm2/macros.hpp

```c++
#define method_class(RETURN_TYPE, NAME, ARGS [, POLICY])
#define YOMM2_METHOD_CLASS(RETURN_TYPE, NAME, ARGS [, POLICY])
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

declare_method(std::string, kick, (virtual_<Animal&>));

define_method(std::string, kick, (Dog & dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_method_class) {
    yorel::yomm2::update();

    Animal&& dog = Dog();
    using X = YOMM2_METHOD_CLASS(std::string, kick, (virtual_<Animal&>));
    auto reply = YOMM2_METHOD_CLASS(std::string, kick, (virtual_<Animal&>))::fn(dog);
    BOOST_TEST(reply == "bark");
}
