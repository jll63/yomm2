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

    virtual void pet() = 0;
};

class Cat : public Animal {
    void pet() override{/*purr*/};
};

register_classes(Animal, Cat);

declare_method(void, pet, (virtual_<Animal&>));

// Implement 'pet' for Cats.
define_method(void, pet, (Cat & Cat)) {
    // purr
}

declare_method(void, pet_vp, (virtual_ptr<Animal>));

// Implement 'pet' for Cats.
define_method(void, pet_vp, (virtual_ptr<Cat> Cat)) {
    // purr
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
    _mm_clflush(default_policy::static_vptr<Cat>);
    _mm_clflush(&default_policy::static_vptr<Cat>);
    _mm_clflush(default_policy::vptrs.data());
    _mm_clflush(&default_policy::vptrs);
    _mm_clflush(&default_policy::hash_mult);
    _mm_clflush(&default_policy::hash_shift);
    _mm_clflush(method_class(void, pet, (virtual_<Animal&>))::fn.slots_strides);
    _mm_clflush(
        method_class(void, pet_vp, (virtual_ptr<Animal>))::fn.slots_strides);
}

__attribute__((noinline)) auto overhead() {
    BENCHMARK({});
}

__attribute__((noinline)) auto virtual_function(Animal& a) {
    BENCHMARK(a.pet());
}

__attribute__((noinline)) auto method_via_vptr(virtual_ptr<Animal> ap) {
    BENCHMARK(pet_vp(ap));
}

__attribute__((noinline)) auto method_via_ref(Animal& a) {
    BENCHMARK(pet(a));
}

void test(
    std::string dispatch, std::string cache_temp, Animal& a,
    virtual_ptr<Animal> ap) {
    clear_cache(a);

    if (dispatch == "none") {
        cout << setw(6) << overhead();
    } else if (dispatch == "virtual") {
        cout << setw(6) << virtual_function(a);
    } else if (dispatch == "method_ref") {
        cout << setw(6) << method_via_ref(a);
    } else if (dispatch == "method_vptr") {
        cout << setw(6) << method_via_vptr(ap);
    } else {
        std::cerr << "invalid dispatch value\n";
        exit(1);
    }
}

int main(int, char** argv) {
    yorel::yomm2::update();

    Cat Cat;
    test(argv[1], argv[2] ? argv[2] : "", Cat, virtual_ptr(Cat));

    return 0;
}
