// md<
// <sub>/ ->home / ->reference </sub>
// ## yorel::yomm2::apply_product <small>(experimental)</small>
// <sub>defined in <yorel/yomm2/templates.hpp></sub>
// <!-- -->
// ---
// ```
// template<typename TemplateList, typename... TypeLists>
// using apply_product = /*unspecified*/;
// ```
// <!-- -->
// ---

// `apply_product` takes a ->templates list and list of ->types lists, and
// evaluates to a `types` list consisting of the application of each template to
// the n-fold Cartesian product of the input `types` lists.

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

template<typename, typename> struct bin1;
template<typename, typename> struct bin2;

static_assert(
    std::is_same_v<
        apply_product<
            templates<bin1, bin2>,
            types<a, b>, 
            types<x, y, z> 
        >,
        types<
            bin1<a, x>, bin1<a, y>, bin1<a, z>,
            bin1<b, x>, bin1<b, y>, bin1<b, z>,

            bin2<a, x>, bin2<a, y>, bin2<a, z>,
            bin2<b, x>, bin2<b, y>, bin2<b, z>
        >
    >);

// >

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>
#include <yorel/yomm2/keywords.hpp>

BOOST_AUTO_TEST_CASE(test) {
}
