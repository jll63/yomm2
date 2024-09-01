// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/templates.hpp>

// clang-format off

using namespace yorel::yomm2;

namespace test_product {

template<int N>
struct n;

static_assert(
    std::is_same_v<
        product<
            types<n<1>, n<2> >,
            types< n<3>, n<4>, n<5> >
        >,
        types<
            types<n<1>, n<3>>,
            types<n<1>, n<4>>,
            types<n<1>, n<5>>,
            types<n<2>, n<3>>,
            types<n<2>, n<4>>,
            types<n<2>, n<5>>
        >
    >);

template<typename, typename> struct bin1;
template<typename, typename> struct bin2;

static_assert(
    std::is_same_v<
        apply_product<
            templates<bin1, bin2>,
            types<n<1>, n<2>>,
            types<n<3>, n<4>, n<5>>
        >,
        types<
            bin1<n<1>, n<3>>,
            bin1<n<1>, n<4>>,
            bin1<n<1>, n<5>>,
            bin1<n<2>, n<3>>,
            bin1<n<2>, n<4>>,
            bin1<n<2>, n<5>>,
            bin2<n<1>, n<3>>,
            bin2<n<1>, n<4>>,
            bin2<n<1>, n<5>>,
            bin2<n<2>, n<3>>,
            bin2<n<2>, n<4>>,
            bin2<n<2>, n<5>>
        >
    >);

template<typename T>
struct ordinary_matrix;

template<typename T>
struct square_matrix;

template<typename T>
using matrix_types = types<ordinary_matrix<T>, square_matrix<T>>;

template<typename T1, typename T2>
struct add;

template<typename T1, typename T2>
struct multiply;

template<typename T1, typename T2>
using binary_operations = product<
    apply_product<
        templates<add, multiply>,
        types<T1>,
        types<T2>
    >,
    matrix_types<T1>,
    matrix_types<T2>
>;

static_assert(
    std::is_same_v<
        transform_product<
            binary_operations,
            types<int, double>,
            types<int, double>
        >,
        types<
            types<add<int, int>, ordinary_matrix<int>, ordinary_matrix<int>>,
            types<add<int, int>, ordinary_matrix<int>, square_matrix<int>>,
            types<add<int, int>, square_matrix<int>, ordinary_matrix<int>>,
            types<add<int, int>, square_matrix<int>, square_matrix<int>>,
            types<multiply<int, int>, ordinary_matrix<int>, ordinary_matrix<int>>,
            types<multiply<int, int>, ordinary_matrix<int>, square_matrix<int>>,
            types<multiply<int, int>, square_matrix<int>, ordinary_matrix<int>>,
            types<multiply<int, int>, square_matrix<int>, square_matrix<int>>,
            types<add<int, double>, ordinary_matrix<int>, ordinary_matrix<double>>,
            types<add<int, double>, ordinary_matrix<int>, square_matrix<double>>,
            types<add<int, double>, square_matrix<int>, ordinary_matrix<double>>,
            types<add<int, double>, square_matrix<int>, square_matrix<double>>,
            types<multiply<int, double>, ordinary_matrix<int>, ordinary_matrix<double>>,
            types<multiply<int, double>, ordinary_matrix<int>, square_matrix<double>>,
            types<multiply<int, double>, square_matrix<int>, ordinary_matrix<double>>,
            types<multiply<int, double>, square_matrix<int>, square_matrix<double>>,
            types<add<double, int>, ordinary_matrix<double>, ordinary_matrix<int>>,
            types<add<double, int>, ordinary_matrix<double>, square_matrix<int>>,
            types<add<double, int>, square_matrix<double>, ordinary_matrix<int>>,
            types<add<double, int>, square_matrix<double>, square_matrix<int>>,
            types<multiply<double, int>, ordinary_matrix<double>, ordinary_matrix<int>>,
            types<multiply<double, int>, ordinary_matrix<double>, square_matrix<int>>,
            types<multiply<double, int>, square_matrix<double>, ordinary_matrix<int>>,
            types<multiply<double, int>, square_matrix<double>, square_matrix<int>>,
            types<add<double, double>, ordinary_matrix<double>, ordinary_matrix<double>>,
            types<add<double, double>, ordinary_matrix<double>, square_matrix<double>>,
            types<add<double, double>, square_matrix<double>, ordinary_matrix<double>>,
            types<add<double, double>, square_matrix<double>, square_matrix<double>>,
            types<multiply<double, double>, ordinary_matrix<double>, ordinary_matrix<double>>,
            types<multiply<double, double>, ordinary_matrix<double>, square_matrix<double>>,
            types<multiply<double, double>, square_matrix<double>, ordinary_matrix<double>>,
            types<multiply<double, double>, square_matrix<double>, square_matrix<double>>>
    >);

}

namespace test_add_definition {

struct method {
    using self_type = method;
    template<typename Container> struct add_definition;
};

template<typename...>
struct definition {};

static_assert(std::is_same_v<
    detail::use_definition<definition>::fn<types<method, int>>,
    method::add_definition<definition<method, int>>

>);
}

int main() {
}
