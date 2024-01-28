#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

/***
<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::policy::vptr
entry: yorel::yomm2::policy::external_vptr
entry: yorel::yomm2::policy::external_vptr_vector
entry: yorel::yomm2::policy::external_vptr_map
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---
```
struct vptr;

struct external_vptr;

template<class Policy>
struct external_vptr_vector;

template<class Policy>
struct external_vptr_map;
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
***/

//{

#include <yorel/yomm2/policy.hpp>

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

    Person();

    virtual ~Person() {
    }
};

struct Engineer : Person {
    Engineer();
};

struct my_vptr_policy;

#include <yorel/yomm2/policy.hpp>

// for brevity
using namespace yorel::yomm2;
using namespace policy;

struct vptr_page : virtual default_policy::use_facet<vptr> {

    template<class Class>
    static auto vptr(const Class& arg) {
        if constexpr (std::is_base_of_v<Number, Class>) {
            auto page = reinterpret_cast<std::uintptr_t>(&arg) & ~1023;
            return *reinterpret_cast<std::uintptr_t**>(page);
        } else if constexpr (std::is_base_of_v<Person, Class>) {
            return arg.vptr;
        } else {
            return default_policy::use_facet<policy::vptr>::vptr(arg);
        }
    }
};

struct my_vptr_policy : default_policy::replace<vptr, vptr_page> {};

template<class T>
class Page {
  private:
    static constexpr size_t page_size = 1024;
    char* base;
    T* top;

  public:
    Page() {
        base =
            reinterpret_cast<char*>(std::aligned_alloc(page_size, page_size));
        *reinterpret_cast<std::uintptr_t**>(base) =
            my_vptr_policy::static_vptr<T>;
        void* first = base + sizeof(std::uintptr_t*);
        size_t space = page_size - sizeof(std::uintptr_t*);
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

Person::Person() {
    vptr = my_vptr_policy::static_vptr<Person>;
}

Engineer::Engineer() {
    vptr = my_vptr_policy::static_vptr<Engineer>;
}

#define YOMM2_DEFAULT_POLICY my_vptr_policy
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>

register_classes(Number, Integer, Rational, Person, Engineer);

#include <iostream>

declare_method(
    void, add,
    (virtual_<Person&>, virtual_<const Number&>, virtual_<const Number&>,
     std::ostream&));

define_method(
    void, add, (Person&, const Number& a, const Number& b, std::ostream& os)) {
    os << "I don't know how to do this...\n";
}

define_method(
    void, add,
    (Person&, const Integer& a, const Integer& b, std::ostream& os)) {
    os << a.value << " + " << b.value << " = " << a.value + b.value << "\n";
}

define_method(
    void, add,
    (Engineer&, const Rational& a, const Rational& b, std::ostream& os)) {
    os << a.num << "/" << a.den << " + " << b.num << "/" << b.den << " = "
       << a.num * b.den + a.den * b.num << "/" << a.den * b.den << "\n";
}

BOOST_AUTO_TEST_CASE(ref_vptr_page) {
    static_assert(sizeof(Integer) == sizeof(int));
    static_assert(sizeof(Rational) == 2 * sizeof(int));
    update<my_vptr_policy>();
    Page<Integer> ints;
    Number &i_2 = ints.construct(2), &i_3 = ints.construct(3);
    Page<Rational> rationals;
    Number &r_2_3 = rationals.construct(2, 3),
           &r_3_4 = rationals.construct(3, 4);
    Person&& alice = Engineer();
    Person&& bob = Person();
    add(bob, i_2, i_3, std::cout);
    add(bob, r_2_3, r_3_4, std::cout);
    add(alice, r_2_3, r_3_4, std::cout);
}

//}
