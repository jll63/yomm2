// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>
#include <x86intrin.h>

#include <iostream>
#include <iomanip>

using std::cout;
using std::setw;
using namespace yorel::yomm2;

class Animal {
  public:
    virtual ~Animal() {
    }

    virtual void kick() = 0;
};

class Dog : public Animal {
    void kick() override{};
};

register_classes(Animal, Dog);

declare_method(void, kick, (virtual_<Animal&>));

// Implement 'kick' for dogs.
define_method(void, kick, (Dog & dog)) {
}

declare_method(void, kick_vp, (virtual_ptr<Animal>));

// Implement 'kick' for dogs.
define_method(void, kick_vp, (virtual_ptr<Dog> dog)) {
}

#define BENCHMARK_N(expr, N)                                                   \
    {                                                                          \
        unsigned int dummy;                                                    \
        _mm_mfence();                                                          \
        _mm_lfence();                                                          \
        auto start = __rdtsc();                                                \
        for (auto i = N; i--;) {                                               \
            expr;                                                              \
        }                                                                      \
        auto end = __rdtscp(&dummy);                                           \
        _mm_lfence();                                                          \
        cout << setw(6) << (end - start);                                      \
    }

#define BENCHMARK(expr)                                                        \
    unsigned int dummy;                                                        \
    _mm_mfence();                                                              \
    _mm_lfence();                                                              \
    auto start = __rdtsc();                                                    \
    expr;                                                                      \
    auto end = __rdtscp(&dummy);                                               \
    _mm_lfence();                                                              \
    return end - start;

__attribute__((noinline)) void clear_cache(Animal& a) {
    _mm_clflush(&a);
    _mm_clflush(*(void**)&a);
    _mm_clflush(default_policy::static_vptr<Dog>);
    _mm_clflush(&default_policy::static_vptr<Dog>);
    _mm_clflush(default_policy::vptrs.data());
    _mm_clflush(&default_policy::vptrs);
    _mm_clflush(&default_policy::hash_mult);
    _mm_clflush(&default_policy::hash_shift);
    _mm_clflush(
        method_class(void, kick, (virtual_<Animal&>))::fn.slots_strides_p);
    _mm_clflush(
        &method_class(void, kick, (virtual_<Animal&>))::fn.slots_strides_p);
    _mm_clflush(
        method_class(void, kick_vp, (virtual_ptr<Animal>))::fn.slots_strides_p);
    _mm_clflush(&method_class(
                     void, kick_vp, (virtual_ptr<Animal>))::fn.slots_strides_p);
}

__attribute__((noinline)) auto overhead() {
    BENCHMARK(0);
}

__attribute__((noinline)) auto
virtual_function(Animal& a) {
    BENCHMARK(a.kick());
}

__attribute__((noinline)) auto
method_via_vptr(virtual_ptr<Animal> ap) {
    BENCHMARK(kick_vp(ap));
}

__attribute__((noinline)) auto
method_via_ref(Animal& a) {
    BENCHMARK(kick(a));
}

void test(int dispatch, Animal& a, virtual_ptr<Animal> ap) {
    clear_cache(a);

    switch (dispatch) {
    case 0:
        cout << "\n";
        break;
    case 1:
        cout << setw(6) << overhead();
        break;
    case 2:
        cout << setw(6) << virtual_function(a);
        break;
    case 3:
        cout << setw(6) << method_via_ref(a);
        break;
    case 4:
        cout << setw(6) << method_via_vptr(ap);
        break;
    case 5:
        BENCHMARK_N(0, 100);
        break;
    case 6:
        BENCHMARK_N(a.kick(), 100);
        break;
    case 7:
        BENCHMARK_N(kick(a), 100);
        break;
    case 8:
        BENCHMARK_N(kick_vp(ap), 100);
        break;
    default:
        std::cerr << "invalid\n";
        exit(1);
    }
}

int main(int, char** argv) {
    yorel::yomm2::update();
    Dog dog;
    test(atoi(argv[1]), dog, virtual_ptr(dog));

    return 0;
}
