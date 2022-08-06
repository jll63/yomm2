// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>

#include <benchmark/benchmark.h>
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/templates.hpp>

#include "benchmarks_parameters.hpp"

using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;
using namespace boost::mp11;

#if defined(YOMM2_BENCHMARK_HIERARCHIES)
enum { NH = YOMM2_BENCHMARK_HIERARCHIES };
#elif defined(NDEBUG)
enum { NH = 100 };
#else
enum { NH = 5 };
#endif

const std::string yomm2_ = "yomm2_";

auto OBJECTS() {
    static size_t n = 1000;

    if (auto objects_from_env = getenv("YOMM2_BENCHMARKS_OBJECTS")) {
        n = std::stoi(objects_from_env);
    }

    return n;
}

std::default_random_engine alloc_rnd;
std::uniform_int_distribution<std::size_t> alloc_dist{0, 500};

template<typename N, typename Inheritance, typename Work>
struct Dog;

template<typename N, typename Inheritance, typename Work>
struct Cat;

struct vfunc {};
struct baseline {};
template<size_t N>
using int_ = std::integral_constant<size_t, N>;
using policies = types<hash_factors_in_globals, hash_factors_in_method>;

struct ordinary_base {
    template<class Base>
    struct base : Base {};
};

struct virtual_base {
    template<class Base>
    struct base : virtual Base {};
};

using inheritance_types = types<ordinary_base, virtual_base>;

struct no_work {
    using return_type = void;
    static void fn(std::uintptr_t, std::uintptr_t) {
    }
};

struct some_work {
    static std::uintptr_t fn(std::uintptr_t a, std::uintptr_t b) {
        auto x = 1;

        for (int i = 0; i < 8; ++i) {
            if (a & 1) {
                x += 1;
            }

            if (b & 1) {
                x *= 2;
            }

            a >>= 1;
            b >>= 1;
        }

        return x;
    }

    using return_type = decltype(
        fn(std::declval<std::uintptr_t>(), std::declval<std::uintptr_t>())
    );
};

using work_types = types<no_work, some_work>;

template<typename N, typename Inheritance, typename Work>
struct Animal {
    using work_type = Work;

    void* operator new(size_t size) {
        return ::operator new(size + alloc_dist(alloc_rnd));
    }

    virtual ~Animal() {
    }

    virtual typename Work::return_type kick() = 0;
    virtual typename Work::return_type meet(Animal& other) = 0;
    virtual typename Work::return_type meet(Dog<N, Inheritance, Work>& other) = 0;
    virtual typename Work::return_type meet(Cat<N, Inheritance, Work>& other) = 0;
};

template<typename N, typename Inheritance, typename Work>
struct Dog : Inheritance::template base<Animal<N, Inheritance, Work>> {
    virtual typename Work::return_type kick() {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(this));
    }

    virtual typename Work::return_type meet(Animal<N, Inheritance, Work>& other) {
        return other.meet(*this);
    }

    virtual typename Work::return_type meet(Dog<N, Inheritance, Work>& other) {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(&other));
    }

    virtual typename Work::return_type meet(Cat<N, Inheritance, Work>& other) {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(&other));
    }
};

template<typename N, typename Inheritance, typename Work>
struct Cat : Inheritance::template base<Animal<N, Inheritance, Work>> {
    virtual typename Work::return_type kick() {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(this));
    }

    virtual typename Work::return_type meet(Animal<N, Inheritance, Work>& other) {
        return other.meet(*this);
    }

    virtual typename Work::return_type meet(Dog<N, Inheritance, Work>& other) {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(&other));
    }

    virtual typename Work::return_type meet(Cat<N, Inheritance, Work>& other) {
        return Work::fn(std::uintptr_t(this), std::uintptr_t(&other));
    }
};

static_assert(std::is_base_of_v<
    Animal<int_<1>, ordinary_base, no_work>, Dog<int_<1>, ordinary_base, no_work>
 >);

template<typename N, typename Inheritance, typename Work>
struct animal_class_definitions : 
    class_declaration<Animal<N, Inheritance, Work>>,
    class_declaration<Dog<N, Inheritance, Work>, Animal<N, Inheritance, Work>>,
    class_declaration<Cat<N, Inheritance, Work>, Animal<N, Inheritance, Work>> {
};

