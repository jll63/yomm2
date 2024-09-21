// clang-format off

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/virtual_shared_ptr.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

/***
entry: virtual_ptr
entry: virtual_shared_ptr
entry: make_virtual_shared
hrefs: virtual_ptr-final
headers: yorel/yomm2/core.hpp, yorel/yomm2.hpp, yorel/yomm2.hpp

```c++
template<class Class, class Policy = YOMM2_DEFAULT_POLICY>
class virtual_ptr;
```

`virtual_ptr` is a fat pointer that consists of a pointer to an object, and a
pointer to its associated method table. It can be used in ->`declare_method`, in
place of the ->`virtual_`, for virtual method parameters.

Unlike `virtual_`, `virtual_ptr` should be used in method definitions as well -
see examples below.

`virtual_ptr` can be passed either by value, or by const reference. In general,
passing by value should be preferred, as it makes it possible to pass the two
embedded pointers via registers. However, see the paragraph about
specializations below.

*Note*: calling `update` invalidates existing `virtual_ptr`s.

### Template parameters

`Class` - The type of the object.
`pOLICY` - The type of the object.

### Specializations

```
template<class Class, class Policy = default_policy>
class virtual_ptr<std::shared_ptr<Class>>;
```

This specialization uses the standard `shared_ptr` to store the address of the
object.

Virtual shared pointers should be passed by const reference, to avoid excessive
manipulations of the reference count.

## Member functions

|                               |                              |
| ----------------------------- | ---------------------------- |
| ([constructor](#constructor)) | constructs a new virtual_ptr |


### observers

|                               |                                 |
| ----------------------------- | ------------------------------- |
| [get](#get)                   | returns the stored pointer      |
| [operator*](#deref-operator)  | dereferences the stored pointer |
| [operator->](#deref-operator) | dereferences the stored pointer |


## Static member functions

|                                                    |                           |
| -------------------------------------------------- | ------------------------- |
| [template@<class Other> final(Other& obj)](#final) | returns a new virtual_ptr |


## Non member types

|                                                                  |                                                 |
| ---------------------------------------------------------------- | ----------------------------------------------- |
| [template@<class Class> virtual_shared_ptr](#virtual_shared_ptr) | alias for `virtual_ptr<std::shared_ptr<Class>>` |

## Non member functions

|                                                                      |                                                 |
| -------------------------------------------------------------------- | ----------------------------------------------- |
| [template@<class Class> make_virtual_shared()](#make_virtual_shared) | creates an object and returns a new virtual_ptr |

## virtual_ptr

|                                                                          |     |
| ------------------------------------------------------------------------ | --- |
| `template@<class Other$gt; virtual_ptr(Other& obj)`                      | (1) |
| `template@<class Other$gt; virtual_ptr(const virtual_ptr<Other>& other)` | (2) |

(1) Constructs a `virtual_ptr` that contains a reference to `obj`, which must be
compatible with `Class`, and a pointer to the method table corresponding to
`obj`'s *dynamic* type. If the dynamic type of `obj` is the same as `Other`
(i.e. `typeid(obj) == typeid(Other)`), the hash table is not looked up.

(2) Constructs a `virtual_ptr` that contains a reference to `obj`, and a pointer
to the method table corresponding to `obj`'s *dynamic* type. If the dynamic type
of `obj` is the same as `Other` (i.e. `typeid(obj) == typeid(Other)`),
the hash table is not looked up.

## get

|                                       |
| ------------------------------------- |
| `element_type* get() const noexcept;` |

Returns the stored pointer.

### Parameters
(none)

### Return value
The stored pointer.

<a name="deref-operator"></a>
## operator*, operator->

|                                       |
| ------------------------------------- |
| `Class& operator*() const noexcept;`  |
| `Class* operator->() const noexcept;` |

Dereferences the stored pointer.

### Parameters

(none)

### Return value

1) The result of dereferencing the stored pointer, i.e., *get().
2) The stored pointer, i.e., get().

## final

|                                                      |     |
| ---------------------------------------------------- | --- |
| `template@<class Other$gt; static final(Other& obj)` | (1) |

Constructs a `virtual_ptr` that contains a reference to `obj`, which must be
compatible with `Class`, and a pointer to the method table corresponding to
`obj`'s *static* type. Neither `Class` nor `Other` are not required to be
polymorphic types.

In debug builds, `final` compares `typeid(Class)` and `typeid(obj)`. If they are
different, `error_handler` is called with a `method_table_error` object. This
can help detect misuses of `final`, but only for polymorphic classes.

## virtual_shared_ptr

`virtual_shared_ptr<Class>` is an alias for
`virtual_ptr<std::shared_ptr<Class>>`.

## make_virtual_shared

|                                                   |     |
| ------------------------------------------------- | --- |
| `template@<class Class$gt; make_virtual_shared()` |     |

Constructs an object, using `std::make_shared`, and return a `virtual_ptr` to
it. No hash table lookup is performed.

This construct is always safe to use, even with non-polymorphic types.

# Discussion

Calls to methods through a `virtual_ptr` are almost as efficient as virtual
function calls, because they do not require a hash table lookup, unlike calls
made using the orthogonal mode. The lookup is performed only once, when the
pointer is created via a call to the constructor, and the result is cached.

Calling `update` invalidates all the existing `virtual_ptr`s.

## Example

***/

