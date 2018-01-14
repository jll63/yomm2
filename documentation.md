# Documentation

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

### macros declare_method, YOMM2_DECLARE

#### Synopsis:
```
declare_method(return_type, method_name, parameter_1, ..., parammeter_n)
```



### template virtual_

#### Synopsis:
```
```

### macros begin_method, YOMM2_METHOD

#### Synopsis:
```
```

### next

#### Synopsis:
```
```

### macros end_method, YOMM2_END

#### Synopsis:
```
```

### macros register_class, YOMM2_CLASS

#### Synopsis:
```
```

### Semantics

### Dependencies

* a C++17 capable compiler

* The following Boost libraries: Preprocessor, DynamicBitset, TypeTraits

* For tests: Boost.Test version 1.65

* cmake version 3.5
