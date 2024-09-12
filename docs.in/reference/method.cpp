/***
entry: method
hrefs: method-fn, method-next_type, method-add_function, method-add_definition, method-use_next
headers: yorel/yomm2/core.hpp, yorel/yomm2.hpp

```c++
template<
    typename Name, typename ReturnType, typename... Args,
    class Policy = default_policy
>
struct method; // not defined

template<typename Name, typename ReturnType, typename... Args, class Policy>
struct method<Name, ReturnType(Args...), Policy>;
```

`method` provides a static function object, `fn`, that takes a list of arguments
of type `Args`, *minus* the `virtual_` decorator, and returns `ReturnType`.
Method definitions can be added with the [`method::add_function`](#add_function)
and [`method::add_definition`](#add_definition) class templates.

## Template parameters

* **Name**: a type that differentiates methods with the same signature. It is
recommended to declare a class (there is no need to define it) for each method
name in the same namespace. ->YOMM2_SYMBOL can be used for that effect.

* **ReturnType**: the type of the value returned by the function, or void. May
  not be `auto`.

* **Args**: the types of the method's parameters. At least one parameter must be
  decorated with ->virtual_.

* **Policy**: the ->policy of the method. The method is added to the policy's
  method list; its dispatch characteristics are determined by the policy. If not
  specified, ->`default_policy` is used.

## Member functions

| Name                         | Description                        |
| ---------------------------- | ---------------------------------- |
| [constructor](#constructor)  | construct and register the method  |
| [destructor](#destructor)    | destruct and unregister the method |
| [operator()](#call-operator) | call the method                    |

## constructor

```c++
method<Name, ReturnType(Args...)>::method();
```
Add the method to the policy's method list.

## destructor

```c++
method<Name, ReturnType(Args...)>::~method();
```
Remove the method from the policy's method list.

## call operator
```c++
method<Name, ReturnType(Args...)>::operator()(args...);
```
Call the method. The dynamic types of the arguments corresponding to a
->virtual_ parameter determine which method definition to call.

## Static member variable

| Name      | Description                        |
| --------- | ---------------------------------- |
| [fn](#fn) | function object to call the method |

## fn

```c++
method<Name, ReturnType(Args...)>::fn;
```

The single instance of `method<Name, ReturnType(Args...)>`. Used to call the method.

## Member types

| Name                              | Description                                               |
| --------------------------------- | --------------------------------------------------------- |
| [add_function](#add_function)     | add a definition to the method                            |
| [add_definition](#add_definition) | add a definition container to the method                  |
| [next_type](#next_type)           | type of a pointer to the next most specialised definition |
| [use_next](#use_next)             | CRTP base for definitions that use `next`                 |

## add_function

```c++
template<auto Function>
struct add_function {
    explicit add_function(next_type* next = nullptr);
};
```

Register `Function` as a definition of the `method`. If specified, `next` is
set to a pointer to the next most specialised definition, or to an error
handler if the next definition does not excist, or is ambiguous.

The parameters of `Function` must be compatible with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be compatible with the return type of the method.

## add_definition

```c++
template<typename Container>
struct add_definition {
    add_definition();
};
```

Register static member function `Container::fn` as a definition of the
`method`. If static member variable `Container::next` exists, it is set to a
pointer to the next most specialised definition, or to an error handler if
the next definition does not exist, or is ambiguous.

The parameters of `Container::fn` must be compatible with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be compatible with the return type of the method.

## next_type

```c++
template<typename Name, typename ReturnType, typename... Args>
struct method<Name, ReturnType(Args...)> {
    using next_type = unspecified;
};
```

Register `Function` as a definition of the `method`. If specified, `next` is
set to a pointer to the next most specialised definition, or to an error
handler if the next definition does not excist, or is ambiguous.

The parameters of `Function` must be compatible with the corresponding
parameters in the method when virtual, and invariant otherwise. The return
type of `Function` must be compatible with the return type of the method.

## use_next

```c++
template<typename Container>
struct use_next {
    static next_type next;
};
template<typename Name, typename ReturnType, typename... A, typename... Unspecified>
template<typename Container>
typename method<Name, ReturnType(A...), Unspecified...>::next_type
method<Name, ReturnType(A...), Unspecified...>::use_next<Container>::next;
```

[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
base class for definition containers that need to call the next most
specialised method.

Static member variables must be declared inside the class definition, and
defined outside of it. This is cumbersome. `use_next` declares, and defines,
a static `next` function pointer, which can be injected in the container's
scope via inheritance, to be picked up by `add_container`. If this doesn't
make sense, see the example below.

## Example

***/

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

//***

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/symbols.hpp> // for YOMM2_GENSYM

#include <memory>
#include <string>

struct Animal { virtual ~Animal() {} };
struct Cat : Animal {};
struct Dog : Animal {};
struct Bulldog : Dog {};

namespace yomm2 = yorel::yomm2; // for brevity
using yomm2::virtual_;

YOMM2_STATIC(yomm2::use_classes<Animal, Cat, Dog, Bulldog>);

struct kick_methods;
using kick = yomm2::method<kick_methods, std::string(virtual_<Animal&>)>;

std::string kick_cat(Cat& dog) { return "hiss"; }
YOMM2_STATIC(kick::add_function<kick_cat>);

std::string kick_dog(Dog& dog) { return "bark"; }
YOMM2_STATIC(kick::add_function<kick_dog>);

struct kick_bulldog : kick::use_next<kick_bulldog> {
    static std::string fn(Bulldog& dog) { return next(dog) + " and bite"; }
};
YOMM2_STATIC(kick::add_definition<kick_bulldog>);

struct YOMM2_SYMBOL(pet); // use obfuscated name
using pet = yomm2::method<YOMM2_SYMBOL(pet), std::string(virtual_<Animal&>)>;

std::string pet_cat(Cat& dog) { return "purr"; }
YOMM2_STATIC(pet::add_function<pet_cat>);

std::string pet_dog(Dog& dog) { return "wag tail"; }
YOMM2_STATIC(pet::add_function<pet_dog>);

BOOST_AUTO_TEST_CASE(ref_method_example) {
    yomm2::update();

    std::unique_ptr<Animal>
        felix = std::make_unique<Cat>(),
        snoopy = std::make_unique<Dog>(),
        hector = std::make_unique<Bulldog>();

    BOOST_TEST(kick::fn(*felix) == "hiss");
    BOOST_TEST(kick::fn(*snoopy) == "bark");
    BOOST_TEST(kick::fn(*hector) == "bark and bite");

    BOOST_TEST(pet::fn(*felix) == "purr");
    BOOST_TEST(pet::fn(*snoopy) == "wag tail");
    BOOST_TEST(pet::fn(*hector) == "wag tail");
}

//***
