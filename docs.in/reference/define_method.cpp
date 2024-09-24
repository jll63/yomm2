#ifdef YOMM2_MD

hrefs: YOMM2_OVERRIDE

macro: define_method
headers: yorel/yomm2/cute.hpp, yorel/yomm2.hpp></sub

```c++
#define_method(/*unspecified*/) /*unspecified*/
```

### Usage

```
define_method(name, (method-parameter-list), return-type) {
    ...
}

define_method(return-type, name, container, (method-parameter-list)) {
    ...
}
```

Add a definition to a method.

Locate a method with the same name, with a signature compatible with
`method-parameter-list`, and add the definition to the method's list of
definitions. The method must exist and must be unique. `return-type` must be
compatible with the method's return type. `return-type` may be `auto`.

The types of the arguments must _not_ be decorated with `virtual_`.

Inside the block, a function pointer named `next` points to the next most
specific definition, if one exists, and it is unique. Otherwise, `next` points
to an error handler, which writes a message to `stderr`, and terminates the
program via `abort`. The handler can be customized. See
->set_method_call_error_handler.


See the documentation of ->declare_method for information on handling types that
contain commas.

If `container` is specified, the method definition is placed inside the
container, which must have been declared with `method_container`. See the
documentation of ->`method_container` for more information on method containers.

## Example

#endif

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_CODE

#include <string>
#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Animal { virtual ~Animal() {} };
struct Dog : Animal {};
struct Bulldog : Dog {};

register_classes(Animal, Dog, Bulldog);

declare_method(kick, (virtual_<Animal&>), std::string);

define_method(kick, (Dog& dog), std::string) {
    return "bark";
}

define_method(kick, (Bulldog& dog), std::string) {
    return next(dog) + " and bite";
}

BOOST_AUTO_TEST_CASE(ref_example) {
    yorel::yomm2::initialize();

    Dog snoopy;
    Bulldog hector;
    Animal* animal;

    animal = &snoopy;
    BOOST_TEST(kick(*animal) == "bark");

    animal = &hector;
    BOOST_TEST(kick(*animal) == "bark and bite");
}

#endif
