#include <memory>
#include <string>

#include <yorel/yomm2/cute.hpp>

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::cout;
using yorel::yomm2::virtual_;

struct matrix {
    virtual ~matrix() {}
        // ...
};

struct dense_matrix    : matrix { /* ... */ };
struct diagonal_matrix : matrix { /* ... */ };

//auto log = yorel::yomm2::detail::log_on(&cerr);

register_class(matrix);
register_class(dense_matrix, matrix);
register_class(diagonal_matrix, matrix);

declare_method(string, to_json, virtual_<const matrix&>);

begin_method(string, to_json, const dense_matrix& m) {
    return "json for dense matrix...";
} end_method;

begin_method(string, to_json, const diagonal_matrix& m) {
    return "json for diagonal matrix...";
} end_method;

declare_method(
    shared_ptr<const matrix>,
    times,
    virtual_<shared_ptr<const matrix>>, virtual_<shared_ptr<const matrix>>);

declare_method(
    shared_ptr<const matrix>,
    times,
    double, virtual_<shared_ptr<const matrix>>);

declare_method(
    shared_ptr<const matrix>,
    times,
    virtual_<shared_ptr<const matrix>>, double);

begin_method(
    shared_ptr<const matrix>,
    times,
    shared_ptr<const matrix>, shared_ptr<const matrix>) {
    return make_shared<matrix>();
} end_method;

begin_method(
    shared_ptr<const matrix>,
    times,
    shared_ptr<const diagonal_matrix>, shared_ptr<const diagonal_matrix>) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(
    shared_ptr<const matrix>,
    times,
    double a, shared_ptr<const matrix> m) {
    return make_shared<matrix>();
} end_method;

begin_method(
    shared_ptr<const matrix>,
    times,
    double a, shared_ptr<const diagonal_matrix> m) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(
    shared_ptr<const matrix>,
    times,
    shared_ptr<const diagonal_matrix> m, double a) {
    return make_shared<diagonal_matrix>();
} end_method;

begin_method(
    shared_ptr<const matrix>,
    times,
    shared_ptr<const matrix> m, double a) {
    return make_shared<matrix>();
} end_method;

#define check(expr) {if (!(expr)) {cerr << #expr << " failed\n";}}

int main() {
    yorel::yomm2::update_methods();

    shared_ptr<const matrix> a = make_shared<dense_matrix>();
    shared_ptr<const matrix> b = make_shared<diagonal_matrix>();

    cout << to_json(*a) << "\n"; // json for dense matrix
    cout << to_json(*b) << "\n"; // json for diagonal matrix

    return 0;
}
