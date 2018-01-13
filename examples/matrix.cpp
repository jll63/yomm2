#include <memory>

#include <yorel/yomm2.hpp>

using std::shared_ptr;
using std::make_shared;
using yorel::yomm2::virtual_;

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

//auto log = yorel::yomm2::details::log_on(&std::cerr);

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(shared_ptr<matrix>, times, virtual_<const matrix*>, virtual_<const matrix*>);
YOMM2_DECLARE(shared_ptr<matrix>, times, double, virtual_<const matrix*>);
YOMM2_DECLARE(shared_ptr<matrix>, times, virtual_<const matrix*>, double);

YOMM2_DEFINE(shared_ptr<matrix>, times, const matrix*, const matrix*) {
    return make_shared<matrix>();
} YOMM2_END;

YOMM2_DEFINE(shared_ptr<matrix>, times, const diagonal_matrix*, const diagonal_matrix*) {
    return make_shared<diagonal_matrix>();
} YOMM2_END;

YOMM2_DEFINE(shared_ptr<matrix>, times, double a, const matrix* m) {
    return make_shared<matrix>();
} YOMM2_END;

YOMM2_DEFINE(shared_ptr<matrix>, times, double a, const diagonal_matrix* m) {
    return make_shared<diagonal_matrix>();
} YOMM2_END;

YOMM2_DEFINE(shared_ptr<matrix>, times, const diagonal_matrix* m, double a) {
    return make_shared<diagonal_matrix>();
} YOMM2_END;

YOMM2_DEFINE(shared_ptr<matrix>, times, const matrix* m, double a) {
    return make_shared<matrix>();
} YOMM2_END;

#define check(expr) {if (!(expr)) {std::cerr << #expr << " failed\n";}}

int main() {
    yorel::yomm2::update_methods();
    shared_ptr<matrix> a = make_shared<dense_matrix>();
    shared_ptr<matrix> b = make_shared<diagonal_matrix>();
    check(dynamic_cast<const diagonal_matrix*>(times(2, b.get()).get()));
    return 0;
}
