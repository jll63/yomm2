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
using boost::mp11::mp_list;

namespace test_product {

template<int N>
struct n;

static_assert(
    std::is_same_v<
        product<
            mp_list<n<1>, n<2> >,
            mp_list< n<3>, n<4>, n<5> >
        >,
        mp_list<
            mp_list<n<1>, n<3>>,
            mp_list<n<1>, n<4>>,
            mp_list<n<1>, n<5>>,
            mp_list<n<2>, n<3>>,
            mp_list<n<2>, n<4>>,
            mp_list<n<2>, n<5>>
        >
    >);

template<typename, typename> struct bin1;
template<typename, typename> struct bin2;

static_assert(
    std::is_same_v<
        apply_product<
            templates<bin1, bin2>,
            mp_list<n<1>, n<2>>,
            mp_list<n<3>, n<4>, n<5>>
        >,
        mp_list<
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
using matrix_types = mp_list<ordinary_matrix<T>, square_matrix<T>>;

template<typename T1, typename T2>
struct add;

template<typename T1, typename T2>
struct multiply;

template<typename T1, typename T2>
using binary_operations = product<
    apply_product<
        templates<add, multiply>,
        mp_list<T1>,
        mp_list<T2>
    >,
    matrix_types<T1>,
    matrix_types<T2>
>;

static_assert(
    std::is_same_v<
        transform_product<
            binary_operations,
            mp_list<int, double>,
            mp_list<int, double>
        >,
        mp_list<
            mp_list<add<int, int>, ordinary_matrix<int>, ordinary_matrix<int>>,
            mp_list<add<int, int>, ordinary_matrix<int>, square_matrix<int>>,
            mp_list<add<int, int>, square_matrix<int>, ordinary_matrix<int>>,
            mp_list<add<int, int>, square_matrix<int>, square_matrix<int>>,
            mp_list<multiply<int, int>, ordinary_matrix<int>, ordinary_matrix<int>>,
            mp_list<multiply<int, int>, ordinary_matrix<int>, square_matrix<int>>,
            mp_list<multiply<int, int>, square_matrix<int>, ordinary_matrix<int>>,
            mp_list<multiply<int, int>, square_matrix<int>, square_matrix<int>>,
            mp_list<add<int, double>, ordinary_matrix<int>, ordinary_matrix<double>>,
            mp_list<add<int, double>, ordinary_matrix<int>, square_matrix<double>>,
            mp_list<add<int, double>, square_matrix<int>, ordinary_matrix<double>>,
            mp_list<add<int, double>, square_matrix<int>, square_matrix<double>>,
            mp_list<multiply<int, double>, ordinary_matrix<int>, ordinary_matrix<double>>,
            mp_list<multiply<int, double>, ordinary_matrix<int>, square_matrix<double>>,
            mp_list<multiply<int, double>, square_matrix<int>, ordinary_matrix<double>>,
            mp_list<multiply<int, double>, square_matrix<int>, square_matrix<double>>,
            mp_list<add<double, int>, ordinary_matrix<double>, ordinary_matrix<int>>,
            mp_list<add<double, int>, ordinary_matrix<double>, square_matrix<int>>,
            mp_list<add<double, int>, square_matrix<double>, ordinary_matrix<int>>,
            mp_list<add<double, int>, square_matrix<double>, square_matrix<int>>,
            mp_list<multiply<double, int>, ordinary_matrix<double>, ordinary_matrix<int>>,
            mp_list<multiply<double, int>, ordinary_matrix<double>, square_matrix<int>>,
            mp_list<multiply<double, int>, square_matrix<double>, ordinary_matrix<int>>,
            mp_list<multiply<double, int>, square_matrix<double>, square_matrix<int>>,
            mp_list<add<double, double>, ordinary_matrix<double>, ordinary_matrix<double>>,
            mp_list<add<double, double>, ordinary_matrix<double>, square_matrix<double>>,
            mp_list<add<double, double>, square_matrix<double>, ordinary_matrix<double>>,
            mp_list<add<double, double>, square_matrix<double>, square_matrix<double>>,
            mp_list<multiply<double, double>, ordinary_matrix<double>, ordinary_matrix<double>>,
            mp_list<multiply<double, double>, ordinary_matrix<double>, square_matrix<double>>,
            mp_list<multiply<double, double>, square_matrix<double>, ordinary_matrix<double>>,
            mp_list<multiply<double, double>, square_matrix<double>, square_matrix<double>>>
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
    detail::use_definition<definition>::fn<mp_list<method, int>>,
    method::add_definition<definition<method, int>>

>);
}

int main() {
}
