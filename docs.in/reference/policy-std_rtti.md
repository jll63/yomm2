entry: policy::std_rtti
headers: yorel/yomm2/policy.hpp, yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
struct std_rtti;
```

Implement `rtti` using the RTTI facilities provided by standard C++.

## Static member functions

| Name                                               | Description                                     |
| -------------------------------------------------- | ----------------------------------------------- |
| type_id [**static_type\<T>**](#static_type)        | return a `type_id` for `T`                      |
| type_id [**dynamic_type\<T>**](#dynamic_type)      | return a `type_id` for an object's dynamic type |
| void [**type_name\<Stream>**](#type_name)          | write a description of `type_id` to a stream    |
| *unspecified* [**type_index**](#type_index)        | return a unique key for a `type_id`             |
| D [**dynamic_cast_ref\<D, B>**](#dynamic_cast_ref) | cast from base class to derived class           |

### static_type

```c++
template<class Class>
static type_id static_type();
```

Return `&typeid(Class)`, cast to a `type_id`.

**Template parameters**

* Class: a class registered via ->register_classes or ->use_classes.

### dynamic_type

```c++
template<class Class>
static type_id dynamic_type(const Class& obj);
```

Return `&typeid(obj)`, cast to a `type_id`.

**Template parameters**

* Class: a class registered via ->register_classes or ->use_classes.

### type_name

```c++
template<typename Stream>
static void type_name(type_id type, Stream& stream);
```

Execute `stream << reinterpret_cast<const std::type_info*>(type)->name()`.

**Template parameters**

* `Stream`: a model of ->`RestrictedOutputStream`.

**Function parameters**

* `type`: the type id of the class to describe.
* `stream`: the stream to write the description to.


### type_index

```c++
static /*unspecified*/ type_index(type_id type);
```

Return `std::type_index(*reinterpret_cast<const std::type_info*>(type))`.

The function is required because C++ does *not* guarantee that there is a single
instance of `std::type_info` for each specific type. `update` builds a map
associating the `std::type_index`s to a set of `type_id`s, thus ensuring proper
operation, even in the (unlikely) case that some types have multiple `type_info`
objects, and thus `static_type` and `dynamic_type` do not return the same value
for the same class.

**Function parameters**

* `type`: the type id of a class

### dynamic_cast_ref

```c++
template<typename Derived, typename Base>
static Derived dynamic_cast_ref(Base&& obj);
```

Cast `obj` using the `dynamic_cast` operator. Note that YOMM2 will use a
`static_cast` whenever possible.

**Template parameters**

* `Base`: a registered class.  `Base&&` is guaranteed to evaluate to a
  reference
* `Derived`: a registered class, derived from `Base`.

**Function parameters**

* `obj`: the object to cast.
  to a `Base` object.
