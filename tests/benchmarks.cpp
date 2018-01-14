// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#define MAIN

#define INHERITANCE
#define PREFIX normal
#include "benchmark_matrices.hpp"

#undef INHERITANCE
#undef PREFIX

#define INHERITANCE virtual
#define PREFIX virtual
#include "benchmark_matrices.hpp"

int main(int argc, char** argv) {
    yorel::yomm2::update_methods();

    benchmark::RegisterBenchmark("virtual function call", normal_ns::virtual_function);
    benchmark::RegisterBenchmark("uni-method call", normal_ns::uni_method);
    benchmark::RegisterBenchmark("double dispatch", normal_ns::double_dispatch);
    benchmark::RegisterBenchmark("multi-method call", normal_ns::multi_method);

    benchmark::RegisterBenchmark("virtual function call (virtual inheritance)", virtual_ns::virtual_function);
    benchmark::RegisterBenchmark("uni-method call (virtual inheritance)", virtual_ns::uni_method);
    benchmark::RegisterBenchmark("double dispatch(virtual inheritance)", virtual_ns::double_dispatch);
    benchmark::RegisterBenchmark("multi-method call (virtual inheritance)", virtual_ns::multi_method);

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
virtual function call                                2 ns          2 ns  393105986
uni-method call                                      2 ns          2 ns  339690614
double dispatch                                      2 ns          2 ns  395433996
multi-method call                                    3 ns          3 ns  212230988
virtual function call (virtual inheritance)          2 ns          2 ns  394559425
uni-method call (virtual inheritance)                2 ns          2 ns  298056685
double dispatch(virtual inheritance)                 2 ns          2 ns  341053349
multi-method call (virtual inheritance)              3 ns          3 ns  212678531

===================================================================================
clang version 5.0.0-3 (tags/RELEASE_500/final)
-----------------------------------------------------------------------------------
Benchmark                                            Time           CPU Iterations
-----------------------------------------------------------------------------------
virtual function call                                2 ns          2 ns  393459076
uni-method call                                      2 ns          2 ns  335140869
double dispatch                                      2 ns          2 ns  296683771
multi-method call                                    3 ns          3 ns  208823581
virtual function call (virtual inheritance)          2 ns          2 ns  392972597
uni-method call (virtual inheritance)                2 ns          2 ns  296582445
double dispatch(virtual inheritance)                 2 ns          2 ns  340165177
multi-method call (virtual inheritance)              3 ns          3 ns  203740055

#endif
