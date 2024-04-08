

# yorel::yomm2::policy::minimal_rtti
<sub>defined in <yorel/yomm2/policy.hpp>, also provided by<yorel/yomm2/core.hpp>, <yorel/yomm2/keywords.hpp></sub>

```c++
struct minimal_rtti;
```

Minimal implementation of [`rtti`](/yomm2/reference/policy-rtti.html) that does not use any actual _runtime_
type information. This facet can be used in programs that call methods solely
via [`virtual_ptr`](/yomm2/reference/virtual_ptr.html)s created using the [final](/yomm2/reference/virtual_ptr.html) constructs. Virtual
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

Allocate a static `char` variable and return its address, cast to [`type_id`](/yomm2/reference/type_id.html).

**Template parameters**

* Class: a class registered via [register_classes](/yomm2/reference/use_classes.html) or [use_classes](/yomm2/reference/use_classes.html).

## Example


```c++
#include <string>
#include <vector>

#include <yorel/yomm2/keywords.hpp>
using namespace yorel::yomm2;

struct Animal {};
struct Cat : Animal {};
struct Dog : Animal {};

struct final_policy : policy::basic_policy<final_policy, policy::minimal_rtti> {
};

template<class Class>
using vptr = virtual_ptr<Class, final_policy>;

register_classes(Animal, Dog, Cat, final_policy);

declare_method(std::string, kick, (vptr<Animal>), final_policy);

define_method(std::string, kick, (vptr<Cat> cat)) {
    return "hiss";
}

define_method(std::string, kick, (vptr<Dog> dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_static_rtti) {
    update<final_policy>();

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
```
