
<sub>/ [home](/README.md) / [reference](README.md) </sub>
## yorel::yomm2::method
<sub>defined in <yorel/yomm2/core.hpp>, also provided by
<yorel/yomm2/keywords.hpp></sub>
<!-- -->
---
```
template<typename Key, typename Signature, typename... Unspecified>
struct method<Key, R(Args...), Unspecified...>; /* undefined */
template<typename Key, typename R, typename... Args, typename... Unspecified>
struct method<Key, R(Args...), Unspecified...> { ... }
```
<!-- -->
---

`method` implements a function object that takes a list of arguments of type
`Args`, *minus* the `virtual_` decorator, and returns a value of type `R`.

The method can be called via the singleton [`method::fn`](#fn). Method
definitions can be added with the [`method::add_function`](#add_function) and
[`method::add_definition`](#add_definition) class templates.

At least one of the `Args` parameter types must be decorated with [virtual_](virtual_.md). 

`Key` is a user-suplied type that makes it possible to have distinct methods
with the same signature.

### member functions
|                              |                                    |
| ---------------------------- | ---------------------------------- |
| [constructor](#constructor)  | construct and register the method  |
| [destructor](#destructor)    | destruct and unregister the method |
| [operator()](#call-operator) | call the method                    |

### static member variable
|           |                               |
| --------- | ----------------------------- |
| [fn](#fn) | function object to call the method |

### member types
|                         |                                                           |
| ----------------------- | --------------------------------------------------------- |
| [next_type](#next_type) | type of a pointer to the next most specialised definition |

### member class templates
|                                   |                                           |
| --------------------------------- | ----------------------------------------- |
| [add_function](#add_function)     | add a definition to the method            |
| [add_definition](#add_definition) | add a definition container to the method  |
| [use_next](#use_next)             | CRTP base for definitions that use `next` |

### constructor
```c++
method<Key, R(Args...)>::method();
```
Add the method to the global method list.

### destructor
```c++
method<Key, R(Args...)>::~method();
```
Remove the method from the global method list.

### call operator
```c++
method<Key, R(Args...)>::operator()(args...);
```
Call the method. The dynamic types of the arguments corresponding to a
[virtual_](virtual_.md) parameter determine which method definition to call.

### fn
```c++
method<Key, R(Args...)>::fn;
```

The single instance of `method<Key, R(Args...)>`. Used to call the method.

### next_type
```c++
template<typename Key, typename R, typename... Args>
struct method<Key, R(Args...)> {
    using next_type = /*unspecified*/;
};
```

Register `Function` as a definition of the `method`. If specified, `next` is
set to a pointer to the next most specialised definition, or to an error
handler if the next definition does not excist, or is ambiguous.

The parameters of `Function` must be covariant with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be covariant with the return type of the method.

### add_function
```c++
template<typename Key, typename R, typename... Args>
struct method<Key, R(Args...)> {
    template<auto Function>
    struct add_function {
        explicit add_function(next_type* next = nullptr);
    };
};
```

Register `Function` as a definition of the `method`. If specified, `next` is
set to a pointer to the next most specialised definition, or to an error
handler if the next definition does not excist, or is ambiguous.

The parameters of `Function` must be covariant with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be covariant with the return type of the method.

### add_definition
```c++
template<typename Key, typename R, typename... Args>
struct method<Key, R(Args...)> {
    template<typename Container>
    struct add_definition {
        add_definition();
    };
};
```

Register static member function `Container::fn` as a definition of the
`method`. If static member variable `Container::next` exists, it is set to a
pointer to the next most specialised definition, or to an error handler if
the next definition does not exist, or is ambiguous.

The parameters of `Container::fn` must be covariant with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be covariant with the return type of the method.

### use_next
```c++
template<typename Key, typename R, typename... Args, typename... Unspecified>
struct method<Key, R(Args...)> {
    template<typename Container>
    struct use_next {
        static next_type next;
    };
};
template<typename Key, typename R, typename... A, typename... Unspecified>
template<typename Container>
typename method<Key, R(A...), Unspecified...>::next_type
method<Key, R(A...), Unspecified...>::use_next<Container>::next;
```

[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
base class for definition containers that need to call the next most
specialised method.

Static member variables must be declared inside the class definition, and
defined outside of it. This is cumbersome. `use_next` declares, and defines,
a static `next` function pointer, which can be injected in the container's
scope via inheritance, to be picked up by `add_container`. If this doesn't
make sense, see the example below.

## example

```c++

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/symbols.hpp> // for YOMM2_GENSYM

#include <memory>
#include <string>

struct Animal { virtual ~Animal() {} };
struct Cat : Animal {};
struct Dog : Animal {};
struct Bulldog : Dog {};

using namespace yorel::yomm2; // for brevity

use_classes<Animal, Cat, Dog, Bulldog> YOMM2_GENSYM;

struct YOMM2_SYMBOL(kick_methods);
using kick = method<YOMM2_SYMBOL(kick_methods), std::string(virtual_<Animal&>)>;

std::string kick_cat(Cat& dog) { return "hiss"; }
kick::add_function<kick_cat> YOMM2_GENSYM;

std::string kick_dog(Dog& dog) { return "bark"; }
kick::add_function<kick_dog> YOMM2_GENSYM;

struct kick_bulldog : kick::use_next<kick_bulldog> {
    static std::string fn(Bulldog& dog) { return next(dog) + " and bite"; }
};
kick::add_definition<kick_bulldog> YOMM2_GENSYM;

struct YOMM2_SYMBOL(pet_methods);
using pet = method<YOMM2_SYMBOL(pet_methods), std::string(virtual_<Animal&>)>;

std::string pet_cat(Cat& dog) { return "purr"; }
pet::add_function<pet_cat> YOMM2_GENSYM;

std::string pet_dog(Dog& dog) { return "wag tail"; }
pet::add_function<pet_dog> YOMM2_GENSYM;

BOOST_AUTO_TEST_CASE(call_method) {
    update_methods();

    std::shared_ptr<Animal>
        felix = std::make_shared<Cat>(),
        snoopy = std::make_shared<Dog>(),
        hector = std::make_shared<Bulldog>();

    BOOST_TEST(kick::fn(*felix) == "hiss");
    BOOST_TEST(kick::fn(*snoopy) == "bark");
    BOOST_TEST(kick::fn(*hector) == "bark and bite");

    BOOST_TEST(pet::fn(*felix) == "purr");
    BOOST_TEST(pet::fn(*snoopy) == "wag tail");
    BOOST_TEST(pet::fn(*hector) == "wag tail");
}

```
