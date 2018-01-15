# Reference

There is very little to know in terms of functions, types, etc. The API of
yomm2 consists of two headers, five pseudo keywords, and one function.

It is however very important to understand the semantics of method dispatch.

### header yorel/yomm2.hpp

#### Synopsis:
```
#include <yorel/yomm2.hpp>
```

This is the library's main header. It defines the YOMM2_* macros, and namespace
yorel::yomm2 and its content.

### header yorel/yomm2/cute.hpp

#### Synopsis:
```
#include <yorel/yomm2/cute.hpp>
```

### namespace yorel::yomm2

### yorel::yomm2::update_methods()

#### Synopsis:
```
int main(int argc, const char** argv) {
    yorel::yomm2::update_methods();
    // ...
}
```

Create the tables used during method dispatch.

This function must be called before any method is called (typically in
`main`). It must also be called after a shared library is dynamically loaded or
unloaded, if the library adds method declarations, method definitions, or
classes derived from classes that are used as virtual arguments.

### macros YOMM2_DECLARE, declare_method

#### Synopsis:
```
YOMM2_DECLARE(return_type, method, (type...));

return_type rv = method(unmarked_type... arg);

```

Declare a method.

Create an inline function `method` that returns a `return type` and takes an
argument list of `unmarked_type...`.  At least one `type` (but not necessarily
all) must be marked with `virtual_`.

The `unmarked_type...` consist of `type...` without the `virtual_` marker.

When `method` is called, the dynamic types of the arguments marked with
`virtual_` are examined, and the most specific definition compatible with
`unmarked_type...` is called. If no compatible definition exists, or if
several compatible definitions exist but none of them is more specific than
all the others, the call is illegal and an error handler is executed. By
default it writes a diagnostic on `std::cerr ` and terminates the program via
`abort`. The handler can be customized.

`declare_method` is an alias for `YOMM2_DECLARE` created by header
`yorel/yomm2/cute.hpp`.

NOTE:

* The parameter list `type...` _must_ be surrounded by parentheses.

* The parameters in `type...` consist of _just_ a type, e.g. `int` is correct
  but `int i` is not.

### Examples:
```
declare_method(std::string, kick, (virtual_<Animal&>));
declare_method(std::string, meet, (virtual_<Animal&>, virtual_<Animal&>));
declare_method(bool, approve, (virtual_<Role&>, virtual_<Expense&>), double);
```

### template virtual_

#### Synopsis:
```
virtual_<type>
```
Mark a parameter as virtual. Meaningful only inside a method declaration parameter list.

`type` must be a reference, a pointer or a `std::shared_ptr` to a polymorphic
type, perhaps qualified with `const`.

### Examples:
```
using yorel::yomm2::virtual_;

struct Animal {
    virtual ~Animal();
};

YOMM2_DECLARE(void, kick, (virtual_<Animal*>));
YOMM2_DECLARE(void, kick, (virtual_<Animal&>));
YOMM2_DECLARE(void, kick, (virtual_<shared_ptr<Animal>>));
YOMM2_DECLARE(void, kick, (virtual_<const Animal*>));
YOMM2_DECLARE(void, kick, (virtual_<const Animal&>));
YOMM2_DECLARE(void, kick, (virtual_<shared_ptr<const Animal>>));

```

Given a polymorphic class `Animal`, these are all the valid ways of specifying
a virtual Animal argument in a method declaration. NOTE that
virtual_<shared_ptr<const Animal>&> is _not_ in this list and is _not_
supported; passing shared_ptrs by const reference is a bad idea anyway.

### macros YOMM2_METHOD, YOMM2_END, begin_method, end_method

#### Synopsis:
```
YOMM2_BEGIN(return_type, name, (type... argument)) {
    ....
} YOMM2_END;
```

Add an implementation to a method.

Locate a method that can be called with the specified `type...` list and add
the definition to the method's list of definitions. The method must exist and
must be unique.

### Examples:
```
// implement 'kick' for dogs
YOMM2_BEGIN(std::string, kick, (Dog& dog)) {
  return "bark";
} YOMM2_END;

// implement 'kick' for bulldogs
YOMM2_BEGIN(std::string, kick, (Bulldog& dog)) {
    return next(dog) + " and bite";
} YOMM2_END;

// 'meet' catch-all implementation
YOMM2_BEGIN(std::string, meet, (Animal&, Animal&)) {
  return "ignore";
} YOMM2_END;

YOMM2_BEGIN(std::string, meet, (Dog& dog1, Dog& dog2)) {
  return "wag tail";
} YOMM2_END;

YOMM2_BEGIN(std::string, meet, (Dog& dog, Cat& cat)) {
  return "chase";
} YOMM2_END;

YOMM2_BEGIN(std::string, meet, (Cat& cat, Dog& dog)) {
  return "run";
} YOMM2_END;
```

### next

#### Synopsis:
```
YOMM2_BEGIN(return_type, name, (type... arg)) {
    ....
    next(arg...);
    ...
} YOMM2_END;
```

Call the next most specific implementation. Valid only inside a method.

#### Example:

```
// Executive is-a Employee

YOMM2_DEFINE(double, pay, (const Employee&)) {
    return 3000;
} YOMM2_END;

YOMM2_DEFINE(double, pay, (const Executive& exec)) {
    return next(exec) // call pay(const Employee&)
           + 2000;    // bonus
} YOMM2_END;

const Employee& elon = Executive();
double paycheck = pay(elon); // 3000 + 2000
```

### macros register_class, YOMM2_CLASS

#### Synopsis:
```
YOMM2_CLASS(polymorphic_type);
YOMM2_CLASS(polymorphic_type, polymorphic_base_type...);
```

Register a class.

Every class that is used as a virtual parameter, and its subclasses, must be
registered with `YOMM2_CLASS`.

#### Examples:
```
YOMM2_CLASS(Animal);
YOMM2_CLASS(Dog, Animal);
```
