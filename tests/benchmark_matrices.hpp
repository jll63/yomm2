// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <benchmark/benchmark.h>

using namespace yorel::yomm2;

#define PREFIXED(ID) BOOST_PP_CAT(PREFIX, BOOST_PP_CAT(_, ID))

namespace PREFIXED(ns) {

struct dense_matrix;
struct diagonal_matrix;

struct matrix {
    virtual ~matrix() {}
    virtual double times(double) const = 0;
    virtual double times(const matrix& other) const = 0;
    virtual double times2(const dense_matrix& other) const;
    virtual double times2(const diagonal_matrix& other) const;
};

struct dense_matrix : INHERITANCE matrix {
    virtual double times(double) const;
    virtual double times(const matrix& other) const;
    virtual double times2(const dense_matrix& other) const;
};

struct diagonal_matrix : INHERITANCE matrix {
    virtual double times(double) const;
    virtual double times(const matrix& other) const;
    virtual double times2(const diagonal_matrix& other) const;
};

YOMM2_DECLARE(
    double,
    times,
    (double, yorel::yomm2::virtual_<const matrix&>),
    POLICY);

YOMM2_DECLARE(
    double,
    times,
    (yorel::yomm2::virtual_<const matrix&>, yorel::yomm2::virtual_<const matrix&>),
    POLICY);

#ifdef MAIN

// -----------------------------------------------------------------------------

YOMM2_CLASS(matrix);
YOMM2_CLASS(dense_matrix, matrix);
YOMM2_CLASS(diagonal_matrix, matrix);

YOMM2_DEFINE(double, times, (double a, const matrix& m)) {
    return 0;
}

YOMM2_DEFINE(double, times, (double a, const diagonal_matrix& m)) {
    return 0;
}

// -----------------------------------------------------------------------------

YOMM2_DEFINE(double, times, (const matrix&, const matrix&)) {
    return 1;
}

YOMM2_DEFINE(double, times, (const diagonal_matrix&, const diagonal_matrix&)) {
    return 2;
}

// -----------------------------------------------------------------------------

double call_virtual_function(double s, const matrix& m);
double call_uni_method(double s, const matrix& m);
double call_double_dispatch(const matrix& a, const matrix& b);
double call_multi_method(const matrix& a, const matrix& b);

void virtual_function(benchmark::State& state) {
    const matrix& m = dense_matrix();
    for (auto _ : state) {
        call_virtual_function(2, m);
    }
}

void uni_method(benchmark::State& state) {
    const matrix& m = dense_matrix();
    for (auto _ : state) {
        call_uni_method(2, m);
    }
}

void double_dispatch(benchmark::State& state) {
    const matrix& a = dense_matrix();
    const matrix& b = diagonal_matrix();
    for (auto _ : state) {
        call_double_dispatch(a, b);
    }
}

void multi_method(benchmark::State& state) {
    const matrix& a = dense_matrix();
    const matrix& b = diagonal_matrix();
    for (auto _ : state) {
        call_multi_method(a, b);
    }
}

#else

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

double call_double_dispatch(const matrix& a, const matrix& b) {
    return a.times(b);
}

double call_multi_method(const matrix& a, const matrix& b) {
    return times(a, b);
}

#endif

}
