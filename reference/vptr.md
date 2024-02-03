
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

Virtual functions are sometimes criticized for having a non-zero overhead.
Consider the following class hierarchy:


```c++
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
```
 Making these classes polymorphic would double the size of `Integer`, and
increase the size of `Rational` by 50%.

A possible solution is to allocate objects in homogeneous pages, and store the
pointer to the v-table at the beginning of each page. It is thus shared between
a large number of objects. To make it easy to find the beginning of the page, we
allocate the pages on a 1024 byte boundary.

For this, we create a new `vptr` facet, which checks if the object is derived
from `Number`. If yes, it locates the base of the page (`&obj & ~1023`) and
returns the vptr stored there. Otherwise, it falls back to the default behavior,
so we can have both `Number` and normal, polymorphic classes as parameters in
multi-methods.


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

struct vptr_page : virtual default_static_policy::use_facet<vptr> {
    template<class Class>
    static auto dynamic_vptr(Class& arg) {
        if constexpr (std::is_base_of_v<Number, std::remove_const_t<Class>>) {
            auto page = reinterpret_cast<std::uintptr_t>(&arg) & ~1023;
            return *reinterpret_cast<std::uintptr_t**>(page);
        } else {
            return default_static_policy::use_facet<vptr>::dynamic_vptr(arg);
        }
    }
};

struct number_aware_policy : default_static_policy::replace<vptr, vptr_page> {};

// Make it the default policy.
#define YOMM2_DEFAULT_POLICY number_aware_policy
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>
```
 Here is a bare-bone implementation of the page allocator: 
```c++
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
            number_aware_policy::static_vptr<T>;
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
```


Let's create a couple of "ordinary" classes, register all the classes, and
define a method:


```c++
struct Person {
    virtual ~Person() {
    }
};

struct Engineer : Person {};

register_classes(Number, Integer, Rational, Person, Engineer);

#include <iosfwd>

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
    (const Engineer&, const Integer& a, const Rational& b, std::ostream& os)) {
    os << a.value << " + " << b.num << "/" << b.den << " = "
       << a.value * b.den + b.num << "/" << b.den;
}

define_method(
    void, add,
    (const Engineer&, const Rational& a, const Integer& b, std::ostream& os)) {
    os << a.num << "/" << a.den << " + " << b.value << " = "
       << a.num + b.value * a.den << "/" << a.den;
}

define_method(
    void, add,
    (const Engineer&, const Rational& a, const Rational& b, std::ostream& os)) {
    os << a.num << "/" << a.den << " + " << b.num << "/" << b.den << " = "
       << a.num * b.den + a.den * b.num << "/" << a.den * b.den;
}
```

And now let's test:

```c++
#include <sstream>

BOOST_AUTO_TEST_CASE(ref_vptr_page) {
    // Check for zero overhead.
    static_assert(sizeof(Integer) == sizeof(int));
    static_assert(sizeof(Rational) == 2 * sizeof(int));

    // Call update(). This must be done before we create any Numbers.
    update<number_aware_policy>();

    // Allocate a few Integers...
    Page<Integer> ints;
    const Number &i_2 = ints.construct(2), &i_3 = ints.construct(3);

    // ...and a few Rationals.
    Page<Rational> rationals;
    const Number &r_2_3 = rationals.construct(2, 3),
                 &r_3_4 = rationals.construct(3, 4);

    const Person& alice = Engineer();
    const Person& bob = Person();
    std::ostringstream out1, out2, out3, out4; // to capture results

    add(bob, i_2, i_3, out1);
    BOOST_TEST(out1.str() == "2 + 3 = 5");

    add(bob, r_2_3, r_3_4, out2);
    BOOST_TEST(out2.str() == "I don't know how to do this...");

    add(alice, r_2_3, r_3_4, out3);
    BOOST_TEST(out3.str() == "2/3 + 3/4 = 17/12");

    add(alice, i_2, r_2_3, out4);
    BOOST_TEST(out4.str() == "2 + 2/3 = 8/3");
}
```


[1]: This is similar to the way YOMM11 works.


