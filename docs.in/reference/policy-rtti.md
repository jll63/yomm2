entry: policy::rtti, policy::deferred_static_rtti
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp


```c++
struct rtti;

struct deferred_static_rtti : virtual rtti;
```

A `rtti` facet provides type information for classes and objects, implements
downcast in presence of virtual inheritance, and writes descriptions of types to
an `ostream`-like object.

A `deferred_static_rtti`, derived from `rtti`, directs YOMM2 to defer collection
of static type information until `update` is called. This makes it possible to
interface with custom RTTI systems that use static constructors to assign type
information.

## Requirements for implementations of **rtti**

| Name                                               | Description                                     | Required                     |
| -------------------------------------------------- | ----------------------------------------------- | ---------------------------- |
| type_id [**static_type\<T>**](#static_type)        | return a `type_id` for `T`                      | yes                          |
| type_id [**dynamic_type\<T>**](#dynamic_type)      | return a `type_id` for an object's dynamic type | depending on context         |
| void [**type_name\<Stream>**](#type_name)          | write a description of `type_id` to a stream    | no                           |
| *unspecified* [**type_index**](#type_index)        | return a unique key for a `type_id`             | depending on context         |
| D [**dynamic_cast_ref\<D, B>**](#dynamic_cast_ref) | cast from base class to derived class           | if using virtual inheritance |

(all members are static)

### static_type

```c++
template<class Class>
static type_id static_type();
```

Return a type id for `Class`.

**Template parameters**

* **Class**: a class registered via ->register_classes or ->use_classes.

### dynamic_type

```c++
template<class Class>
static type_id dynamic_type(const Class& obj);
```

Return a type id for `obj`. May be omitted if using final `virtual_ptr`s only.
Required.

**Template parameters**

* **Class**: a class registered via ->register_classes or ->use_classes.

### type_name

```c++
template<typename Stream>
static void type_name(type_id type, Stream& stream);
```

Write a description of the class identified by `type` to `stream`. Not required.
`rtti` provides a default implementation that prints `type_id(type)`.
Implementations should provide a more readable description, if possible.

**Template parameters**

* **Stream**: a model of ->`RestrictedOutputStream`.

**Function parameters**

* **type**: the type id of the class to describe.
* **stream**: the stream to write the description to.


### type_index

```c++
static /*unspecified*/ type_index(type_id type);
```

Return an unspecified object that uniquely identifies a class. Required if
`static_type` can return different type ids for the same class. See
->`policy-std_rtti` for an example of this.

**Function parameters**

* **type**: the type id of a class

### dynamic_cast_ref

```c++
template<typename Derived, typename Base>
static Derived dynamic_cast_ref(Base&& obj);
```

Cast `obj` to `Derived`, a subclass of `Base`. Required if virtual inheritance
is used in the registered classes.

**Template parameters**

* **Base**: a registered class.  `Base&&` is guaranteed to evaluate to a
  reference.
* **Derived**: a registered class, derived from `Base`.

**Function parameters**

* **obj**: the object to cast.
  to a `Base` object.

## Implementations of **rtti**

|                      |                      |
| -------------------- | -------------------- |
| ->policy-std_rtti    | use standard RTTI    |
| ->policy-minimal_rtti | use static RTTI only |
