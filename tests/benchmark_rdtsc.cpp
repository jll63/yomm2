// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <x86intrin.h>

#include <cstddef>
#include <iostream>
#include <iomanip>

using std::cout;
using std::setw;
using namespace yorel::yomm2;
using namespace policies;

struct intrusive : default_policy::rebind<intrusive>::remove<
                       external_vptr>::remove<type_hash> {
    template<class Class>
    static auto dynamic_vptr(const Class& arg) {
        return arg.yomm2_vptr;
    }
};

struct std_unordered_map
    : default_policy::rebind<std_unordered_map>::replace<
          external_vptr, vptr_map<std_unordered_map>>::remove<type_hash> {};

#if __has_include(<boost/unordered/unordered_flat_map.hpp>)

#include <boost/unordered/unordered_flat_map.hpp>
#define FLAT_MAP_AVAILABLE

struct flat_std_unordered_map
    : default_policy::rebind<flat_std_unordered_map>::replace<
          external_vptr,
          vptr_map<
              flat_std_unordered_map,
              boost::unordered_flat_map<type_id, const std::uintptr_t*>>>::
          remove<type_hash> {};

#endif

namespace stat {

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

    void pet_vf() override { /*purr*/ };
};

struct YOMM2_SYMBOL(pet_ref);
struct YOMM2_SYMBOL(pet_vp);
struct YOMM2_SYMBOL(pet_iptr);

} // namespace stat

namespace dyn {

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

    void pet_vf() override { /*purr*/ };
};

} // namespace dyn

template<class Policy>
struct use_policy {
    use_policy() {
        YOMM2_STATIC(
            use_classes<
                dyn::Animal, dyn::Cat, stat::Animal, stat::Cat, Policy>);
        initialize<Policy>();
    }
};

std::vector<std::pair<
    std::string,
    unsigned (*)(
        dyn::Animal&, virtual_ptr<dyn::Animal>, stat::Animal&,
        virtual_ptr<stat::Animal>)>>
    benchmarks;

struct register_benchmark {
    explicit register_benchmark(
        std::string name,
        unsigned (*fun)(
            dyn::Animal&, virtual_ptr<dyn::Animal>, stat::Animal&,
            virtual_ptr<stat::Animal>)) {
        benchmarks.emplace_back(name, fun);
    }
};

#define BENCHMARK(NAME, EXPR)                                                  \
    __attribute__((noinline)) unsigned NAME(                                   \
        dyn::Animal& dyn_ref, virtual_ptr<dyn::Animal> dyn_vp,                 \
        stat::Animal& stat_ref, virtual_ptr<stat::Animal> stat_vp) {           \
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

BENCHMARK(vf, dyn_ref.pet_vf());

// -----------------------------------------------------------------------------
// others

namespace dyn {

// -----------------------------------------------------------------------------
// method, argument passed by reference

declare_method(void, pet_ref, (virtual_<Animal&>));

// Implement 'pet_ref' for Cats.
define_method(void, pet_ref, (Cat & Cat)) {
    // purr
}

BENCHMARK(ref, pet_ref(dyn_ref));

// -----------------------------------------------------------------------------
// method, argument passed by virtual_ptr

declare_method(void, pet_vp, (virtual_ptr<Animal>));

// Implement 'pet_vp' for Cats.
define_method(void, pet_vp, (virtual_ptr<Cat> cat)) {
    // purr
}

BENCHMARK(vp, pet_vp(dyn_vp));

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
BENCHMARK(iptr, pet_iptr(dyn_ref));

// -----------------------------------------------------------------------------
// std_unordered_map

declare_method(void, pet_sum, (virtual_<Animal&>), std_unordered_map);

// Implement 'pet_sum' for Cats.
define_method(void, pet_sum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<std_unordered_map>);
BENCHMARK(sum, pet_sum(dyn_ref));

// -----------------------------------------------------------------------------
// boost::flat_std_unordered_map

#ifdef FLAT_MAP_AVAILABLE

declare_method(void, pet_fum, (virtual_<Animal&>), flat_std_unordered_map);

// Implement 'pet_fum' for Cats.
define_method(void, pet_fum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<flat_std_unordered_map>);
BENCHMARK(fum, pet_fum(dyn_ref));

#endif

} // namespace dyn

namespace stat {

// -----------------------------------------------------------------------------
// method, argument passed by reference

declare_method(void, pet_ref, (virtual_<Animal&>));

// Implement 'pet_ref' for Cats.
define_method(void, pet_ref, (Cat & Cat)) {
    // purr
}

BENCHMARK(stat_ref, pet_ref(stat_ref));

// -----------------------------------------------------------------------------
// method, argument passed by virtual_ptr

declare_method(void, pet_vp, (virtual_ptr<Animal>));

// Implement 'pet_vp' for Cats.
define_method(void, pet_vp, (virtual_ptr<Cat> cat)) {
    // purr
}

BENCHMARK(stat_vp, pet_vp(stat_vp));

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
BENCHMARK(stat_iptr, pet_iptr(stat_ref));

// -----------------------------------------------------------------------------
// std_unordered_map

declare_method(void, pet_sum, (virtual_<Animal&>), std_unordered_map);

// Implement 'pet_sum' for Cats.
define_method(void, pet_sum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<std_unordered_map>);
BENCHMARK(stat_sum, pet_sum(stat_ref));

// -----------------------------------------------------------------------------
// boost::flat_std_unordered_map

#ifdef FLAT_MAP_AVAILABLE

declare_method(void, pet_fum, (virtual_<Animal&>), flat_std_unordered_map);

// Implement 'pet_fum' for Cats.
define_method(void, pet_fum, (Cat & Cat)) {
    // purr
}

YOMM2_STATIC(use_policy<flat_std_unordered_map>);
BENCHMARK(stat_fum, pet_fum(stat_ref));

#endif

} // namespace stat

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
    dyn::Cat dyn_ref;
    auto dyn_vp = virtual_ptr(dyn_ref);
    stat::Cat stat_ref;
    auto stat_vp = virtual_ptr(stat_ref);

    std::string what = "all";
    bool flush = true;
    int count = 10;

    auto arg = argv + 1;

    if (*arg) {
        what = *arg++;
    }

    // if (what == "generate") {
    //     boost::mp11::mp_for_each<detail::types<
    //         default_policy, intrusive, std_unordered_map,
    //         flat_std_unordered_map>>([](auto Policy) {
    //         {
    //             compiler<decltype(Policy)> comp;
    //             comp.compile();
    //             comp.write_static_offsets(std::cout);
    //         }
    //     });

    //     return 0;
    // }

    if (*arg) {
        count = std::stoi(*arg++);
    }

    if (*arg) {
        flush = !strcmp(*arg++, "flush");
    }

    if (what == "all") {
        for (auto& benchmark : benchmarks) {
            cout << setw(10) << benchmark.first;
        }
        cout << "\n";

        if (!flush) {
            for (auto& benchmark : benchmarks) {
                benchmark.second(dyn_ref, dyn_vp, stat_ref, stat_vp);
            }
        }

        for (auto i = count; i--;) {
            for (auto& benchmark : benchmarks) {
                if (flush) {
                    flush_cache();
                }

                cout << setw(10)
                     << benchmark.second(dyn_ref, dyn_vp, stat_ref, stat_vp);
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
            benchmark->second(dyn_ref, dyn_vp, stat_ref, stat_vp);
        }

        cout << setw(10)
             << benchmark->second(dyn_ref, dyn_vp, stat_ref, stat_vp);
    }

    return 0;
}
