/***
entry: policies::minimal_rtti
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2.hpp

```c++
struct minimal_rtti;
```

Minimal implementation of ->`policy-rtti` that does not use any actual _runtime_
type information. This facet can be used in programs that call methods solely
via ->`virtual_ptr`s created using the ->virtual_ptr-final constructs. Virtual
inheritance is not supported. Classes are not required to be polymorphic.

## Static member functions

| Name                                          | Description                                     |
| --------------------------------------------- | ----------------------------------------------- |
| type_id [**static_type\<T>**](#static_type)   | return a `type_id` for `T`                      |

### static_type

```c++
template<class Class>
static type_id static_type();
```

Allocate a static `char` variable and return its address, cast to ->`type_id`.

**Template parameters**

* Class: a class registered via ->register_classes or ->use_classes.

## Example

***/

#define BOOST_TEST_MODULE checked_perfect_hash
#include <boost/test/included/unit_test.hpp>

//***
#include <string>
#include <vector>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>
using namespace yorel::yomm2;

struct Animal {};
struct Cat : Animal {};
struct Dog : Animal {};

struct final_policy
    : policies::basic_policy<final_policy, policies::minimal_rtti> {};

template<class Class>
using vptr = virtual_ptr<Class, final_policy>;

register_classes(Animal, Dog, Cat, final_policy);

declare_method(kick, (vptr<Animal>), std::string, final_policy);

define_method(kick, (vptr<Cat> cat), std::string) {
    return "hiss";
}

define_method(kick, (vptr<Dog> dog), std::string) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_static_rtti) {
    initialize<final_policy>();

    Cat felix;
    Dog snoopy;

    // type erasure:
    std::vector<vptr<Animal>> animals = {
        vptr<Animal>::final(felix),
        vptr<Animal>::final(snoopy),
    };

    BOOST_TEST(kick(animals[0]) == "hiss");
    BOOST_TEST(kick(animals[1]) == "bark");
}
//***
