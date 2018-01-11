#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#include <benchmark/benchmark.h>

#include "benchmark_matrices.hpp"

using yorel::yomm2::virtual_;

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DECLARE(double, times, virtual_<const matrix&>, virtual_<const matrix&>);
YOMM2_DECLARE(double, times, double, virtual_<const matrix&>);

YOMM2_DEFINE(double, times, const matrix&, const matrix&) {
    return 0;
} YOMM2_END;

YOMM2_DEFINE(double, times, const diagonal_matrix&, const diagonal_matrix&) {
    return 0;
} YOMM2_END;

YOMM2_DEFINE(double, times, double a, const matrix& m) {
    return a * a;
} YOMM2_END;

YOMM2_DEFINE(double, times, double a, const diagonal_matrix& m) {
    return a * a;
} YOMM2_END;

void CallVirtualFunction(benchmark::State& state) {
    const matrix& dense = dense_matrix();
    for (auto _ : state) {
        dense.times(2);
    }
}

BENCHMARK(CallVirtualFunction);

void CallUniMethod(benchmark::State& state) {
    const matrix& dense = dense_matrix();
    for (auto _ : state) {
        times(2, dense);
    }
}

BENCHMARK(CallUniMethod);

int main(int argc, char** argv) {
    yorel::yomm2::update_methods();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
