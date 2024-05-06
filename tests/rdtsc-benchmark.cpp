// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>
#include <x86intrin.h>

#include <cstddef>
#include <iostream>
#include <iomanip>

using std::cout;
using std::setw;
using namespace yorel::yomm2;
using namespace policy;

struct intrusive : default_policy::rebind<intrusive>::remove<
                       external_vptr>::remove<type_hash> {
    template<class Class>
    static auto dynamic_vptr(const Class& arg) {
        return arg.yomm2_vptr;
    }
};

struct Animal {
    virtual ~Animal() {
    }

    virtual void pet_vf() = 0;

    const std::uintptr_t* yomm2_vptr = 0;
};

struct Cat : Animal {
    Cat() {
        yomm2_vptr = intrusive::static_vptr<Cat>;
    }

    void pet_vf() override{/*purr*/};
};

template<class Policy>
struct use_policy {
    use_policy() {
        YOMM2_STATIC(use_classes<Animal, Cat, Policy>);
        update<Policy>();
    }
};

std::vector<
    std::pair<std::string, unsigned (*)(Animal& r, virtual_ptr<Animal> vp)>>
    benchmarks;

struct register_benchmark {
    explicit register_benchmark(
        std::string name, unsigned (*fun)(Animal& r, virtual_ptr<Animal> vp)) {
        benchmarks.emplace_back(name, fun);
    }
};

#define BENCHMARK(NAME, EXPR)                                                  \
    __attribute__((noinline)) unsigned NAME(                                   \
        Animal& r, virtual_ptr<Animal> vp) {                                   \
        unsigned int dummy;                                                    \
        _mm_sfence();                                                          \
        _mm_lfence();                                                          \
        auto start = __rdtscp(&dummy);                                         \
        EXPR;                                                                  \
        auto end = __rdtscp(&dummy);                                           \
        _mm_sfence();                                                          \
        _mm_lfence();                                                          \
        return end - start;                                                    \
    }                                                                          \
    register_benchmark YOMM2_GENSYM(#NAME, NAME)

// -----------------------------------------------------------------------------
// ovh

BENCHMARK(ovh, {});

// -----------------------------------------------------------------------------
// virtual function

BENCHMARK(vf, r.pet_vf());

// -----------------------------------------------------------------------------
// method, argument passed by reference

declare_method(void, pet_ref, (virtual_<Animal&>));

// Implement 'pet_ref' for Cats.
define_method(void, pet_ref, (Cat & Cat)) {
    // purr
}

BENCHMARK(ref, pet_ref(r));

// -----------------------------------------------------------------------------
// method, argument passed by virtual_ptr

declare_method(void, pet_vp, (virtual_ptr<Animal>));

// Implement 'pet_vp' for Cats.
define_method(void, pet_vp, (virtual_ptr<Cat> cat)) {
    // purr
}

BENCHMARK(vp, pet_vp(vp));

// ref and vp
YOMM2_STATIC(use_policy<default_policy>);

// -----------------------------------------------------------------------------
// intrusive

declare_method(void, pet_iptr, (virtual_<Animal&>), intrusive);

// Implement 'pet_iptr' for Cats.
define_method(void, pet_iptr, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<intrusive>);
BENCHMARK(iptr, pet_iptr(r));

// -----------------------------------------------------------------------------
// std::unordered_map

struct using_unordered_map
    : default_policy::rebind<using_unordered_map>::replace<
          external_vptr, vptr_map<using_unordered_map>>::remove<type_hash> {};

declare_method(void, pet_sum, (virtual_<Animal&>), using_unordered_map);

// Implement 'pet_sum' for Cats.
define_method(void, pet_sum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<using_unordered_map>);
BENCHMARK(sum, pet_sum(r));

// -----------------------------------------------------------------------------
// boost::flat_unordered_map

#if __has_include(<boost/unordered/unordered_flat_map.hpp>)

#include <boost/unordered/unordered_flat_map.hpp>

struct flat_unordered_map
    : default_policy::rebind<flat_unordered_map>::replace<
          external_vptr,
          vptr_map<
              flat_unordered_map,
              boost::unordered_flat_map<type_id, const std::uintptr_t*>>>::
          remove<type_hash> {};

declare_method(void, pet_fum, (virtual_<Animal&>), flat_unordered_map);

// Implement 'pet_fum' for Cats.
define_method(void, pet_fum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<flat_unordered_map>);
BENCHMARK(fum, pet_fum(r));

#endif

// -----------------------------------------------------------------------------
// driver

__attribute__((noinline)) void flush_cache() {
    std::vector<int> v1, v2;

    for (int n = 1024 * 1024; n--;) {
        v1.push_back(n);
        v2.push_back(n);
    }

    for (int i = 0; i < v1.size(); ++i) {
        v1[i] += v2[i];
    }
}

int main(int argc, char** argv) {
    Cat r;
    auto vp = virtual_ptr(r);

    std::string what = "all";
    bool flush = true;
    int count = 100;

    auto arg = argv + 1;

    if (*arg) {
        what = *arg++;
    }

    if (*arg) {
        count = std::stoi(*arg++);
    }

    if (*arg) {
        flush = !strcmp(*arg++, "flush");
    }

    if (what == "all") {
        for (auto& benchmark : benchmarks) {
            cout << setw(6) << benchmark.first;
        }
        cout << "\n";

        if (!flush) {
            for (auto& benchmark : benchmarks) {
                benchmark.second(r, vp);
            }
        }

        for (auto i = count; i--;) {
            for (auto& benchmark : benchmarks) {
                if (flush) {
                    flush_cache();
                }

                cout << setw(6) << benchmark.second(r, vp);
            }
            cout << "\n";
        }
    } else {
        auto benchmark =
            std::find_if(benchmarks.begin(), benchmarks.end(), [what](auto bm) {
                return bm.first == what;
            });

        if (benchmark == benchmarks.end()) {
            std::cerr << "unknown benchmark " << what << "\n";
            exit(1);
        }

        if (flush) {
            flush_cache();
        } else {
            benchmark->second(r, vp);
        }

        cout << setw(6) << benchmark->second(r, vp);
    }

    return 0;
}
