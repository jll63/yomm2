// clang-format off

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/intrusive.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_MD
<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::root
entry: yorel::yomm2::derived
headers: yorel/yomm2/core.hpp

---
```c++
template<class Class, class Policy = default_policy>
struct root {
    unspecified_type unspecified_name;
    root();
    unspecified_type yomm2_mptr() const;
    void yomm2_mptr(unspecified_type mptr);
};

template<class Class, class... Bases>
struct derived {
    derived();
};
```
---
YOMM2 uses per-class _method tables_ to dispatch calls efficiently. This is
similar to the way virtual functions are implemented. In orthogonal mode, the
method table for an object is obtained from a hash table. The hash function is
collision-free, and very efficient. The overhead is ~25% compared to virtual
functions.

Hash table lookup can be eliminated for YOMM2-aware class hierarchies. This
is done by publicly inheriting from
[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) class
templates `yomm2::root` and `yomm2::derived`:

#endif

namespace direct_intrusive {

#ifdef YOMM2_CODE

namespace yomm2 = yorel::yomm2; // for brevity

class Animal : public yomm2::root<Animal> {
  public:
    virtual ~Animal() {
    }
};

class Dog : public Animal, public yomm2::derived<Dog> {
};

declare_method(void, kick, (virtual_<Animal&>));

declare_method(void, meet, (virtual_<Animal&>, virtual_<Animal&>));

#endif

define_method(void, meet, (Dog& a, Dog& b)) {
    // wag tail
}

define_method(void, kick, (Dog & dog)) {
    // bark
}

void call_kick(Animal& animal) {
    kick(animal);
}

void call_meet(Animal& a, Animal& b) {
    meet(a, b);
}

register_classes(Animal, Dog);

BOOST_AUTO_TEST_CASE(reference_direct_intrusive) {
    yorel::yomm2::update();
    Dog dog;
    call_kick(dog);
    call_meet(dog, dog);
}

} // namespace direct_intrusive

#ifdef YOMM2_MD

A call to `kick` compiles to just three instructions:

```
	mov	rax, qword ptr [rdi + 8]
	mov	rcx, qword ptr [rip + method<kick>::fn+96]
	jmp	qword ptr [rax + 8*rcx]         # TAILCALL
```

A call to `meet` compiles to:

```
	mov	rax, qword ptr [rdi + 8]
	mov	rcx, qword ptr [rip + method<meet>::fn+96]
	mov	rax, qword ptr [rax + 8*rcx]
	mov	rcx, qword ptr [rsi + 8]
	mov	rdx, qword ptr [rip + method<meet>::fn+104]
	mov	ecx, dword ptr [rcx + 8*rdx]
	imul	rcx, qword ptr [rip + method<meet>::fn+112]
	mov	rax, qword ptr [rax + 8*rcx]
	jmp	rax                             # TAILCALL


```

`root` plants two functions, both called `yomm2_mptr`, and a pointer, with an
obfuscated name, in the target class. All classes derived from a YOMM2
`root<Class>` class must derive from `derived<Class>`. Both templates define a
default constructor that sets the method table pointer.

In debug builds, hash table lookup is always performed, and the result is
compared with the method pointer stored inside the object. If they differ, an
error is raised. This helps detect missing `derived` specifications.

The second template argument - either `direct`, the default, or `indirect` -
specifies how the method pointer is stored inside the objects. In `direct` mode,
it is a straight pointer to the method table. While such objects exist,
`update` cannot be called safely (for example, after dynamically loading
a library), because the pointers would be invalidated.

In indirect mode, objects contains a pointer to a pointer to the method
table. Because of the indirection, this makes method calls slightly slower, but
`update` can be safely called at any time.

Intrusive mode works with multiple inheritance, but not with repeated
inheritance, just like the orthogonal mode [^1]. If a class inherits from more
than one YOMM2-aware classes, it must specify these classes as additional
arguments to `derived`. It also needs to include a `using` directive to
disambiguate the `yomm2_ptr` accessors.

For example:

#endif

namespace intrusive_multiple_inheritance {

#ifdef YOMM2_CODE
namespace yomm2 = yorel::yomm2;

struct Animal : public yomm2::root<Animal> {
public:
    virtual ~Animal() {
    }
};

class Property : public yomm2::root<Property> {
public:
    virtual ~Property() {
    }
};

class Dog : public Animal, public Property,
    public yomm2::derived<Dog, Animal, Property>
{
public:
    using yomm2::derived<Dog, Animal, Property>::yomm2_mptr;
    using YoMm2_S_mptr_policy_ = Animal::YoMm2_S_mptr_policy_;
};

struct Pitbull : Dog, yomm2::derived<Pitbull> {};

register_classes(Animal, Property, Dog, Pitbull);

declare_method(void*, pet, (virtual_<Animal&>));

define_method(void*, pet, (Pitbull & dog)) {
    return &dog;
}

declare_method(void*, sell, (virtual_<Property&>));

define_method(void*, sell, (Dog & dog)) {
    return &dog;
}

#endif

BOOST_AUTO_TEST_CASE(reference_direct_intrusive_mi) {
    yomm2::update();
    Pitbull dog;
    Animal& animal = dog;
    Property& property = dog;
    BOOST_TEST(pet(animal) == &dog);
    BOOST_TEST(sell(property) == &dog);
}

} // namespace direct_intrusive

#ifdef YOMM2_MD
[^1]: Repeated inheritance _could_ be made to work with intrusive mode. This is
    not supported for two reasons: it would create inconsistencies in the way
    methods are dispatched; and it would require classes to do complicated,
    error-prone work.

#endif
