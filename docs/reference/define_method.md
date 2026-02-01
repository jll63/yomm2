> **DEPRECATION NOTICE**<br>
> YOMM2 has been superseded by Boost.OpenMethod. See README for more details.





<span style="font-size:xx-large;"><strong>define_method</strong><br/></span><br/>
<sub>defined in <yorel/yomm2/cute.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub></sub><br/>

```c++
#define_method(/*unspecified*/) /*unspecified*/
```

### Usage

```
define_method(return-type, name, (method-parameter-list)) {
    ...
}

define_method(container, return-type, name, (method-parameter-list)) {
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
[set_method_call_error_handler](/yomm2/reference/method_call_error.html).


See the documentation of [declare_method](/yomm2/reference/declare_method.html) for information on handling types that
contain commas.

If `container` is specified, the method definition is placed inside the
container, which must have been declared with `method_container`. See the
documentation of [`method_container`](/yomm2/reference/method_container.html) for more information on method containers.

## Example


```c++
#include <string>
#include <yorel/yomm2/keywords.hpp>

struct Animal { virtual ~Animal() {} };
struct Dog : Animal {};
struct Bulldog : Dog {};

register_classes(Animal, Dog, Bulldog);

declare_method(std::string, kick, (virtual_<Animal*>));

define_method(std::string, kick, (Dog* dog)) {
    return "bark";
}

define_method(std::string, kick, (Bulldog* dog)) {
    return next(dog) + " and bite";
}

BOOST_AUTO_TEST_CASE(ref_example) {
    yorel::yomm2::update();

    Dog snoopy;
    Bulldog hector;
    Animal* animal;

    animal = &snoopy;
    BOOST_TEST(kick(animal) == "bark");

    animal = &hector;
    BOOST_TEST(kick(animal) == "bark and bite");
}
```
