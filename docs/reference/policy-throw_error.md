

# yorel::yomm2::policy::**throw_error**
<sub>defined in yorel::yomm2::policy by <yorel/yomm2/core.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub>


    struct throw_error;

`throw_error` implements [`error_handler`](/yomm2/reference/policy-error_handler.html) by throwing errors as
exceptions.

If exceptions are disabled, `throw_error` is not defined.

## Static member functions

|                                   |                                 |
| --------------------------------- | ------------------------------- |
| [**error**](#error) | throw the error variant's value |

### error

```c++
static void error(const error_type& error_variant);
```

Extract the value of `error_variant`, and throw it as an exception.

## Example

```c++
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

namespace yomm2 = yorel::yomm2;
using yomm2::virtual_;

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

using throw_policy = yomm2::default_policy::replace<
    yomm2::policy::error_handler, yomm2::policy::throw_error>;

register_classes(Animal, Dog, throw_policy);

declare_method(void, kick, (virtual_<Animal&>), throw_policy);

BOOST_AUTO_TEST_CASE(ref_throw_error) {
    yomm2::update<throw_policy>();

    bool threw = false;

    try {
        Animal&& dog = Dog();
        kick(dog);
    } catch (yomm2::resolution_error& error) {
        BOOST_TEST(error.status == yomm2::resolution_error::no_definition);
        BOOST_TEST(
            error.method_name ==
            typeid(method_class(void, kick, (virtual_<Animal&>), throw_policy))
                .name());
        BOOST_TEST(error.arity == 1);
        BOOST_TEST(error.types[0] == throw_policy::static_type<Dog>());
        threw = true;
    }

    BOOST_TEST(threw);
}
```
