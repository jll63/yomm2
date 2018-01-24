// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

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

YOMM2_DECLARE(Subtype, times, (virtual_<const matrix&>, virtual_<const matrix&>));
YOMM2_DECLARE(Subtype, times, (double, virtual_<const matrix&>));
YOMM2_DECLARE(Subtype, times, (virtual_<const matrix&>, double));

YOMM2_DEFINE(Subtype, times, (const matrix&, const matrix&)) {
    return MATRIX_MATRIX;
}

YOMM2_DEFINE(Subtype, times, (const diagonal_matrix&, const diagonal_matrix&)) {
    return DIAGONAL_DIAGONAL;
}

YOMM2_DEFINE(Subtype, times, (double a, const matrix& m)) {
    return SCALAR_MATRIX;
}

YOMM2_DEFINE(Subtype, times, (double a, const diagonal_matrix& m)) {
    return SCALAR_DIAGONAL;
}

YOMM2_DEFINE(Subtype, times, (const diagonal_matrix& m, double a)) {
    return DIAGONAL_SCALAR;
}

YOMM2_DEFINE(Subtype, times, (const matrix& m, double a)) {
    return MATRIX_SCALAR;
}

BOOST_AUTO_TEST_CASE(simple)
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

namespace errors {

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(void, times, (virtual_<const matrix&>, virtual_<const matrix&>));

YOMM2_DEFINE(void, times, (const diagonal_matrix&, const matrix&)) {
}

YOMM2_DEFINE(void, times, (const matrix&, const diagonal_matrix&)) {
}

int error_code = -1;
_YOMM2_DEBUG(std::string method_name);

void test_handler(const yorel::yomm2::method_call_error& error) {
    error_code = error.code;
    _YOMM2_DEBUG(method_name = error.method_name);
}

BOOST_AUTO_TEST_CASE(error_handling)
{
    yorel::yomm2::update_methods();
    yorel::yomm2::set_method_call_error_handler(test_handler);
    times(dense_matrix(), dense_matrix());
    BOOST_TEST(error_code == yorel::yomm2::method_call_error::not_implemented);
    _YOMM2_DEBUG(
        BOOST_TEST(
            method_name == "times(virtual_<const matrix&>, virtual_<const matrix&>)"));
    _YOMM2_DEBUG(method_name = "");
    times(diagonal_matrix(), diagonal_matrix());
    BOOST_TEST(error_code == yorel::yomm2::method_call_error::ambiguous);
    _YOMM2_DEBUG(
        BOOST_TEST(
            method_name == "times(virtual_<const matrix&>, virtual_<const matrix&>)"));
}

}
