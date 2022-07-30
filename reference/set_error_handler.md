
<sub>/ [home](/README.md) / [reference](README.md) </sub>
## yorel::yomm2::hash_search_error
## yorel::yomm2::resolution_error
## yorel::yomm2::unknown_class_error
## yorel::yomm2::error_type
## yorel::yomm2::error_handler_type
## yorel::yomm2::set_error_handler
<sub>defined in header <yorel/yomm2/core.hpp>; also provided by
<yorel/yomm2/keywords.hpp>, <yorel/yomm2.hpp></sub>
<!-- -->
---
```
struct resolution_error {
    enum status_type { no_definition = 1, ambiguous } status;
    /*unspecified*/
};

struct unknown_class_error { /*unspecified*/ };

struct hash_search_error { /*unspecified*/ };

using error_type = std::variant<
    resolution_error, unknown_class_error, hash_search_error>;

using error_handler_type = void (*)(const error_type& error);

error_handler_type set_error_handler(error_handler_type handler);
```
---
All errors are reported via an indirect call to a handler, passing it a
`std::variant` that identifies the specific error. The handler can be set to
a user-defined function with `set_error_handler`. The library calls `abort()`
immediately after calling the handler, but the handler can prevent program
termination by throwing an exception. The default handler writes an error
message to `std::cerr` in debug mode.

The handler can determine the exact type of the error by examining the
variant:
- `hash_search_error`: [update_methods](update_methods.md) could not find a hash function for
  the registered classes.
- `resolution_error`: there is no applicable definition for the arguments of
  a method call or (next)[method.md#add_definition], or it is ambiguous; the
  `status` field is set accordingly
- `unknown_class_error`: a class that has not been registered is used as a
  base in [register_class](register_class.md), or in a method declaration or definition, or (in
  debug mode only) as an argument for a method call

`set_error_handler` returns the previous handler.

## example


```c++

#include <stdexcept>
#include <yorel/yomm2/keywords.hpp>

struct Dog {
    virtual ~Dog() {
    }
};

register_classes(Dog);

declare_method(void, kick, (virtual_<Dog&>));

using namespace yorel::yomm2; // for brevity

void (*next_error_handler)(const error_type& ev);

void no_definition_handler(const error_type& ev) {
    if (auto error = std::get_if<resolution_error>(&ev)) {
        if (error->status == resolution_error::no_definition) {
            throw std::runtime_error("not defined");
        }
    }
    
    next_error_handler(ev);
}

BOOST_AUTO_TEST_CASE(set_error_handler_example) {
    next_error_handler = set_error_handler(no_definition_handler);
    update_methods();

    try {
        Dog snoopy;
        kick(snoopy);
    } catch (const std::runtime_error& error) {
        BOOST_TEST(error.what() == "not defined");
        return;
    }
    BOOST_FAIL("did not throw");
}


```