mp_apply<
    aggregate,
    apply_product<
        templates<animal_class_definitions>,
        mp_iota_c<NH>,
        types<ordinary_base, virtual_base>,
        types<no_work, some_work>
    >
> YOMM2_GENSYM;

template<typename...>
struct definition; // : not_defined {};

template<typename M, typename T>
struct definition<M, T> {
    static auto fn(T& a) {
        return T::work_type::fn(std::uintptr_t(&a), std::uintptr_t(&a));
    }
};

template<typename M, typename T, typename U>
struct definition<M, T, U> {
    static auto fn(T& a, U& b) {
        return T::work_type::fn(std::uintptr_t(&a), std::uintptr_t(&b));
    }
};

struct YOMM2_SYMBOL(kick);
template<typename M, typename P>
using kick = method<
    YOMM2_SYMBOL(kick),
    typename M::work_type::return_type(virtual_<M&>),
    P
>;

template<typename N, typename I, typename W>
struct definition1 {
    use_definitions<
        definition,
        product<
            apply_product<templates<kick>, types<Animal<N, I, W>>, policies>,
            types<Dog<N, I, W>, Cat<N, I, W>>>>
        YOMM2_GENSYM;
};

mp_apply<
    aggregate,
    apply_product<
        templates<definition1>,
        mp_iota_c<NH>,
        types<ordinary_base, virtual_base>,
        types<no_work, some_work>
    >
> YOMM2_GENSYM;

struct YOMM2_SYMBOL(meet);
template<typename M, typename P>
using meet = method<
    YOMM2_SYMBOL(meet),
    typename M::work_type::return_type(virtual_<M&>, virtual_<M&>),
    P
>;

template<typename N, typename I, typename W>
struct definition2 : use_definitions<
    definition,
    product<
        apply_product<
            templates<meet>,
            types<Animal<N, I, W>>,
            policies
        >,
        types<Dog<N, I, W>, Cat<N, I, W>>,
        types<Dog<N, I, W>, Cat<N, I, W>>
    >
> {};

mp_apply<
    aggregate,
    apply_product<
        templates<definition2>, mp_iota_c<NH>,
        types<ordinary_base, virtual_base>,
        types<no_work, some_work>
    >
> YOMM2_GENSYM;

struct abstract_sample {
    virtual void collect_vptrs(std::vector<uintptr_t>& vptrs) const = 0;
    virtual void* allocate() = 0;
    virtual void call(baseline, int_<1>) = 0;
    virtual void call(vfunc, int_<1>) = 0;
    virtual void call(vfunc, int_<2>) = 0;
    virtual void call(hash_factors_in_globals, int_<1>) = 0;
    virtual void call(hash_factors_in_globals, int_<2>) = 0;
    virtual void call(hash_factors_in_method, int_<1>) = 0;
    virtual void call(hash_factors_in_method, int_<2>) = 0;
};

struct distribution {
    const char* name;
    uintptr_t lo{std::numeric_limits<uintptr_t>::max()}, hi{0};
    double mean{0}, stddev{0};

    explicit distribution(
        const char* name, const std::vector<uintptr_t>& values)
        : name(name) {
        assert(values.size() >= 2);

        {
            auto iter = values.begin();
            auto value = *iter++;
            auto next = *iter++;
            double sum = 0;

            while (iter != values.end()) {
                assert(value <= next);
                auto diff = next - value;
                sum += diff;

                if (diff < lo) {
                    lo = diff;
                }

                if (diff > hi) {
                    hi = diff;
                }

                value = next;
                next = *iter++;
            }

            mean = sum / (values.size() - 1);
        }

        {
            auto iter = values.begin();
            auto value = *iter++;
            auto next = *iter++;
            double sum = 0;

            while (iter != values.end()) {
                auto diff = mean - (next - value);
                sum += diff * diff;

                value = next;
                next = *iter++;
            }

            stddev = std::sqrt(sum / (values.size() - 1));
        }
    }
};

template<typename N, typename Inheritance, typename Work>
struct sample : abstract_sample {
    using Base = Animal<N, Inheritance, Work>;
    std::default_random_engine rnd;
    std::uniform_int_distribution<std::size_t> dist{0, OBJECTS() - 1};
    std::vector<Base*> objects;

