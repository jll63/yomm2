#ifdef YOMM2_MD
headers : yorel / yomm2 / core.hpp, yorel / yomm2.hpp,
    yorel /
    yomm2.hpp

```c++ error_handler_type set_error_handler(error_handler_type handler);
```

    All errors are reported via an indirect call to a handler,
    passing it a
`std::variant` that identifies the specific error.The
        handler can be set to a user -
    defined function with `set_error_handler`.The
        library calls `abort()` immediately after calling the handler,
    but the handler can prevent program termination by throwing an exception
        .The default handler writes an error message to `stderr` in debug mode
        .

`set_error_handler` returns the previous handler.

    ##Example

#endif

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>

#ifdef YOMM2_CODE

#include <stdexcept>
#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

    struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(Animal, Dog);

declare_method(poke, (virtual_<Animal&>), void);

using namespace yorel::yomm2; // for brevity

default_policy::error_handler_type next_error_handler;

void no_definition_handler(const default_policy::error_variant& ev) {
    if (auto error = std::get_if<resolution_error>(&ev)) {
        if (error->status == resolution_error::no_definition) {
            throw std::runtime_error("not defined");
        }
    }

    next_error_handler(ev);
}

BOOST_AUTO_TEST_CASE(ref_set_error_handler_example) {
    next_error_handler =
        default_policy::set_error_handler(no_definition_handler);
    initialize();

    try {
        Dog snoopy;
        poke(snoopy);
    } catch (const std::runtime_error& error) {
        BOOST_TEST(error.what() == "not defined");
        return;
    }

    BOOST_FAIL("did not throw");
}

#endif
