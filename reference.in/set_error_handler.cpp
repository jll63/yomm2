#ifdef YOMM2_MD
<sub> /->home /
           ->reference</ sub>

               entry : yorel::yomm2::error_type entry
    : yorel::yomm2::error_handler_type entry
    : yorel::yomm2::set_error_handler entry
    : yorel::yomm2::hash_search_error entry
    : yorel::yomm2::resolution_error entry
    : yorel::yomm2::unknown_class_error headers : yorel /
    yomm2 / core.hpp,
    yorel / yomm2 / keywords.hpp,
    yorel /
        yomm2.hpp

        -- -
``` struct resolution_error {
    enum status_type { no_definition = 1, ambiguous } status;
    /*unspecified*/
};

struct unknown_class_error { /*unspecified*/
};

struct hash_search_error { /*unspecified*/
};

using error_type =
    std::variant<resolution_error, unknown_class_error, hash_search_error>;

using error_handler_type = void (*)(const error_type& error);

error_handler_type set_error_handler(error_handler_type handler);
```
---
All errors are reported via an indirect call to a handler, passing it a
`std::variant` that identifies the specific error. The handler can be set to
a user-defined function with `set_error_handler`. The library calls `abort()`
immediately after calling the handler, but the handler can prevent program
termination by throwing an exception. The default handler writes an error
message to `stderr` in debug mode.

The handler can determine the exact type of the error by examining the
variant:
- `hash_search_error`: ->update could not find a hash function for
  the registered classes.
- `resolution_error`: there is no applicable definition for the arguments of
  a method call or (next)[method.md#add_definition], or it is ambiguous; the
  `status` field is set accordingly
- `unknown_class_error`: a class that has not been registered is used as a
  base in ->register_class, or in a method declaration or definition, or (in
  debug mode only) as an argument for a method call

`set_error_handler` returns the previous handler.

## example

#endif

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_CODE

    #include <stdexcept>
    #include <yorel/yomm2/keywords.hpp>

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(Animal, Dog);

declare_method(void, kick, (virtual_<Animal&>));

using namespace yorel::yomm2; // for brevity

error_handler_type next_error_handler;

void no_definition_handler(const error_type& ev) {
    if (auto error = std::get_if<resolution_error>(&ev)) {
        if (error->status == resolution_error::no_definition) {
            if (error->tis[0] == &typeid(Dog)) {
                throw std::runtime_error("not defined");
            } else {
                throw std::runtime_error(
                    std::string("wrong typeid: ") + error->tis[0]->name());
            }
        }
    }

    next_error_handler(ev);
}

BOOST_AUTO_TEST_CASE(reference_set_error_handler_example) {
    next_error_handler = set_error_handler(no_definition_handler);
    update();

    try {
        Dog snoopy;
        kick(snoopy);
    } catch (const std::runtime_error& error) {
        BOOST_TEST(error.what() == "not defined");
        return;
    }

    BOOST_FAIL("did not throw");
}

#endif
