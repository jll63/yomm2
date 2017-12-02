#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>

using std::is_same;
using yorel::yomm2::virtual_;

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

YOMM2_DECLARE(void, times, virtual_<const matrix&>, virtual_<const matrix&>);
YOMM2_DECLARE(void, times, double, virtual_<const matrix&>);
YOMM2_DECLARE(void, times, virtual_<const matrix&>, double);

YOMM2_DEFINE(void, times, const matrix&, const matrix&) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix&, const diagonal_matrix&) {
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const diagonal_matrix& m) {
} YOMM2_END;

YOMM2_DEFINE(void, times, double a, const matrix& m) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const diagonal_matrix& m, double a) {
} YOMM2_END;

YOMM2_DEFINE(void, times, const matrix& m, double a) {
} YOMM2_END;

int main()
{
    yorel::yomm2::update_methods();

    const matrix& dense = dense_matrix();
    const matrix& diag = diagonal_matrix();
    times(dense, dense);
    times(2, dense);
    times(dense, 2);
    times(diag, dense);
    times(diag, diag);
}