    static sample instance;

    void collect_vptrs(std::vector<uintptr_t>& vptrs) const override {
        Cat<N, Inheritance, Work> sylvester;
        vptrs.push_back(*reinterpret_cast<uintptr_t*>((&sylvester)));
        Dog<N, Inheritance, Work> hector;
        vptrs.push_back(*reinterpret_cast<uintptr_t*>((&hector)));
    }

    ~sample() {
        for (auto p : objects) {
            delete p;
        }
    }

    void* allocate() override {
        if (objects.size() == OBJECTS()) {
            return nullptr;
        }

        auto obj = (objects.size() & 1) ? (Base*)new Cat<N, Inheritance, Work>
                                        : new Dog<N, Inheritance, Work>;
        objects.push_back(obj);

        return obj;
    }

    Base& draw() {
        return *objects[dist(rnd)];
    }

    void call(baseline, int_<1>) override {
        benchmark::DoNotOptimize(call_baseline(draw(), draw()));
    }

    int call_baseline(Base& a, Base& b) {
        return 0;
    }

    void call(vfunc, int_<1>) override {
        call_virtual1(draw(), draw());
    }

    int call_virtual1(Base& a, Base& b) {
        a.kick();
        return 0;
    }

    void call(vfunc, int_<2>) override {
        call_virtual2(draw(), draw());
    }

    int call_virtual2(Base& a, Base& b) {
        a.meet(b);
        return 0;
    }

    void call(hash_factors_in_globals, int_<1>) override {
        call_hash_factors_in_globals_1(draw(), draw());
    }

    int call_hash_factors_in_globals_1(Base& a, Base& b) {
        kick<Animal<N, Inheritance, Work>, hash_factors_in_globals>::fn(a);
        return 0;
    }

    void call(hash_factors_in_globals, int_<2>) override {
        call_hash_factors_in_globals_2(draw(), draw());
    }

    int call_hash_factors_in_globals_2(Base& a, Base& b) {
        meet<Animal<N, Inheritance, Work>, hash_factors_in_globals>::fn(a, b);
        return 0;
    }

    void call(hash_factors_in_method, int_<1>) override {
        call_hash_factors_in_method_1(draw(), draw());
    }

    int call_hash_factors_in_method_1(Base& a, Base& b) {
        kick<Animal<N, Inheritance, Work>, hash_factors_in_method>::fn(a);
        return 0;
    }

    void call(hash_factors_in_method, int_<2>) override {
        call_hash_factors_in_method_2(draw(), draw());
    }

    int call_hash_factors_in_method_2(Base& a, Base& b) {
        meet<Animal<N, Inheritance, Work>, hash_factors_in_method>::fn(a, b);
        return 0;
    }
};

template<typename N, typename Inheritance, typename Work>
sample<N, Inheritance, Work> sample<N, Inheritance, Work>::instance;

template<typename Inheritance, typename Work>
std::vector<abstract_sample*> samples;

std::vector<abstract_sample*> all_samples;

template<typename...>
struct name;

template<>
struct name<baseline> {
    static constexpr const char* fn() {
        return "baseline";
    }
};

template<>
struct name<vfunc> {
    static constexpr const char* fn() {
        return "virtual";
    }
};

template<>
struct name<ordinary_base> {
    static constexpr const char* fn() {
        return "ordinary_base";
    }
};

template<>
struct name<virtual_base> {
    static constexpr const char* fn() {
        return "virtual_base";
    }
};

template<>
struct name<no_work> {
    static constexpr const char* fn() {
        return "no_work";
    }
};

template<>
struct name<some_work> {
    static constexpr const char* fn() {
        return "some_work";
    }
};

template<>
struct name<hash_factors_in_globals> {
    static constexpr const char* fn() {
        return "hash_factors_in_globals";
    }
};

template<>
struct name<hash_factors_in_method> {
    static constexpr const char* fn() {
        return "hash_factors_in_method";
    }
};

template<typename Dispatch, typename Arity, typename Inheritance, typename Work>
struct Benchmark {
    static std::string product_name;

    Benchmark() {
        benchmark::RegisterBenchmark(product_name.c_str(), run);
    }

