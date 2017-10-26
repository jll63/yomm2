#include <iostream>
#include <type_traits>

#include <yorel/openmethods.hpp>

using std::is_same;
namespace yomm = yorel::openmethods;

struct Matrix {
    virtual ~Matrix() {}
};

struct Dense_matrix : Matrix {};
struct Diagonal_matrix : Matrix {};

declare_method(void, times, const Matrix*, const Matrix*);
declare_method(void, times, double, const Matrix*);
declare_method(void, times, const Matrix*, double);

begin_method(void, times, const Matrix*, const Matrix*) {
} end_method;

int main()
{
    const Matrix& dense = Dense_matrix();
    const Matrix& diag = Diagonal_matrix();
    times(&dense, &dense);
    times(2, &dense);
    times(&dense, 2);
    times(&diag, &dense);
    times(&diag, &diag);
}
