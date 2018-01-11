#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using yorel::yomm2::virtual_;

namespace matrices {

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(void, times, virtual_<const matrix&>, virtual_<const matrix&>);
YOMM2_DECLARE(void, times, double, virtual_<const matrix&>);
YOMM2_DECLARE(void, times, virtual_<const matrix&>, double);

YOMM2_DEFINE(void, times, const matrix&, const matrix&) {
    std::cout << "matrix * matrix\n";
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix&, const diagonal_matrix&) {
    std::cout << "diagonal_matrix * diagonal_matrix\n";
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const matrix& m) {
    std::cout << "double * matrix\n";
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const diagonal_matrix& m) {
    std::cout << "double * diagonal_matrix\n";
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix& m, double a) {
    std::cout << "diagonal_matrix * double\n";
} YOMM2_END;

YOMM2_DEFINE(void, times, const matrix& m, double a) {
    std::cout << "matrix * double\n";
} YOMM2_END;

BOOST_AUTO_TEST_CASE(compilation)
{
    yorel::yomm2::update_methods();
    const matrix& dense = dense_matrix();
    const matrix& diag = diagonal_matrix();
    times(dense, dense);
    times(diag, diag);
    times(diag, dense);
    times(2, dense);
    times(dense, 2);
    times(diag, 2);
}

}
