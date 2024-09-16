#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

/***
entry: policy::vptr_placement, policy::external_vptr
headers: yorel/yomm2/policy.hpp, yorel/yomm2.hpp
```
struct vptr_placement;
struct external_vptr;
```
The `vptr_placement` facet is responsible for retrieving a pointer to the v-table for an
object.

YOMM2 implements method dispatch in a way similar to native virtual function
dispatch: for each virtual argument, fetch a pointer to the dispatch data (known
as the v-table), and use it to select a pointer to a function. YOMM2 v-tables
contain pointers to functions for unary methods, and, for multi-methods,
pointers to, and coordinates in, a multi-dimensional table of pointers to
functions.

The `vptr_placement` facet is used during method call to fetch the vptr_placement for virtual
arguments corresponding to the `virtual_` parameters in the method
declaration. It is also used by the constructor of `virtual_ptr` to obtain a
vptr_placement on the basis of an object's dynamic type.

`virtual_ptr::final`, and the related convenience functions, assume that the
static and dynamic types of their argument are the same. The vptr_placement is obtained
statically from the policy's `static_vptr<Class>` member. It is conceivable
to organize an entire program around the "final" constructs; thus, the `vptr_placement`
facet is optional.

`external_vptr` is a sub-category of `facet`. If present, it provides a
`publish_vptrs` function, called by `update`.

### Requirements for implementations of `vptr_placement`

An implementation of `vptr_placement` must provide the following static function template:

| Name                          | Description                                     |
| ----------------------------- | ----------------------------------------------- |
| [dynamic_vptr](#dynamic_vptr) | return the address of the v-table for an object |


### dynamic_vptr

```c++
template<class Class>
static const std::uintptr_t* dynamic_vptr(const Class& arg);
```

## Requirements for implementations of `external_vptr`

In addition to the requirements for `vptr_placement`, an implementation of `external_vptr`
must provide the following static function template:

| Name                            | Description    |
| ------------------------------- | -------------- |
| [publish_vptrs](#publish_vptrs) | initialization |

### publish_vptrs

```c++
template<typename ForwardIterator>
static void publish_vptrs(ForwardIterator first, ForwardIterator last);
```

Called by `update` with a range of ->`RuntimeClass` objects, after the v-tables
have been set up.

### Implementations of `external_vptr`

| Name                              | Description                                                |
| -------------------- | ----------------------------------------- |
| ->policy-vptr_map    | store the vptrs in a `std::unordered_map` |
| ->policy-vptr_vector | store the vptrs in a `std::vector`        |

## Example

Virtual functions are sometimes criticized for having a non-zero overhead.
Consider the following class hierarchy:

***/

//***
#include <iosfwd>

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
//***

/***
Making these classes polymorphic would double the size of `Integer`, and
increase the size of `Rational` by 50%.

A possible solution is to allocate objects of the same class as pages, and store
the pointer to the v-table at the beginning of each page. It is thus shared by a
large number of objects. To make it easy to find the beginning of the page, we
allocate the pages on a 1024 byte boundary (or some other power of two).

For this, we create a new `vptr_placement` facet, which checks if the object is
derived from `Number`. If yes, it locates the base of the page (`&obj & ~1023`)
and returns the vptr_placement stored there. Otherwise, it falls back to the
default behavior, so we can have both `Number` and normal, polymorphic classes
as parameters in multi-methods.
***/

#ifdef _MSC_VER
#include <malloc.h>

namespace std {
void* aligned_alloc(std::size_t alignment, std::size_t size) {
    return _aligned_malloc(size, alignment);
}
} // namespace std

#define free _aligned_free
#endif

//***
#include <cstdlib>

#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace policies;

struct vptr_page : virtual default_policy::use_facet<vptr_placement> {
    template<class Class>
    static auto dynamic_vptr(Class& arg) {
        if constexpr (std::is_base_of_v<Number, std::remove_const_t<Class>>) {
            auto page = reinterpret_cast<std::uintptr_t>(&arg) & ~1023;
            return *reinterpret_cast<std::uintptr_t**>(page);
        } else {
            return default_policy::use_facet<vptr_placement>::dynamic_vptr(arg);
        }
    }
};

struct number_aware_policy
    : default_policy::replace<vptr_placement, vptr_page> {};

// Make it the default policy.
#define YOMM2_DEFAULT_POLICY number_aware_policy
#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

//***

/***
 Here is a bare-bone implementation of the page allocator:
***/

//***

template<class T>
class Page {
  private:
    static constexpr std::size_t page_size = 1024;
    char* base;
    T* top;

  public:
    Page() {
        base =
            reinterpret_cast<char*>(std::aligned_alloc(page_size, page_size));
        *reinterpret_cast<std::uintptr_t**>(base) =
            number_aware_policy::static_vptr<T>;
        void* first = base + sizeof(std::uintptr_t*);
        std::size_t space = page_size - sizeof(std::uintptr_t*);
        std::align(alignof(T), sizeof(T), first, space);
        top = reinterpret_cast<T*>(first);
    }

    ~Page() {
        free(base);
    }

    template<typename... U>
    T& construct(U... args) {
        if (reinterpret_cast<char*>(top + 1) > base + page_size) {
            throw std::bad_alloc();
        }

        return *new (top++) T(std::forward<U>(args)...);
    }
};
//***

/***
Let's create a couple of "ordinary" classes, register all the classes, and
define a method:
***/

//***

struct Person {
    virtual ~Person() {
    }
};

struct Engineer : Person {};

register_classes(Number, Integer, Rational, Person, Engineer);

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

//***

/***
Here is a test:
***/

//***

#include <sstream>

BOOST_AUTO_TEST_CASE(ref_vptr_page) {
    // Check for zero overhead.
    static_assert(sizeof(Integer) == sizeof(int));
    static_assert(sizeof(Rational) == 2 * sizeof(int));

    // Call initialize(). This must be done before we create any Numbers.
    initialize<number_aware_policy>();

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

//***

/***

[1]: This is similar to the way YOMM11 works.

***/
