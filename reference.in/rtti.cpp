#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_MD

<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::vptr
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct rtti;
struct deferred_static_rtti;
struct std_rtti;
struct final_only_rtti;
```

A `rtti` facet provides type information for classes and objects, implements
downcast in presence of virtual inheritance, and writes descriptions of types to
an `ostream`-like object.

A `deferred_static_rtti`, derived from `rtti`, directs YOMM2 to defer collection
of static type information until `update` is called. This makes it possible to
interface with custom RTTI systems that use static constructors to assign type
information.

`std_rtti` implements `rtti` on top of standard C++ constructs (`typeid` and
`dynamic_cast`).

`final_only_rtti` is a minimal implementation that requires no RTTI. It can be
used only with methods that only have `virtual_ptr` virtual arguments, which
must be created using `virtual_ptr::final` or `final_virtual_ptr`.

### static member functions

|                                           |                                                 |                                             |
| ----------------------------------------- | ----------------------------------------------- | ------------------------------------------- |
| [static_type_id&lt;T>](#static_type_id)   | return a `type_id` for `T`                      | required                                    |
| [dynamic_type_id&lt;T>](#dynamic_type_id) | return a `type_id` for an object's dynamic type | not required for `virtual_ptr`              |
| [type_name&lt;Stream>](#type_name)        | write a description of `type_id` to a stream    | optional                                    |
| [type_index](#type_index)                 | return a unique key for a `type_id`             | optional                                    |
| [dynamic_cast_&lt;D, B>](#dynamic_cast_)  | cast from base class to derived class           | required in presence of virtual inheritance |


## Notes




#endif

#ifdef YOMM2_CODE

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

#endif

#ifdef YOMM2_MD

## See also
The [custom RTTI tutorial](custom_rtti_tutorial.md) for a full explanation
of these facets.

#endif