    static void run(benchmark::State& state) {
        std::default_random_engine rnd;
        std::uniform_int_distribution<std::size_t> dist{0, NH - 1};
        for (auto _ : state) {
            samples<Inheritance, Work>[dist(rnd)] -> call(Dispatch(), Arity());
        }
    }
};

template<typename Dispatch, typename Arity, typename Inheritance, typename Work>
std::string Benchmark<Dispatch, Arity, Inheritance, Work>::product_name =
    std::string(name<Dispatch>::fn())
        + " arity_" + std::to_string(Arity::value)
        + " " + name<Inheritance>::fn()
        + " " + name<Work>::fn();

int main(int argc, char** argv) {
    yorel::yomm2::update_methods();

    std::ostringstream version;
#if defined(__clang__)
    auto compiler = "clang";
    version << __clang_major__ << "." << __clang_minor__ << "."
            << __clang_patchlevel__;
#else
#ifdef __GNUC__
    auto compiler = "gcc";
    version << __GNUC__ << "." << __GNUC_MINOR__;
#endif
#endif
#ifdef _MSC_VER
    auto compiler = "Visual Studio";
    version << _MSC_VER;
#endif
    benchmark::AddCustomContext("yomm2_compiler", compiler);
    benchmark::AddCustomContext("yomm2_compiler_version", version.str());
#ifdef NDEBUG
    benchmark::AddCustomContext("yomm2_build_type", "release");
#else
    benchmark::AddCustomContext("yomm2_build_type", "debug");
#endif
    benchmark::AddCustomContext("yomm2_hierarchies", std::to_string(NH));
    benchmark::AddCustomContext("yomm2_objects", std::to_string(OBJECTS()));

    std::vector<uintptr_t> vptrs;

    mp_for_each<mp_iota_c<NH>>([&vptrs](auto I_value) {
        using I = decltype(I_value);
        mp_for_each<inheritance_types>([&vptrs](auto INH_value) {
            using INH = decltype(INH_value);
            mp_for_each<work_types>([&vptrs](auto W_value) {
                using W = decltype(W_value);
                samples<INH, W>.push_back(&sample<I, INH, W>::instance);
                all_samples.push_back(&sample<I, INH, W>::instance);
            });
        });
    });

    std::vector<uintptr_t> obj_ptrs;

    {
        std::default_random_engine rnd;
        std::uniform_int_distribution<std::size_t> dist;

        auto incomplete_samples = all_samples;

        while (!incomplete_samples.empty()) {
            // pick one at random
            auto i = dist(rnd) % incomplete_samples.size();
            auto sample_set = incomplete_samples[i];
            // make it allocate one object
            if (auto obj = sample_set->allocate()) {
                obj_ptrs.push_back(reinterpret_cast<uintptr_t>(obj));
            } else {
                // if it is full, remove it
                incomplete_samples.erase(incomplete_samples.begin() + i);
            }
        }
    }

    // std::sort(vptrs.begin(), vptrs.end());
    // std::sort(obj_ptrs.begin(), obj_ptrs.end());

    // for (const auto& dist :
    //      {distribution("vptrs", vptrs),
    //       distribution("obj_ptrs", obj_ptrs)}) {
    //     benchmark::AddCustomContext(
    //         yomm2_ + dist.name + "_lo", std::to_string(dist.lo));
    //     benchmark::AddCustomContext(
    //         yomm2_ + dist.name + "_hi", std::to_string(dist.hi));
    //     benchmark::AddCustomContext(
    //         yomm2_ + dist.name + "_mean", std::to_string(dist.mean));
    //     benchmark::AddCustomContext(
    //         yomm2_ + dist.name + "_stddev", std::to_string(dist.stddev));
    // }

    Benchmark<baseline, int_<1>, ordinary_base, no_work> YOMM2_GENSYM;

    // clang-format off
    mp_apply<
        std::tuple,
        apply_product<
            templates<Benchmark>,
            types<
                vfunc,
                hash_factors_in_globals,
                hash_factors_in_method
            >,
            types<
                std::integral_constant<size_t, 1>,
                std::integral_constant<size_t, 2>
            >,
            types<ordinary_base, virtual_base>,
            types<no_work, some_work>
        >
    > YOMM2_GENSYM;

    benchmark::Initialize(&argc, argv);

    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}
