<sub> ->home / ->reference </sub>

entry: friend_method
headers: yorel/yomm2/cute.hpp, yorel/yomm2/keywords.hpp

entry: YOMM2_FRIEND
headers: yorel/yomm2/macros.hpp, yorel/yomm2.hpp

---
```
#define friend_method(container) /*unspecified*/
#define friend_method(container, return_type, name, (function-parameter-list)) /*unspecified*/
```
---
Grant friendship to all the methods inside a container friend of a class, or to
a specific method. See [containers](examples/containers) for an example.

## example
See the [containers](../examples/containers) example.
