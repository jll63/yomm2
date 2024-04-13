# YOMM2_STATIC
<sub>defined in <yorel/yomm2/symbols.hpp>, also provided by <yorel/yomm2/keywords.hpp></sub><br/>

```c++
#define YOMM2_STATIC(...) static __VA_ARGS__ YOMM2_GENSYM
```

`YOMM2_STATIC` provides a convenient way to create a static object just for
executing its constructor at static initialization time. The macro creates an
obfuscated name for the object, which is unlikely to clash with any other name.

### Example

```c++
// Instantiate a 'use_classes' object to register three classes.
YOMM2_STATIC(yorel::yomm2::use_classes<Animal, Cat, Dog>);
```
