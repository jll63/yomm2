// md<
// <sub>/ ->home / ->reference </sub>
// ## yorel::yomm2::virtual_
// <sub>defined in headers <yorel/yomm2/core.hpp>, also provided by
// <yorel/yomm2/keywords.hpp> and <yorel/yomm2.hpp></sub>
// <!-- -->
// ---
// template\<class C\>\
// struct virtual_;
// <!-- -->
// ---

// Mark a method parameter as virtual.

// `type` must be a reference, a rvalue reference, a pointer or a
// `std::shared_ptr` to a polymorphic type, possibly qualified with `const`.

// ## examples

// >

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

// code<
#include <yorel/yomm2/keywords.hpp>

struct Animal {
    virtual ~Animal() {}
};

declare_method(void, kick, (virtual_<Animal*>));
declare_method(void, kick, (virtual_<Animal&>));
declare_method(void, kick, (virtual_<Animal&&>));
declare_method(void, kick, (virtual_<std::shared_ptr<Animal>>));
declare_method(void, kick, (virtual_<const std::shared_ptr<Animal>&>));
declare_method(void, kick, (virtual_<const Animal*>));
declare_method(void, kick, (virtual_<const Animal&>));
declare_method(void, kick, (virtual_<std::shared_ptr<const Animal>>));
declare_method(void, kick, (virtual_<const std::shared_ptr<const Animal>&>));
// >

BOOST_AUTO_TEST_CASE(test) {
}
