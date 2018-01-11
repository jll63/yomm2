
#include "benchmark_matrices.hpp"

double dense_matrix::times(double a) const {
    return a * a;
}

double diagonal_matrix::times(double a) const {
    return a * a;
}