namespace ref_virtual_ptr {

//***

using yorel::yomm2::virtual_ptr;

class Animal {
  public:
    virtual ~Animal() {
    }
};

class Dog : public Animal {
};

class Cat : public Animal {
};

register_classes(Animal, Dog, Cat);

declare_method(kick, (virtual_ptr<Animal>, std::ostream&), void);

define_method(kick, (virtual_ptr<Dog> dog, std::ostream& os), void) {
    os << "bark";
}

define_method(kick, (virtual_ptr<Cat> cat, std::ostream& os), void) {
    os << "hiss";
}

declare_method(meet, (virtual_ptr<Animal>, virtual_ptr<Animal>, std::ostream&), void);

define_method(meet, (virtual_ptr<Dog> a, virtual_ptr<Dog> b, std::ostream& os), void) {
    os << "wag tail";
}

define_method(meet, (virtual_ptr<Cat> a, virtual_ptr<Dog> b, std::ostream& os), void) {
    os << "run";
}

define_method(meet, (virtual_ptr<Dog> a, virtual_ptr<Cat> b, std::ostream& os), void) {
    os << "chase";
}

BOOST_AUTO_TEST_CASE(ref_virtual_ptr) {
    yorel::yomm2::initialize();

    Dog snoopy, hector;
    Cat sylvester;

    std::vector<virtual_ptr<Animal>> animals;
    animals.emplace_back(virtual_ptr<Animal>(snoopy));
    animals.emplace_back(virtual_ptr<Animal>(sylvester));
    animals.emplace_back(virtual_ptr<Animal>(hector));

    {
        boost::test_tools::output_test_stream os;
        kick(animals[0], os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        boost::test_tools::output_test_stream os;
        kick(animals[1], os);
        BOOST_CHECK(os.is_equal("hiss"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[1], os);
        BOOST_CHECK(os.is_equal("chase"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[2], os);
        BOOST_CHECK(os.is_equal("wag tail"));
    }
}

//***

void call_kick(virtual_ptr<Animal> animal, std::ostream& os) {
    kick(animal, os);
}

void call_meet(virtual_ptr<Animal> a, virtual_ptr<Animal> b, std::ostream& os) {
    meet(a, b, os);
}

} // namespace direct_intrusive

/***

A call to `kick` compiles to three instructions and two memory reads:

```asm
mov	rax, qword ptr [rip + method<kick, ...>::fn+96]
mov	rax, qword ptr [rsi + 8*rax]
jmp	rax
```

A call to `meet` compiles to:

```asm
mov	rax, qword ptr [rip + method<meet, ...>::fn+96]
mov	r8, qword ptr [rsi + 8*rax]
mov	rax, qword ptr [rip + method<meet, ...>::fn+104]
mov	rax, qword ptr [rcx + 8*rax]
imul	rax, qword ptr [rip + method<meet, ...>::fn+112]
mov	rax, qword ptr [r8 + 8*rax]
jmp	rax
```

# Non-polymorphic classes

`final` can be used to add polymorphic behavior to non-polymorphic classes. It
is the responsibility of the caller to ensure that the static type of the object
is, indeed, its exact type.


## Example

***/

namespace ref_virtual_ptr_final {

//***

class Animal {
    // note: no virtual functions
};

class Dog : public Animal {
};

class Cat : public Animal {
};

register_classes(Animal, Dog, Cat);

using yorel::yomm2::virtual_ptr;

declare_method(kick, (virtual_ptr<Animal>, std::ostream&), void);

define_method(kick, (virtual_ptr<Dog> dog, std::ostream& os), void) {
    os << "bark";
}

define_method(kick, (virtual_ptr<Cat> cat, std::ostream& os), void) {
    os << "hiss";
}

declare_method(meet, (virtual_ptr<Animal>, virtual_ptr<Animal>, std::ostream&), void);

define_method(meet, (virtual_ptr<Dog> a, virtual_ptr<Dog> b, std::ostream& os), void) {
    os << "wag tail";
}

define_method(meet, (virtual_ptr<Cat> a, virtual_ptr<Dog> b, std::ostream& os), void) {
    os << "run";
}

define_method(meet, (virtual_ptr<Dog> a, virtual_ptr<Cat> b, std::ostream& os), void) {
    os << "chase";
}

BOOST_AUTO_TEST_CASE(ref_virtual_ptr_final) {
    yorel::yomm2::initialize();

    Dog snoopy, hector;
    Cat sylvester;

    std::vector<virtual_ptr<Animal>> animals = {
        virtual_ptr<Animal>::final(snoopy),
        virtual_ptr<Animal>::final(sylvester),
        virtual_ptr<Animal>::final(hector),
    };

    {
        boost::test_tools::output_test_stream os;
        kick(animals[0], os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        boost::test_tools::output_test_stream os;
        kick(animals[1], os);
        BOOST_CHECK(os.is_equal("hiss"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[1], os);
        BOOST_CHECK(os.is_equal("chase"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[2], os);
        BOOST_CHECK(os.is_equal("wag tail"));
    }
}

//***

/***

For non-polymorphic types, YOMM2 cannot detect misuses of `final`. At best, it
will result in a missing definition error, at worst the wrong definition will be
selected, as in the following example.

***/

//***

define_method(kick, (virtual_ptr<Animal> dog, std::ostream& os), void) {
    os << "wrong call";
}

BOOST_AUTO_TEST_CASE(ref_virtual_ptr_final_incorrect) {
    yorel::yomm2::initialize();

    Dog snoopy;
    Animal& animal = snoopy;
    auto animal_vptr = virtual_ptr<Animal>::final(animal);

    boost::test_tools::output_test_stream os;
    kick(animal_vptr, os);
    BOOST_CHECK(os.is_equal("wrong call"));
}

//***

}

/***

# Virtual shared pointers

The `virtual_ptr<std::shared_ptr<Class>>` specialisation combines the fast
method dispatch of `virtual_ptr` with the lifetime management of
`std::shared_ptr`. Instead of using an ordinary pointer to store the reference
to the object, it uses a `std::shared_ptr`.

`virtual_shared_ptr<Class>` is an alias for
`virtual_ptr<std::shared_ptr<Class>>`.

`make_virtual_shared<Class>` calls `std::make_shared<Class>`, and returns a
`virtual_shared_ptr<Class>`. Since the exact type of the object is known, there
is no need to consult the hash table. For this reason, `virtual_shared_ptr` is
safe to use for non-polymorphic types.

## Example

***/

namespace YOMM2_GENSYM {

//***

class Animal {
    // it does not matter if the class has virtual functions
};

class Dog : public Animal {
};

class Cat : public Animal {
};

register_classes(Animal, Dog, Cat);

using yorel::yomm2::virtual_shared_ptr;

declare_method(kick, (const virtual_shared_ptr<Animal>&, std::ostream&), void);

define_method(kick, (const virtual_shared_ptr<Dog>& dog, std::ostream& os), void) {
    os << "bark";
}

define_method(kick, (const virtual_shared_ptr<Cat>& cat, std::ostream& os), void) {
    os << "hiss";
}

declare_method(meet, (const virtual_shared_ptr<Animal>&,const virtual_shared_ptr<Animal>&, std::ostream&), void);

define_method(meet, (const virtual_shared_ptr<Dog>& a,const virtual_shared_ptr<Dog>& b, std::ostream& os), void) {
    os << "wag tail";
}

define_method(meet, (const virtual_shared_ptr<Cat>& a,const virtual_shared_ptr<Dog>& b, std::ostream& os), void) {
    os << "run";
}

define_method(meet, (const virtual_shared_ptr<Dog>& a,const virtual_shared_ptr<Cat>& b, std::ostream& os), void) {
    os << "chase";
}

BOOST_AUTO_TEST_CASE(ref_make_virtual_shared) {
    yorel::yomm2::initialize();

    using yorel::yomm2::make_virtual_shared;

    std::vector<virtual_shared_ptr<Animal>> animals = {
        make_virtual_shared<Dog>(),
        make_virtual_shared<Cat>(),
        make_virtual_shared<Dog>(),
    };

    {
        boost::test_tools::output_test_stream os;
        kick(animals[0], os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        boost::test_tools::output_test_stream os;
        kick(animals[1], os);
        BOOST_CHECK(os.is_equal("hiss"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[1], os);
        BOOST_CHECK(os.is_equal("chase"));
    }

    {
        boost::test_tools::output_test_stream os;
        meet(animals[0], animals[2], os);
        BOOST_CHECK(os.is_equal("wag tail"));
    }
}

//***
}

/***

[^1]: The only reason why `virtual_ptr` is not called `virtual_ref` is to save
    the name for the time when C++ supports a user-defined dot operator.

***/
