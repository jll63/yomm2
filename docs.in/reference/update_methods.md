entry: update_methods
headers:yorel/yomm2/core.hpp, yorel/yomm2/keywords.hpp

Initialize the data used during method dispatch.

This function must be called before any method is called (typically in `main`).
It must also be called after a shared library is dynamically loaded or unloaded,
if the library adds method declarations, method definitions, or classes derived
from classes that are used as virtual arguments.

### Example

```c++
int main() {
    yorel::yomm2::update_methods();
    // call methods

    // if using dynamically loaded libraries
    void* handle = dlopen("mylib.so", RTLD_NOW);
    yorel::yomm2::update_methods();
    // classes, methods, and definitions from mylib.so are available

    dlclose(handle);
    yorel::yomm2::update_methods();
    // classes, methods, and definitions from mylib.so are no longer available

    return 0;
}
```
