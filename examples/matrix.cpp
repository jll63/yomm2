#include <memory>

#include <yorel/yomm2/cute.hpp>

using std::shared_ptr;
using std::make_shared;
using yorel::yomm2::virtual_;

struct matrix {
    virtual ~matrix() {}
};

struct dense_matrix : matrix {};
struct diagonal_matrix : matrix {};

//auto log = yorel::yomm2::details::log_on(&std::cerr);

register_class(matrix);
register_class(dense_matrix, matrix);
register_class(diagonal_matrix, matrix);

declare_method(
    shared_ptr<matrix>,
    times,
    virtual_<std::shared_ptr<matrix>>, virtual_<std::shared_ptr<matrix>>);

declare_method(shared_ptr<matrix>, times, double, virtual_<std::shared_ptr<const matrix>>);
declare_method(shared_ptr<matrix>, times, virtual_<std::shared_ptr<matrix>>, double);

begin_method(shared_ptr<matrix>, times, std::shared_ptr<matrix>, std::shared_ptr<matrix>) {
    return make_shared<matrix>();
} end_method;

begin_method(shared_ptr<matrix>, times, std::shared_ptr<diagonal_matrix>, std::shared_ptr<diagonal_matrix>) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(shared_ptr<matrix>, times, double a, std::shared_ptr<const matrix> m) {
    return make_shared<matrix>();
} end_method;

begin_method(shared_ptr<matrix>, times, double a, std::shared_ptr<const diagonal_matrix> m) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(shared_ptr<matrix>, times, std::shared_ptr<diagonal_matrix> m, double a) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(shared_ptr<matrix>, times, std::shared_ptr<matrix> m, double a) {
    return make_shared<matrix>();
} end_method;

#define check(expr) {if (!(expr)) {std::cerr << #expr << " failed\n";}}

int main() {
    yorel::yomm2::update_methods();
    shared_ptr<matrix> a = make_shared<dense_matrix>();
    shared_ptr<matrix> b = make_shared<diagonal_matrix>();
    check(dynamic_cast<const diagonal_matrix*>(times(2, b).get()));
    return 0;
}
