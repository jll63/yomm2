#ifdef YOMM2_MD

macro: declare_static_method
headers: yorel/yomm2/cute.hpp, yorel/yomm2.hpp
hrefs: YOMM2_STATIC_DECLARE

```c++
#define declare_static_method(name, (types), return-type) /*unspecified*/
```

Declare a method as a static member in a `struct` or `class`. Otherwise, the
macro does exactly the same things as ->declare_method.

This macro can be used when Argument Dependent Lookup is considered Evil. Note
that there is no need for a static version of ->define_method, since it does not
create functions that can be picked via ADL.

## Example

#endif

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_CODE

#include <string>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Engineer { virtual ~Engineer() {} };
struct SeniorEngineer : Engineer {};

register_classes(Engineer, SeniorEngineer);

declare_method(speak, (virtual_<const Engineer&>), std::string);

define_method(speak, (const Engineer&), std::string) {
  return "engineers love ADL";
}

define_method(speak, (const SeniorEngineer& engineer), std::string) {
  return "senior " + next(engineer);
}

struct no_adl {
    declare_static_method(speak, (virtual_<const Engineer&>), std::string);
};

define_method(no_adl::speak, (const Engineer&), std::string) {
  return "engineers hate ADL";
}

define_method(no_adl::speak, (const SeniorEngineer& engineer), std::string) {
  return "senior " + next(engineer);
}

BOOST_AUTO_TEST_CASE(ref_love_adl) {
    yorel::yomm2::initialize();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(speak(engineer) == "engineers love ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(speak(engineer) == "senior engineers love ADL");
    }
}

BOOST_AUTO_TEST_CASE(ref_hate_adl) {
    yorel::yomm2::initialize();

    {
        const Engineer &engineer = Engineer();
        BOOST_TEST(no_adl::speak(engineer) == "engineers hate ADL");
    }

    {
        const Engineer &engineer = SeniorEngineer();
        BOOST_TEST(no_adl::speak(engineer) == "senior engineers hate ADL");
    }
}

#endif
