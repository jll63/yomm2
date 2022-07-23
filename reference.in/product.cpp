// md<
// <sub>/ ->home / ->reference </sub>
// ## yorel::yomm2::product <small>(experimental)</small>
// <sub>defined in <yorel/yomm2/templates.hpp></sub>
// <!-- -->
// ---
// ```
// template<typename... TypeLists>
// using product = /*unspecified*/;
// ```
// <!-- -->
// ---

// `product` takes a list of ->types lists, and evaluates to a `types` list
// consisting of the n-fold Cartesian product of the input lists.

// ## example
// >

// clang-format off

// code<
#include <type_traits>
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/templates.hpp>

using namespace yorel::yomm2;

struct a;
struct b;
struct x;
struct y;
struct z;

static_assert(
    std::is_same_v<
        product< 
            types<a, b>, 
            types<x, y, z> 
        >,
        types<
            types<a, x>, types<a, y>, types<a, z>,
            types<b, x>, types<b, y>, types<b, z>
        >
    >);

// >

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>
#include <yorel/yomm2/keywords.hpp>

BOOST_AUTO_TEST_CASE(test) {
}
