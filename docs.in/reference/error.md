entry: error, error_type, error_handler_type, unknown_class_error, hash_search_error, method_table_error, resolution_error
headers: yorel/yomm2/core.hpp,yorel/yomm2/keywords.hpp

```c++
struct error {
};

struct unknown_class_error : error {
    enum { update = 1, call } context;
    type_id type;
};

struct hash_search_error : error {
    size_t attempts;
    size_t buckets;
};

struct method_table_error : error {
    type_id type;
};

struct resolution_error : error {
    enum status_type { no_definition = 1, ambiguous } status;
    std::string_view method_name;
    size_t arity;
    static constexpr size_t max_types = 16;
    type_id types[max_types];
};

using error_type = std::variant<
    resolution_error,
    unknown_class_error,
    hash_search_error,
    method_table_error
>;


using error_handler_type = void (*)(const error_type& error);
```

Classes derived from `error` are used to describe various error conditions.

| Name                                            | Description                           |
| ----------------------------------------------- | ------------------------------------- |
| [**unknown_class_error**](#unknown_class_error) | class has not been registered         |
| [**hash_search_error**](#hash_search_error)     | hash function not found               |
| [**method_table_error**](#method_table_error)   | wrong class for virtual_ptr::final    |
| [**resolution_error**](#resolution_error)       | method call is undefined or ambiguous |

## unknown_class_error

YOMM2 detected that a class used in a method declaration or definition, or a
method call, was not registered via ->use_classes or ->register_classes.

| Member variable                       | Description                                                             |
| ------------------------------------- | ----------------------------------------------------------------------- |
| enum { update = 1, call } **context** | where the error was detected (during `update` or during a method call ) |
| type_id **type**                      | type id of the class                                                    |

## hash_search_error

Random search failed to find the parameters of the hash function.

| Member variable     | Description                                      |
| ------------------- | ------------------------------------------------ |
| size_t **attempts** | how many attempts were made during random search |
| size_t **buckets**  | maximum number of buckets tried                  |

## method_table_error

The class passed to `virtual_ptr::final` is not the same as the object's actual
class.

| Member variable  | Description          |
| ---------------- | -------------------- |
| type_id **type** | type id of the class |

## resolution_error

A single applicable definition could not be found for a method call.

| Member variable                  | Description                                             |
| -------------------------------- | ------------------------------------------------------- |
| status_type **status**           | no_definition, ambiguous                                |
| std::string_view **method_name** | a the method name                                       |
| size_t **arity**                 | number of virtual parameters, and size of `types`       |
| size_t **max_types**             | the maximum number of type ids in `types`               |
| const type_id* **types**         | the type ids of the virtual arguments (up to max_types) |
