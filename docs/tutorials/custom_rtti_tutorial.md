

# Using Custom RTTI

## RTTI, policies and facets

YOMM2 uses Run-Time Type Information for three purposes:
1. Identify classes in the `use_classes` and `register_classes` constructs,
   using the typeid(_type_) operator.
2. Determine the dynamic type of an object, and find the appropriate, using the
   typeid(_expression_) operator.
3. To cast from base to derived class, using the dynamic_cast operator.

The policy's `rtti` facet provides type information, and performs dynamic casts.
It provides the following static member functions:

* `template<typename T> static type_id static_type();`<br>
    Returns a `type_id` (a typedef for `std::uintptr_t`) for type `T`.
    `use_classes` calls `static_type` at static construction time, and stores,
    for each registered class, the `type_id` of the class, and its bases.
    `update` uses that information to piece together a complete description of
    the inheritance relationships for the classes registered within a policy.
    Note that `T` is not restricted to the class of the virtual parameters! It
    is called for each method parameter - for example, `T` can be `int`.
    However, it is acceptable to return the same value for all non-virtual
    parameters. This function is required.

* `template<typename T> static type_id dynamic_type(T& value);`<br>
    Returns the `type_id` of `value`. It is called, during method dispatch, for
    each virtual argument, to locate the appropriate method table for the
    object's _dynamic_ class. If an error occurs (either a missing definition or
    an ambiguous call), it is called for each argument (virtual or not) to
    create an error message. This function is required.

* `template<class Stream> static void type_name(type_id type, Stream&
  stream);`<br>
    Writes a representation of `type` to `stream`. Used to format error
    messages, and by `update` if trace is enabled. `Stream` is a lighweight
    version of `std::ostream` with reduced functionality. It only supports
    insertion of `const char*`, `std::string_view`, pointers and `size_t`. This
    function is optional; if not provided, "type_id(_type_)" is used.

