// Copyright (c) 2018-2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#define MAIN
#include "benchmark_gen.hpp"

int main(int argc, char** argv) {
    yorel::yomm2::update_methods();

    benchmark::RegisterBenchmark(
        "virtual function call", normal_ns::virtual_function);
    benchmark::RegisterBenchmark(
        "uni-method call", normal_ns::uni_method);
    benchmark::RegisterBenchmark(
        "double dispatch", normal_ns::double_dispatch);
    benchmark::RegisterBenchmark(
        "multi-method call", normal_ns::multi_method);

    // ------------------------------------------------------------------------

    benchmark::RegisterBenchmark(
        "virtual function call", virtual_ns::virtual_function);
    benchmark::RegisterBenchmark(
        "uni-method call (virtual inheritance)", virtual_ns::uni_method);
    benchmark::RegisterBenchmark(
        "double dispatch(virtual inheritance)", virtual_ns::double_dispatch);
    benchmark::RegisterBenchmark(
        "multi-method call (virtual inheritance)", virtual_ns::multi_method);

    // ------------------------------------------------------------------------

    benchmark::RegisterBenchmark(
        "virtual function call (hash info in gv)", hash_in_gv_ns::virtual_function);
    benchmark::RegisterBenchmark(
        "uni-method call (hash info in gv)", hash_in_gv_ns::uni_method);
    benchmark::RegisterBenchmark(
        "double dispatch (hash info in gv)", hash_in_gv_ns::double_dispatch);
    benchmark::RegisterBenchmark(
        "multi-method call (hash info in gv)", hash_in_gv_ns::multi_method);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}

#if 0

Run on (8 X 3500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x4)
  L1 Instruction 32K (x4)
  L2 Unified 256K (x4)
  L3 Unified 6144K (x1)

===================================================================================
g++ (Ubuntu 7.2.0-8ubuntu3) 7.2.0
-----------------------------------------------------------------------------------
Benchmark                                            Time           CPU Iterations
-----------------------------------------------------------------------------------
virtual function call                                2 ns          2 ns  388352068
uni-method call                                      2 ns          2 ns  338950548
double dispatch                                      2 ns          2 ns  296939274
multi-method call                                    3 ns          3 ns  213681025
virtual function call (virtual inheritance)          2 ns          2 ns  394059985
uni-method call (virtual inheritance)                2 ns          2 ns  336223393
double dispatch(virtual inheritance)                 3 ns          3 ns  259119960
multi-method call (virtual inheritance)              3 ns          3 ns  211531198

===================================================================================
clang version 5.0.0-3 (tags/RELEASE_500/final)
-----------------------------------------------------------------------------------
Benchmark                                            Time           CPU Iterations
-----------------------------------------------------------------------------------
virtual function call                                2 ns          2 ns  391357163
uni-method call                                      2 ns          2 ns  296146799
double dispatch                                      2 ns          2 ns  297146675
multi-method call                                    3 ns          3 ns  205128046
virtual function call (virtual inheritance)          2 ns          2 ns  395366829
uni-method call (virtual inheritance)                2 ns          2 ns  297651918
double dispatch(virtual inheritance)                 2 ns          2 ns  297348013
multi-method call (virtual inheritance)              3 ns          3 ns  204995332

#endif
