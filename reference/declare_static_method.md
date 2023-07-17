

<sub>/ [home](/README.md) / [reference](/reference/README.md) </sub>

**declare_static_method**<br>
<sub>defined in <yorel/yomm2/cute.hpp>, also provided by<yorel/yomm2/keywords.hpp></sub>


---
```
#define declare_static_method(return-type, name, (types)) /*unspecified*/
```
---
Declare a method as a static member in a `struct` or `class`. Otherwise, the
macro does exactly the same things as [declare_method](declare_method.md).

This macro can be used when Argument Dependent Lookup is considered Evil. Note
that there is no need for a static version of [define_method](define_method.md), since it does not
create functions that can be picked via ADL.

## example


```c++
#include <string>

#include <yorel/yomm2/keywords.hpp>

struct Engineer { virtual ~Engineer() {} };
struct SeniorEngineer : Engineer {};

register_classes(Engineer, SeniorEngineer);

declare_method(std::string, speak, (virtual_<const Engineer&>));

define_method(std::string, speak, (const Engineer&)) {
  return "engineers love ADL";
}

define_method(std::string, speak, (const SeniorEngineer& engineer)) {
  return "senior " + next(engineer);
}

struct no_adl {
    declare_static_method(std::string, speak, (virtual_<const Engineer&>));
};

define_method(std::string, no_adl::speak, (const Engineer&)) {
  return "engineers hate ADL";
}

define_method(std::string, no_adl::speak, (const SeniorEngineer& engineer)) {
  return "senior " + next(engineer);
}

BOOST_AUTO_TEST_CASE(reference_love_adl) {
    yorel::yomm2::update();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(speak(engineer) == "engineers love ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(speak(engineer) == "senior engineers love ADL");
    }
}

BOOST_AUTO_TEST_CASE(reference_hate_adl) {
    yorel::yomm2::update();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(no_adl::speak(engineer) == "engineers hate ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(no_adl::speak(engineer) == "senior engineers hate ADL");
    }
}
```
