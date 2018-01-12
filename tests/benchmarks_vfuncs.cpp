
#include "benchmark_matrices.hpp"

double dense_matrix::times(double a) const {
    return 1;
}

double diagonal_matrix::times(double a) const {
    return 2;
}

// -----------------------------------------------------------------------------

double matrix::times2(const dense_matrix& other) const {
    return 1;
}

double matrix::times2(const diagonal_matrix& other) const {
    return 1;
}

// -----------------------------------------------------------------------------

double dense_matrix::times(const matrix& other) const {
    return other.times2(*this);
}

double dense_matrix::times2(const dense_matrix& other) const {
    return 1;
}

// -----------------------------------------------------------------------------

double diagonal_matrix::times(const matrix& other) const {
    return other.times2(*this);
}

double diagonal_matrix::times2(const diagonal_matrix& other) const {
    return 2;
}

// -----------------------------------------------------------------------------

double call_virtual_function(double s, const matrix& m) {
    return m.times(s);
}

double call_uni_method(double s, const matrix& m){
    return times(s, m);
}
