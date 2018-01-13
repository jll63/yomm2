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

enum Subtype {
    SCALAR_MATRIX, SCALAR_DIAGONAL, MATRIX_SCALAR, DIAGONAL_SCALAR,
  MATRIX_MATRIX, DIAGONAL_DIAGONAL
};

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(Subtype, times, virtual_<const matrix&>, virtual_<const matrix&>);
YOMM2_DECLARE(Subtype, times, double, virtual_<const matrix&>);
YOMM2_DECLARE(Subtype, times, virtual_<const matrix&>, double);

YOMM2_DEFINE(Subtype, times, const matrix&, const matrix&) {
    return MATRIX_MATRIX;
} YOMM2_END;

YOMM2_DEFINE(Subtype, times, const diagonal_matrix&, const diagonal_matrix&) {
    return DIAGONAL_DIAGONAL;
} YOMM2_END;

YOMM2_DEFINE(Subtype, times, double a, const matrix& m) {
    return SCALAR_MATRIX;
} YOMM2_END;

YOMM2_DEFINE(Subtype, times, double a, const diagonal_matrix& m) {
    return SCALAR_DIAGONAL;
} YOMM2_END;

YOMM2_DEFINE(Subtype, times, const diagonal_matrix& m, double a) {
    return DIAGONAL_SCALAR;
} YOMM2_END;

YOMM2_DEFINE(Subtype, times, const matrix& m, double a) {
    return MATRIX_SCALAR;
} YOMM2_END;

BOOST_AUTO_TEST_CASE(compilation)
{
    //yorel::yomm2::detail::log_on(&std::cerr);
    yorel::yomm2::update_methods();
    //yorel::yomm2::detail::log_off();
    const matrix& dense = dense_matrix();
    const matrix& diag = diagonal_matrix();
    BOOST_TEST(times(dense, dense) == MATRIX_MATRIX);
    BOOST_TEST(times(diag, diag) == DIAGONAL_DIAGONAL);
    BOOST_TEST(times(diag, dense) == MATRIX_MATRIX);
    BOOST_TEST(times(2, dense) == SCALAR_MATRIX);
    BOOST_TEST(times(dense, 2) == MATRIX_SCALAR);
    BOOST_TEST(times(diag, 2) == DIAGONAL_SCALAR);
}

}
