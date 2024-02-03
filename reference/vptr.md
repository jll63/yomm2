
<sub>/ [home](/reference//README.md) / [reference](/reference//reference/README.md) </sub>

**yorel::yomm2::policy::vptr entry: yorel::yomm2::policy::external_vptr**<br>
**yorel::yomm2::policy::external_vptr_vector entry:**<br>
yorel::yomm2::policy::external_vptr_map headers: yorel/yomm2/core.hpp,
yorel/yomm2/keywords.hpp

---
```
struct vptr;

struct external_vptr;

template<class Policy> struct external_vptr_vector;

template<class Policy> struct external_vptr_map;
```
---
A `vptr` facet is provides a static function that returns a pointer to the
dispatch data for a virtual argument's dynamic class.

YOMM2 implements method dispatch in a way similar to native virtual function
dispatch: for each virtual argument, fetch a pointer to the dispatch data
(the v-table), and use it to select a pointer to a function. Method v-tables
contain pointers to functions for unary methods, and, for multi-methods,
pointers to, and coordinates in, a multi-dimensional table of pointers to
functions.

The `vptr` facet is used during method call to fetch the vptr for virtual
arguments corresponding to the `virtual_` parameters in the method
declaration. It is also used by the constructor of `virtual_ptr` to obtain a
vptr on the basis of an object's dynamic type.

`virtual_ptr::final`, and the related convenience functions, assume that the
static and dynamic types of their argument are the same. The vptr is obtained
statically from the policy's `static_vptr<Class>` member. It is conceivable
to organize an entire program around the "final" constructs; thus, the `vptr`
facet is optional.

`external_vptr` is a sub-category of `facet`. If present, the runtime calls
its static functions to allow it to initialize its data structures.

## Example

This example involves two hierarchies, `Person` (with one subclass, `Engineer`),
and `Number` (with two subclasses, `Integer` and `Rational`). Each hierarchy
collaborates with YOMM2 by providing the pointer to the vtable. The classes are
_not_ polymorphic.

In the `Person` hierarchy, the vptr is stored directly in the object[^1]. The
constructor initializes it with the appropriate vtable, obtained from
`basic_policy::vptr`.

In the `Number` hierarchy, objects are stored in pages, aligned on a 1024 byte
boundary. A page contains objects of the same type, either `Integer` or
`Rational`. The appropriate vptr is stored at the beginning of the page, which
can be found by applying a mask (`~1023`) to the `this` pointer.


```c++
#ifdef _MSC_VER
#include <malloc.h>
#else
#include <cstdlib>
#endif

#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace policy;

struct Person;
struct Number;

struct vptr_page : virtual default_static_policy::use_facet<vptr> {
    template<class QClass>
    static auto vptr(QClass& arg) {
        using Class = std::remove_const_t<QClass>;
        if constexpr (std::is_base_of_v<Number, Class>) {
            auto page = reinterpret_cast<std::uintptr_t>(&arg) & ~1023;
            return *reinterpret_cast<std::uintptr_t**>(page);
        } else if constexpr (std::is_base_of_v<Person, Class>) {
            return arg.vptr;
        } else {
            return default_static_policy::use_facet<policy::vptr>::vptr(arg);
        }
    }
};

struct custom_policy : default_static_policy::replace<vptr, vptr_page> {};

struct Number {};

struct Integer : Number {
    explicit Integer(int value) : value(value) {
    }

    int value;
};

struct Rational : Number {
    Rational(int a, int b) : num(a), den(b) {
    }

    int num, den;
};

struct Person {
    std::uintptr_t* vptr;

    Person() {
        vptr = custom_policy::static_vptr<Person>;
    }
};

struct Engineer : Person {
    Engineer() {
        vptr = custom_policy::static_vptr<Engineer>;
    }
};

template<class T>
class Page {
  private:
    static constexpr size_t page_size = 1024;
    char* base;
    T* top;

  public:
    Page() {
        base = reinterpret_cast<char*>(
#ifdef _MSC_VER
            _aligned_malloc(page_size, page_size)
#else
            std::aligned_alloc(page_size, page_size)
#endif
        );
        *reinterpret_cast<std::uintptr_t**>(base) =
            custom_policy::static_vptr<T>;
        void* first = base + sizeof(std::uintptr_t*);
        size_t space = page_size - sizeof(std::uintptr_t*);
        std::align(alignof(T), sizeof(T), first, space);
        top = reinterpret_cast<T*>(first);
    }

    ~Page() {
#ifdef _MSC_VER
        _aligned_free(base);
#else
        free(base);
#endif
    }

    template<typename... U>
    T& construct(U... args) {
        if (reinterpret_cast<char*>(top + 1) > base + page_size) {
            throw std::bad_alloc();
        }

        return *new (top++) T(std::forward<U>(args)...);
    }
};

#define YOMM2_DEFAULT_POLICY custom_policy
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

register_classes(Number, Integer, Rational, Person, Engineer);

#include <iostream>

declare_method(
    void, add,
    (virtual_<const Person&>, virtual_<const Number&>, virtual_<const Number&>,
     std::ostream&));

define_method(
    void, add,
    (const Person&, const Number& a, const Number& b, std::ostream& os)) {
    os << "I don't know how to do this...";
}

define_method(
    void, add,
    (const Person&, const Integer& a, const Integer& b, std::ostream& os)) {
    os << a.value << " + " << b.value << " = " << a.value + b.value;
}

define_method(
    void, add,
    (const Engineer&, const Rational& a, const Rational& b, std::ostream& os)) {
    os << a.num << "/" << a.den << " + " << b.num << "/" << b.den << " = "
       << a.num * b.den + a.den * b.num << "/" << a.den * b.den;
}

#include <sstream>

BOOST_AUTO_TEST_CASE(ref_vptr_page) {
    static_assert(sizeof(Integer) == sizeof(int));
    static_assert(sizeof(Rational) == 2 * sizeof(int));

    update<custom_policy>();

    Page<Integer> ints;
    const Number &i_2 = ints.construct(2), &i_3 = ints.construct(3);

    Page<Rational> rationals;
    const Number &r_2_3 = rationals.construct(2, 3),
           &r_3_4 = rationals.construct(3, 4);

    const Person& alice = Engineer();
    const Person& bob = Person();

    std::ostringstream out1, out2, out3;

    add(bob, i_2, i_3, out1);
    BOOST_TEST(out1.str() == "2 + 3 = 5");

    add(bob, r_2_3, r_3_4, out2);
    BOOST_TEST(out2.str() == "I don't know how to do this...");

    add(alice, r_2_3, r_3_4, out3);
    BOOST_TEST(out3.str() == "2/3 + 3/4 = 17/12");
}
```


[1]: This is similar to the way YOMM11 works.


