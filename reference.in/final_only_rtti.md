# yorel::yomm2::policy::std_rtti
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

    struct std_rtti;

#include <iostream>
#include <vector>

struct Animal {
};

struct Cat : Animal {};

struct Dog : Animal {};

#undef YOMM2_DEFAULT_POLICY
#define YOMM2_DEFAULT_POLICY minimal_policy

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>
using namespace yorel::yomm2;

struct minimal_policy : policy::basic_policy<minimal_policy, policy::final_only_rtti> {};

template<class Class>
using vptr = virtual_ptr<Class, minimal_policy>;

register_classes(Animal, Dog, Cat);

declare_method(std::string, kick, (vptr<Animal>));

define_method(std::string, kick, (vptr<Cat> cat)) {
    return "hiss";
}

define_method(std::string, kick, (vptr<Dog> dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_rtti) {
    update<minimal_policy>();
    Cat felix;
    Dog snoopy;
    std::vector<vptr<Animal>> animals = {
        final_virtual_ptr<minimal_policy>(felix),
        final_virtual_ptr<minimal_policy>(snoopy),
    };

    for (auto& animal : animals) {
        kick(animal);
    }
}
