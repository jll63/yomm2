entry: update
headers:yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

```c++
/* compiler */ update();                             (1) (since 1.6.0)
/* compiler */ template<class Policy> void update(); (2) (since 1.6.0)

void update();                                       (3) (until 1.6.0)
template<class Policy>void update();                 (4) (until 1.6.0)
```
Initialize the data used during method dispatch.

This function must be called before any method is called (typically in `main`).
It must also be called after a shared library is dynamically loaded or unloaded,
if the library adds method declarations, method definitions, or classes derived
from classes that are used as virtual arguments.

(1) and (3) operate on the default policy. (2) and (4) operate on the specified
policy.

Since version 1.6.0, `update` returns a "compiler" object of an unspecified
type, which contains information gathered while compiling dispatch data. The
only documented member is `report`, a struct containing the following values:

| Name            | Description                                                                      |
| --------------- | -------------------------------------------------------------------------------- |
| cells           | total number of cells used by v-tables and multi-method dispatch tables          |
| not_implemented | total number of argument combinations with no applicable definition              |
| ambiguous       | total number of argument combinations that cannot be resolved due to ambiguities |



```c++
int main() {
    yorel::yomm2::update();
    // call methods

    // if using dynamically loaded libraries
    void* handle = dlopen("mylib.so", RTLD_NOW);
    yorel::yomm2::update();
    // classes, methods, and definitions from mylib.so are available

    dlclose(handle);
    yorel::yomm2::update();
    // classes, methods, and definitions from mylib.so are no longer available

    return 0;
}
```
