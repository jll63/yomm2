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