* `static (unspecified) type_index(type_id type);`<br>
    Returns an object that _uniquely_ identifies a class. Some forms of RTTI
    (like C++'S `typeid` operator) do not guarantee that the type information
    object for a class is unique within the same program. This function is
    called by `update` to consolidate the different type objects for a class.
    The return type must conform to the requirements of a key in a
    `std::unordered_map`. This function is optional; if not provided, `type` is
    assumed to be unique, and used as is.

* `template<typename D, typename B> static D dynamic_cast_ref(B&& obj);`<br>
    Cast `obj` to class `D`. `B&&` is either a lvalue reference (possibly
    cv-qualified) or a rvalue reference. `D` has the same reference category
    (and cv-qualifier if applicable) as `B`. YOMM2 uses `static_cast` to
    downcast method arguments to definition arguments whenever possible. Thus,
    this function required only in presence of virtual inheritance.

Here is the full definition of `std_rtti`:

```c++
struct std_rtti : rtti {
    template<typename T>
    static type_id static_type() {
        return reinterpret_cast<type_id>(&typeid(T));
    }

    template<typename T>
    static type_id dynamic_type(const T& obj) {
        return reinterpret_cast<type_id>(&typeid(obj));
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << reinterpret_cast<const std::type_info*>(type)[name](None)();
    }

    static std::type_index type_index(type_id type) {
        return std::type_index(*reinterpret_cast<const std::type_info*>(type));
    }

    template<typename D, typename B>
    static D dynamic_cast_ref(B&& obj) {
        return dynamic_cast<D>(obj);
    }
};
```

If standard RTTI is disabled, the body of this class is `#ifdef`'ed out, and the
`default_policy` cannot be used.

## A custom RTTI facet

Consider a toy RTTI implementation that uses `const char*`s as type ids. It also
provides its own dynamic casting facility:


```c++
struct Animal {
    Animal(const char* name, const char* type) : name(name), type(type) {
    }

    virtual void* cast_aux(const char* type) {
        return type == static_type ? this : nullptr;
    }

    const char* name;
    const char* type;
    static const char* static_type;
};

const char* Animal::static_type = "Animal";

template<typename Derived, typename Base>
Derived custom_dynamic_cast(Base& obj) {
    using derived_type = std::remove_cv_t<std::remove_reference_t<Derived>>;
    return *reinterpret_cast<derived_type*>(
        const_cast<std::remove_cv_t<Base>&>(obj).cast_aux(
            derived_type::static_type));
}

struct Dog : virtual Animal {
    Dog(const char* name, const char* type = static_type) : Animal(name, type) {
    }

    void* cast_aux(const char* type) override {
        return type == static_type ? this : Animal::cast_aux(type);
    }

    static const char* static_type;
};

const char* Dog::static_type = "Dog";

struct Cat : virtual Animal {
    Cat(const char* name, const char* type = static_type) : Animal(name, type) {
    }

    void* cast_aux(const char* type) override {
        return type == static_type ? this : Animal::cast_aux(type);
    }

    static const char* static_type;
};

const char* Cat::static_type = "Cat";
```


We need to write a `rtti` facet for this RTTI system. Note that, in this
example:

* RTTI is supported only for the `Animal` hierarchy. In the context of game
  programming, typically, RTTI would be supported only for classes derived from
  a "God" class like `UObject`.

* We can count on the value of the `static_type` variables to uniquely identify
  each type, so we don't need to provide a `type_index` function.

* The hierarchy uses virtual inheritance, and we plan to pass `Dog`s and `Cat`s
  to methods that take  `Animal`s. We must thus implement a `dynamic_cast_ref`
  function.


```c++
// for brevity
using namespace yorel::yomm2;

struct custom_rtti : policy::rtti {
    template<typename T>
    static type_id static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return reinterpret_cast<type_id>(T::static_type);
        } else {
            return 0;
        }
    }

    template<typename T>
    static type_id dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return reinterpret_cast<type_id>(obj.type);
        } else {
            return 0;
        }
    }

    template<class Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << (type == 0 ? "?" : reinterpret_cast<const char*>(type));
    }

    template<typename Derived, typename Base>
    static Derived dynamic_cast_ref(Base&& obj) {
        return custom_dynamic_cast<Derived>(obj);
    }
};
```


Now we need to create a policy which is the same as the default policy in every
respect, except for the `rtti` facet. For this, we use two templates, members of
policy classes:

* `template<class NewPolicy>` struct copy;<br>
    Facets within a policy can refer to one another, thus some facets need to
    access the policy they are contained in. `copy` takes the name of the new
    policy class, and copies all the facets the source policy contains. Facets
    that refer the policy are rebound to the new policy.

* `template<class FacetBase, NewFacet>` struct replace;<br>
    Return a policy class where the facet that inherits from `FacetBase` is
    replaced by `NewFacet`.

Thus we create the policy with:


```c++
struct custom_policy : default_policy::rebind<custom_policy>::replace<
                           policy::rtti, custom_rtti> {};
```


Finally, we must specify the new policy during class registration and method
declaration. Macros `register_classes` and `declare_method`, and core API
templates `use_classes` and `method` accept an addition policy argument.


```c++
register_classes(Animal, Dog, Cat, custom_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), custom_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}
```


The `update` function operates on the default policy. We need to call the
`update` function _template_. It takes a policy class as an explicit function
template argument:


```c++
BOOST_AUTO_TEST_CASE(custom_rtti_demo) {
    // Note: call update for our custom policy!
    yorel::yomm2::update<custom_policy>();

    Animal&& a = Dog("Snoopy");
    Animal&& b = Cat("Sylvester");

    {
        std::stringstream os;
        kick(a, os);
        BOOST_TEST(os.str() == "Snoopy barks.");
    }
    {
        std::stringstream os;
        kick(b, os);
        BOOST_TEST(os.str() == "Sylvester hisses.");
    }
}
```


## Taking advantage of custom RTTI specificities

In the previous example, we use `const char*` as type ids. The default policy
uses the addresses of `std::type_id` objects. In both cases, YOMM2 uses a fast,
collision-free hash function to map pointers to a small range of integer
indexes.

Some custom RTTI implementations use integers as type ids, and they are concentrated
in a fairly small range. For example:


```c++
struct Animal {
    Animal(const char* name, size_t type) : name(name), type(type) {
    }

    const char* name;
    size_t type;
    static constexpr size_t static_type = 1;
};

struct Dog : Animal {
    Dog(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static constexpr size_t static_type = 2;
};

struct Cat : Animal {
    Cat(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static constexpr size_t static_type = 3;
};
```


In this situation, we can save time on hashing. If a policy has a `type_hash`
facet (as is with the default policy), it is used to hash the `type_id`s to a
smaller range of integers. Otherwise, the id is used as a straight index in a
table that contains pointers to method tables for all registered classes.

Thus all we need to do is to remove the `type_hash` facet from the policy.

This is controlled by facet `type_hash`. It has two implementations:
`fast_perfect_hash`, used in release builds; and `checked_perfect_hash`, used
in debug builds, that checks that the type ids it is presented with correspond
to classes that were actually registered.

This time, virtual inheritance is not involved, so we dispense with
`dynamic_cast_ref`; we also use the default implementation of `type_name`.


```c++
struct custom_rtti : policy::rtti {
    template<typename T>
    static type_id static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return T::static_type;
        } else {
            return 0;
        }
    }

    template<typename T>
    static type_id dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return obj.type;
        } else {
            return 0;
        }
    }
};

struct custom_policy
    : policy::default_static::rebind<custom_policy>::replace<
          policy::rtti, custom_rtti>::remove<policy::type_hash> {};

register_classes(Animal, Dog, Cat, custom_policy);

declare_method(void, kick, (virtual_<Animal&>, std::ostream&), custom_policy);

define_method(void, kick, (Dog & dog, std::ostream& os)) {
    os << dog.name << " barks.";
}

define_method(void, kick, (Cat & cat, std::ostream& os)) {
    os << cat.name << " hisses.";
}
```


A call to `kick` now compiles to a shorter assembly code:

```asm
mov     rax, qword ptr [rdi + 8]
mov     rcx, qword ptr [rip + basic_domain<custom_policy>::context+24]
mov     rax, qword ptr [rcx + 8*rax]
mov     rcx, qword ptr [rip + method<custom_policy, kick, ...>::fn+80]
mov     rax, qword ptr [rax + 8*rcx]
jmp     rax
```

Namely, the multiplication and shift (i.e. the hash function), and reading the
hash factors, are gone.

## Dealing with static construction order

In the previous examples, type ids were hard-coded. It is unlikely to be the
case in a real custom RTTI system. More likely, type ids will be allocated at
static construction time, like this:


```c++
struct Animal {
    Animal(const char* name, size_t type) : name(name), type(type) {
    }

    const char* name;
    static size_t last_type_id;
    static size_t static_type;
    size_t type;
};

size_t Animal::last_type_id;
size_t Animal::static_type = ++Animal::last_type_id;

struct Dog : Animal {
    Dog(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static size_t static_type;
};

size_t Dog::static_type = ++Animal::last_type_id;

struct Cat : Animal {
    Cat(const char* name, size_t type = static_type) : Animal(name, type) {
    }

    static size_t static_type;
};

size_t Cat::static_type = ++Animal::last_type_id;
```


This is potentially a problem, because YOMM2 itself uses static constructors to
register classes, methods and definitions. In particular, `static_type` is
called at static construction time. There is no guarantee the  `static_type`
variables have already been initialized.

The solution is the `deferred_static_rtti` facet base. It tells YOMM2 to store a
pointer to the `static_type` functions; they will be called by `update`:


```c++
struct custom_rtti : policy::deferred_static_rtti {
    template<typename T>
    static type_id static_type() {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return T::static_type;
        } else {
            return 0;
        }
    }

    template<typename T>
    static type_id dynamic_type(const T& obj) {
        if constexpr (std::is_base_of_v<Animal, T>) {
            return obj.type;
        } else {
            return 0;
        }
    }
};
```


The only change is that the custom facet now inherits from
`deferred_static_rtti`. The rest of the code is as before.


