<sub>/ ->home / ->reference </sub>

entry: yorel::yomm2::class_declaration
headers: yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

---

```c++
template<typename ClassList, typename... Unspecified>
struct class_declaration;
```

### usage
```c++
class_declaration<Class, Bases...> identifier;
class_declaration<types<Class, Bases...>> identifier;
```
---

Register `Class` and its direct `Bases`.

All classes that potentially take part in a method call must be registered with
`class_declaration`, along with the inheritance relationships.

Note that the registration requirement does not only apply to classes used as
virtual parameters, and the classes used as parameters in method definitions
that correspond to virtual parameters. The runtime class of all the objects
potentially partaking in method *calls* must be registered. For example, given
the hierarchy Animal -> Dog -> Bulldog; if a method declaration takes a
`virtual_<Animal>`; if the method has two definitions, one for Animal, and one
for Bulldog; and the program calls the method for a Dog (that is not a Bulldog);
then Dog must be registered as well.

## example
```c++
struct Animal { virtual ~Animal() {} };
struct Dog : Animal {};
struct Bulldog : Dog {};
struct Cat : Animal {};

class_declaration<types<Animal>> register_animal;
class_declaration<types<Dog, Animal>> register_dog;
class_declaration<types<Bulldog, Dog>> register_bulldog;
class_declaration<types<Cat, Animal>> register_cat;
```

## see also
`class_declaration` is a low level construct. It is recommended to use
->use_classes instead, which can register many classes, and their inheritance
relationships,  using a single [registration object](static-object.md).
