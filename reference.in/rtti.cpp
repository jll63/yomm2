#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_MD

<sub> /->home /
           ->reference</ sub>

               entry : yorel::yomm2::policy::vptr headers : yorel /
    yomm2 / core.hpp,
    yorel / yomm2 /
        keywords.hpp

        -- -
``` template<class Policy>
    struct facet;
```
---

#endif

#ifdef YOMM2_MD

## See also
The [custom RTTI tutorial](custom_rtti_tutorial.md) for a full explanation
of these facets.
#endif

#include <iostream>
#include <vector>

struct Animal {
};

struct Cat : Animal {};

struct Dog : Animal {};

struct minimal_policy;

#undef YOMM2_DEFAULT_POLICY
#define YOMM2_DEFAULT_POLICY minimal_policy

#include <yorel/yomm2/keywords.hpp>
using namespace yorel::yomm2;

struct minimal_policy : policy::basic_policy<minimal_policy, policy::rtti> {
    template<typename T>
    static type_id static_type() {
        return reinterpret_cast<type_id>(static_vptr<T>);
    }
};

register_classes(Animal, Dog, Cat);

declare_method(std::string, kick, (virtual_ptr<Animal>));

define_method(std::string, kick, (virtual_ptr<Cat> cat)) {
    return "hiss";
}

define_method(std::string, kick, (virtual_ptr<Dog> dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(ref_rtti) {
    update<minimal_policy>();
    Cat felix;
    Dog snoopy;
    std::vector<virtual_ptr<Animal>> animals = {
        final_virtual_ptr(felix),
        final_virtual_ptr(snoopy),
    };

    for (auto& animal : animals) {
        kick(animal);
    }
}
