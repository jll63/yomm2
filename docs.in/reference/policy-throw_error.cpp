/***
entry: policy::throw_error
location: policy;yorel/yomm2/core.hpp,yorel/yomm2.hpp

    struct throw_error;

`throw_error` implements ->`policy-error_handler` by throwing errors as
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
***/

#define BOOST_TEST_MODULE checked_perfect_hash
#include <boost/test/included/unit_test.hpp>

//***

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

namespace yomm2 = yorel::yomm2;
using yomm2::virtual_;

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

using throw_policy = yomm2::default_policy::replace<
    yorel::yomm2::policies::error_handler, yorel::yomm2::policies::throw_error>;

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
            error.method ==
            yomm2::type_id(
                &typeid(method_class(
                    void, kick, (virtual_<Animal&>), throw_policy))));
        BOOST_TEST(error.arity == 1);
        BOOST_TEST(error.types[0] == throw_policy::static_type<Dog>());
        threw = true;
    }

    BOOST_TEST(threw);
}

//***
