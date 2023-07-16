#ifdef YOMM2_MD

<sub>/ ->home / ->reference </sub>

entry:yomm2::use_classes
headers: yorel/yomm2/core.hpp
<br>
entry: register_classes
headers: yorel/yomm2/cute.hpp, <yorel/yomm2/keywords.hpp
<br>
entry:YOMM2_CLASSES
headers: yorel/yomm2/macros.hpp, yorel/yomm2/yomm2.hpp

---
```
template<typename... Cs> struct use_classes;
```
---

`use_classes`, instantiated as a [static object](static-object.md), registers
a list of classes, and their inheritance relationships. All classes that
potentially take part in a method call must be registered with `use_classes`.

In order for `use_classes` to correctly deduce the inheritance graphs, if a
class is a direct base of another class, they must appear together in a same
instance of `use_classes`. If a class has several direct base classes, they
need not all appear in the same `use_classes`; inheritance relationships can
be added incrementally. See examples below.

Note that the registration requirement does not only apply to classes used as
virtual parameters, and the classes used as parameters in method definitions
that correspond to virtual parameters. The runtime class of all the objects
potentially partaking in method *calls* must be registered. For example,
given the hierarchy Animal -> Dog -> Bulldog; if a method declaration takes a
`virtual_<Animal>`; if the method has two definitions, one for Animal, and
one for Bulldog; and the program calls the method for a Dog (that is not a
Bulldog); then Dog must be registered as well, and it needs to appear with
Animal in a `use_classes`, and with Bulldog in a `use_classes`.

In debug builds, YOMM2 checks at the call site that the runtime classes of
all the virtual arguments has been registered. If not, an error message is
written to `cerr`, and `abort()` is called. The check works even if the YOMM2
runtime was compiled in release mode. If the program itself is compiled in
release mode, and not all the classes have been registered, the program will
segfault, or worse, the wrong method definition may be called.

The time complexity of `use_classes` is O(n^2) a compile time and at runtime
(during `update_methods`). If necessary, large hierarchies can be registered
incrementally.

## macros

`register_classes(...)`  and `YOMM2_CLASSES(...)` are simple wrappers around
`use_classes`. Both are equivalent to `use_classes<...> YOMM2_GENSYM`.

## example

Given the following hierarchy:

#endif

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>
#include <yorel/yomm2/keywords.hpp>

BOOST_AUTO_TEST_CASE(reference_dummy) {}

#ifdef YOMM2_CODE

struct Animal {
    virtual ~Animal() {
    }
};
struct Herbivore : virtual Animal {};
struct Carnivore : virtual Animal {};
struct Omnivore : Herbivore, Carnivore {};
struct Human : Omnivore {};
struct Wolf : Omnivore {};
struct Sheep : Herbivore {};

#endif

#ifdef YOMM2_MD
All the classes can be registered with a single static object:
#endif

#ifdef YOMM2_CODE

// at file scope
using yorel::yomm2::use_classes;
use_classes<
    Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep
> YOMM2_GENSYM;

#endif

#ifdef YOMM2_MD
Or, using either macro:
#endif

#ifdef YOMM2_CODE

register_classes(Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep);
YOMM2_CLASSES(Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep);

#endif

#ifdef YOMM2_MD
Classes can also be registered incrementally:
#endif

#ifdef YOMM2_CODE

use_classes<Animal, Herbivore, Carnivore> YOMM2_GENSYM;
use_classes<Omnivore, Human, Wolf> YOMM2_GENSYM;
use_classes<Sheep, Herbivore> YOMM2_GENSYM;

#endif

#ifdef YOMM2_MD

The following is **wrong**, because `use_classes` cannot infer that `Human`
and `Wolf` derive from `Omnivore`.

#endif

#ifdef YOMM2_CODE

use_classes<Animal, Herbivore, Carnivore, Omnivore> YOMM2_GENSYM;
use_classes<Human, Wolf> YOMM2_GENSYM;      // wrong!
use_classes<Sheep, Herbivore> YOMM2_GENSYM; // ok

#endif

#ifdef YOMM2_MD

## see also

|  | |
|---|---|
| ->YOMM2_GENSYM | generate a symbol

#endif
