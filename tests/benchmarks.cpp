// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#if defined(__GNUC__) and !defined(__clang__)
// Don't benchmark g++ until this is fixed:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
int main() {
}
#else

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>

#if __has_include(<boost/unordered/unordered_flat_map.hpp>)
#include <boost/unordered/unordered_flat_map.hpp>
#define UNORDERED_FLAT_MAP_AVAILABLE                                           \
    __has_include(<boost/unordered/unordered_flat_map.hpp>)
#endif

#include <benchmark/benchmark.h>

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/compiler.hpp>
#include <yorel/yomm2/detail/compiler.hpp>
#include <yorel/yomm2/templates.hpp>

#include "benchmarks_parameters.hpp"

using namespace yorel::yomm2;
using namespace yorel::yomm2::policy;
using yorel::yomm2::detail::dump_type;
using namespace boost::mp11;

#if !defined(NDEBUG)
// enum { NH = 2 };
enum { NH = 1 };
#elif defined(YOMM2_BENCHMARK_HIERARCHIES)
enum { NH = YOMM2_BENCHMARK_HIERARCHIES };
#else
enum { NH = 100 };
#endif

#define mb() asm volatile("mfence" ::: "memory")
#define rmb() asm volatile("lfence" ::: "memory")
#define wmb() asm volatile("sfence" ::: "memory")

const std::string yomm2_ = "yomm2_";

auto OBJECTS() {
    static std::size_t n = 1000;

    if (auto objects_from_env = getenv("YOMM2_BENCHMARKS_OBJECTS")) {
        n = std::stoi(objects_from_env);
    }

    return n;
}

std::default_random_engine alloc_rnd;
std::uniform_int_distribution<std::size_t> alloc_dist{0, 500};

struct ordinary_inheritance {
    static std::string name() {
        return "ordinary_base";
    }
};

struct virtual_inheritance {
    static std::string name() {
        return "virtual_base";
    }
};

using inheritance_types = std::tuple<ordinary_inheritance, virtual_inheritance>;

template<typename>
struct orthogonal_base {
    virtual ~orthogonal_base() {
    }
};

template<typename>
struct direct_intrusive_base {
    using policy = default_policy;
    virtual ~direct_intrusive_base() {
    }
    auto yomm2_vptr() const {
        return vptr;
    };
    std::uintptr_t* vptr;
};

template<typename>
struct indirect_intrusive_base {
    using policy = default_policy;
    virtual ~indirect_intrusive_base() {
    }
    auto yomm2_vptr() const {
        return *vptr;
    };
    std::uintptr_t** vptr;
};

struct virtual_by_reference {
    template<class Population>
    static decltype(auto) draw(Population& pop) {
        return pop.draw();
    }
};

struct no_dispatch : virtual_by_reference {
    static std::string name() {
        return "baseline";
    };
};

struct virtual_dispatch : virtual_by_reference {
    static std::string name() {
        return "virtual_function";
    };
};

struct use_basic_policy : virtual_by_reference {
    struct policy
        : default_static::rebind<policy>::remove<yomm2::policy::trace_output>,
          yomm2::policy::basic_trace_output<policy> {};
    template<typename Inheritance>
    using base_type = orthogonal_base<Inheritance>;
    static std::string name() {
        return "basic_policy";
    };
};

struct std_map_policy : virtual_by_reference {
    struct policy : default_static::rebind<policy>::
                        remove<yomm2::policy::type_hash>::replace<
                            yomm2::policy::external_vptr,
                            yomm2::policy::vptr_map<policy>> {};
    template<typename Inheritance>
    using base_type = orthogonal_base<Inheritance>;
    static std::string name() {
        return "std_map_policy";
    };
};

#if UNORDERED_FLAT_MAP_AVAILABLE
struct flat_map_policy : virtual_by_reference {
    struct policy : default_static::rebind<policy>::
                        remove<yomm2::policy::type_hash>::replace<
                            yomm2::policy::external_vptr,
                            yomm2::policy::vptr_map<
                                policy,
                                boost::unordered_flat_map<
                                    type_id, const std::uintptr_t*>>> {};
    template<typename Inheritance>
    using base_type = orthogonal_base<Inheritance>;
    static std::string name() {
        return "flat_map_policy";
    };
};
#endif

struct direct_intrusive_dispatch : virtual_by_reference {
    struct policy
        : default_static::rebind<policy>::remove<yomm2::policy::external_vptr> {
        template<class Class>
        static auto dynamic_vptr(const Class& arg) {
            return arg.yomm2_vptr();
        }
    };
    template<typename Inheritance>
    using base_type = direct_intrusive_base<Inheritance>;
    static std::string name() {
        return "direct_intrusive";
    };
};

struct indirect_intrusive_dispatch : virtual_by_reference {
    struct policy : default_static::rebind<policy> {};
    template<typename Inheritance>
    using base_type = indirect_intrusive_base<Inheritance>;
    static std::string name() {
        return "indirect_intrusive";
    };
};

struct direct_virtual_ptr_dispatch {
    template<class Population>
    static auto draw(Population& pop) {
        return pop.vptr_draw();
    }
    struct policy : default_static::rebind<policy> {};
    template<typename Inheritance>
    using base_type = orthogonal_base<Inheritance>;
    static std::string name() {
        return "direct_virtual_ptr";
    };
};

struct indirect_virtual_ptr_dispatch {
    template<class Population>
    static auto draw(Population& pop) {
        return pop.ivptr_draw();
    }
    struct policy : default_static::rebind<policy>,
                    yomm2::policy::basic_indirect_vptr<policy> {};
    template<typename Inheritance>
    using base_type = orthogonal_base<Inheritance>;
    static std::string name() {
        return "indirect_virtual_ptr";
    };
};

using method_dispatch_types = std::tuple<
    use_basic_policy, std_map_policy,
#if UNORDERED_FLAT_MAP_AVAILABLE
    flat_map_policy,
#endif
    direct_virtual_ptr_dispatch, indirect_virtual_ptr_dispatch,
    direct_intrusive_dispatch, indirect_intrusive_dispatch>;

using dispatch_types = mp_append<
    std::tuple<no_dispatch, virtual_dispatch, direct_virtual_ptr_dispatch>,
    method_dispatch_types, method_dispatch_types>;

using arity_1 = std::integral_constant<std::size_t, 1>;
using arity_2 = std::integral_constant<std::size_t, 2>;
using arity_types = std::tuple<arity_1, arity_2>;

struct abstract_population;
std::vector<abstract_population*> populations;

struct abstract_population {
    virtual void* allocate() = 0;

    using function_type = std::function<void()>;

    function_type dispatchers[mp_size<dispatch_types>::value]
                             [mp_size<arity_types>::value]
                             [mp_size<inheritance_types>::value];

    template<typename Dispatch, typename Arity, typename Inheritance>
    function_type& dispatcher() {
        return dispatchers[mp_find<dispatch_types, Dispatch>::value]
                          [Arity::value - 1]
                          [mp_find<inheritance_types, Inheritance>::value];
    }
};

template<int I>
using int_ = std::integral_constant<int, I>;

template<typename N>
struct population : abstract_population {
    template<typename Base, int intermediate>
    struct vfunc_intermediate;

    template<int>
    struct intermediate;

    struct vfunc_base {
        virtual void fn() = 0;
        virtual void fn(vfunc_base& other) = 0;
        virtual void fn_dd(intermediate<0>&) = 0;
        virtual void fn_dd(intermediate<1>&) = 0;
    };

    struct vfunc_vbase {
        virtual void v_fn() = 0;
        virtual void v_fn(vfunc_vbase& other) = 0;
        virtual void v_fn_dd(intermediate<0>&) = 0;
        virtual void v_fn_dd(intermediate<1>&) = 0;
    };

    struct base : vfunc_base,
                  orthogonal_base<ordinary_inheritance>,
                  direct_intrusive_base<ordinary_inheritance>,
                  indirect_intrusive_base<ordinary_inheritance>,
                  virtual vfunc_vbase,
                  virtual orthogonal_base<virtual_inheritance>,
                  virtual direct_intrusive_base<virtual_inheritance>,
                  virtual indirect_intrusive_base<virtual_inheritance> {};

    template<int>
    struct intermediate : base {
        void fn() override {
        }
        void fn(vfunc_base& other) override {
            other.fn_dd(*this);
        }
        void fn_dd(intermediate<0>&) override {
        }
        void fn_dd(intermediate<1>&) override {
        }

        void v_fn() override {
        }
        void v_fn(vfunc_vbase& other) override {
            other.v_fn_dd(*this);
        }
        void v_fn_dd(intermediate<0>&) override {
        }
        void v_fn_dd(intermediate<1>&) override {
        }
    };

    using non_leaf_classes = mp_list<
        vfunc_base, orthogonal_base<ordinary_inheritance>,
        direct_intrusive_base<ordinary_inheritance>,
        indirect_intrusive_base<ordinary_inheritance>, vfunc_vbase,
        orthogonal_base<virtual_inheritance>,
        direct_intrusive_base<virtual_inheritance>,
        indirect_intrusive_base<virtual_inheritance>, base, intermediate<0>,
        intermediate<1>>;

    template<typename>
    struct leaf0 : intermediate<0> {
        leaf0() {
            this->direct_intrusive_base<ordinary_inheritance>::vptr =
                direct_intrusive_dispatch::policy::static_vptr<leaf0>;
            this->direct_intrusive_base<virtual_inheritance>::vptr =
                direct_intrusive_dispatch::policy::static_vptr<leaf0>;
            this->indirect_intrusive_base<ordinary_inheritance>::vptr =
                &indirect_intrusive_dispatch::policy::static_vptr<leaf0>;
            this->indirect_intrusive_base<virtual_inheritance>::vptr =
                &indirect_intrusive_dispatch::policy::static_vptr<leaf0>;
        }
    };

    template<typename>
    struct leaf1 : intermediate<1> {
        leaf1() {
            this->direct_intrusive_base<ordinary_inheritance>::vptr =
                direct_intrusive_dispatch::policy::static_vptr<leaf1>;
            this->direct_intrusive_base<virtual_inheritance>::vptr =
                direct_intrusive_dispatch::policy::static_vptr<leaf1>;
            this->indirect_intrusive_base<ordinary_inheritance>::vptr =
                &indirect_intrusive_dispatch::policy::static_vptr<leaf1>;
            this->indirect_intrusive_base<virtual_inheritance>::vptr =
                &indirect_intrusive_dispatch::policy::static_vptr<leaf1>;
        }
    };

    static constexpr std::size_t num_leaf_classes = 10;

    using leaf_classes = mp_rename<
        mp_append<
            mp_transform<leaf0, mp_iota_c<num_leaf_classes / 2>>,
            mp_transform<leaf1, mp_iota_c<(num_leaf_classes + 1) / 2>>>,
        mp_list>;

    using classes = mp_append<non_leaf_classes, leaf_classes>;

    template<typename Base, typename Policy>
    using method_1 = method<population, void(virtual_<Base&>), Policy>;

    template<typename Base, typename Policy>
    using method_2 =
        method<population, void(virtual_<Base&>, virtual_<Base&>), Policy>;

    template<typename Dispatch, typename Inheritance>
    struct ref_methods {
        using Policy = typename Dispatch::policy;
        using Base = typename Dispatch::template base_type<Inheritance>;
        using varg_type = Base&;

        use_classes<classes, Policy> YOMM2_GENSYM;

        using method1 = method<population, void(virtual_<Base&>), Policy>;
        using method2 =
            method<population, void(virtual_<Base&>, virtual_<Base&>), Policy>;

        template<typename T>
        static void fn1(T&) {
        }
        typename method1::template add_function<fn1<intermediate<0>>>
            YOMM2_GENSYM;
        typename method1::template add_function<fn1<intermediate<1>>>
            YOMM2_GENSYM;

        template<typename T, typename U>
        static void fn2(T&, U&) {
        }
        typename method2::template add_function<
            fn2<intermediate<0>, intermediate<0>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<0>, intermediate<1>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<1>, intermediate<0>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<1>, intermediate<1>>>
            YOMM2_GENSYM;
    };

    template<typename Dispatch, typename Inheritance>
    struct vptr_methods {
        using Policy = typename Dispatch::policy;
        using Base = orthogonal_base<Inheritance>;
        template<class Class>
        using vptr = virtual_ptr<Class, Policy>;
        using varg_type = vptr<Base>;

        use_classes<classes, Policy> YOMM2_GENSYM;

        using method1 = method<population, void(vptr<Base>), Policy>;
        using method2 =
            method<population, void(vptr<Base>, vptr<Base>), Policy>;

        template<typename T>
        static void fn1(vptr<T>) {
        }
        typename method1::template add_function<fn1<intermediate<0>>>
            YOMM2_GENSYM;
        typename method1::template add_function<fn1<intermediate<1>>>
            YOMM2_GENSYM;

        template<typename T, typename U>
        static void fn2(vptr<T>, vptr<U>) {
        }
        typename method2::template add_function<
            fn2<intermediate<0>, intermediate<0>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<0>, intermediate<1>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<1>, intermediate<0>>>
            YOMM2_GENSYM;
        typename method2::template add_function<
            fn2<intermediate<1>, intermediate<1>>>
            YOMM2_GENSYM;
    };

    template<typename Dispatch, typename Inheritance>
    struct methods : std::conditional_t<
                         std::is_base_of_v<virtual_by_reference, Dispatch>,
                         ref_methods<Dispatch, Inheritance>,
                         vptr_methods<Dispatch, Inheritance>> {};

    mp_product<methods, method_dispatch_types, inheritance_types> YOMM2_GENSYM;

    template<typename Dispatch, typename Arity, typename Inheritance>
    struct dispatch {
        using Arg = typename methods<Dispatch, Inheritance>::varg_type;

        static void fn(Arg a, Arg b) {
            if constexpr (Arity::value == 1) {
                methods<Dispatch, Inheritance>::method1::fn(a);
            } else {
                static_assert(Arity::value == 2);
                methods<Dispatch, Inheritance>::method2::fn(a, b);
            }
        }
    };

    template<>
    struct dispatch<virtual_dispatch, arity_1, ordinary_inheritance> {
        static void fn(base& a, base& b) {
            a.fn();
        }
    };

    template<>
    struct dispatch<virtual_dispatch, arity_1, virtual_inheritance> {
        static void fn(base& a, base& b) {
            a.v_fn();
        }
    };

    template<>
    struct dispatch<virtual_dispatch, arity_2, ordinary_inheritance> {
        static void fn(base& a, base& b) {
            a.fn(b);
        }
    };

    template<>
    struct dispatch<virtual_dispatch, arity_2, virtual_inheritance> {
        static void fn(base& a, base& b) {
            a.v_fn(b);
        }
    };

    template<typename Arity, typename Inheritance>
    struct dispatch<no_dispatch, Arity, Inheritance> {
        static void fn(base& a, base& b) {
        }
    };

    mp_product<dispatch, dispatch_types, arity_types, inheritance_types>
        YOMM2_GENSYM;

    std::default_random_engine rnd;
    std::uniform_int_distribution<std::size_t> dist{0, OBJECTS() - 1};
    std::vector<base*> objects;
    std::vector<virtual_ptr<base, direct_virtual_ptr_dispatch::policy>> vptrs;
    std::vector<virtual_ptr<base, indirect_virtual_ptr_dispatch::policy>>
        ivptrs;

    static population instance;

    template<typename T>
    static base* make() {
        return new T;
    }

    std::vector<base* (*)()> factories;

    void populate() {
        mp_for_each<leaf_classes>(
            [this](auto value) { factories.push_back(make<decltype(value)>); });

        mp_for_each<dispatch_types>([this](auto D_value) {
            using Dispatch = decltype(D_value);
            mp_for_each<arity_types>([this](auto A_value) {
                using Arity = decltype(A_value);
                mp_for_each<inheritance_types>([this](auto I_value) {
                    using Inheritance = decltype(I_value);
                    this->dispatcher<Dispatch, Arity, Inheritance>() =
                        [this]() {
                            dispatch<Dispatch, Arity, Inheritance>::fn(
                                Dispatch::template draw<population>(*this),
                                Dispatch::template draw<population>(*this));
                        };
                });
            });
        });
    }

    ~population() {
        for (auto p : objects) {
            delete p;
        }
    }

    void* allocate() override {
        if (objects.size() == OBJECTS()) {
            return nullptr;
        }

        auto obj = factories[objects.size() % factories.size()]();
        objects.push_back(obj);
        vptrs.emplace_back(*obj);
        ivptrs.emplace_back(*obj);

        return obj;
    }

    base& draw() {
        return *objects[dist(rnd)];
    }

    auto vptr_draw() {
        return vptrs[dist(rnd)];
    }

    auto ivptr_draw() {
        return ivptrs[dist(rnd)];
    }
};

template<typename N>
population<N> population<N>::instance;

template<typename Dispatch, typename Arity, typename Inheritance>
struct Benchmark {
    std::string name;

    Benchmark() {
        name = Dispatch::name() + "-arity_" + std::to_string(Arity::value) +
            "-" + Inheritance::name();
        benchmark::RegisterBenchmark(name.c_str(), run);
    }

    explicit Benchmark(std::string name) : name(name) {
        benchmark::RegisterBenchmark(name.c_str(), run);
    }

    static void run(benchmark::State& state) {
        std::default_random_engine rnd;
        std::uniform_int_distribution<std::size_t> dist{0, NH - 1};
        for (auto _ : state) {
            populations[dist(rnd)]
                ->dispatcher<Dispatch, Arity, Inheritance>()();
        }
    }
};

int main(int argc, char** argv) {
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

    Benchmark<no_dispatch, arity_1, ordinary_inheritance> YOMM2_GENSYM(
        "baseline");

    mp_apply<
        std::tuple,
        apply_product<
            templates<Benchmark>, mp_remove<dispatch_types, no_dispatch>,
            arity_types, inheritance_types>>
        YOMM2_GENSYM;

    mp_for_each<method_dispatch_types>(
        [](auto value) { update<typename decltype(value)::policy>(); });

    mp_for_each<mp_iota_c<NH>>([](auto I_value) {
        using I = decltype(I_value);
        population<I>::instance.populate();
        populations.push_back(&population<I>::instance);
    });

    std::vector<uintptr_t> obj_ptrs;

    {
        std::default_random_engine rnd;
        std::uniform_int_distribution<std::size_t> dist;

        auto incomplete_populations = populations;

        while (!incomplete_populations.empty()) {
            // pick one at random
            auto i = dist(rnd) % incomplete_populations.size();
            auto population_set = incomplete_populations[i];
            // make it allocate one object
            if (auto obj = population_set->allocate()) {
                obj_ptrs.push_back(reinterpret_cast<uintptr_t>(obj));
            } else {
                // if it is full, remove it
                incomplete_populations.erase(
                    incomplete_populations.begin() + i);
            }
        }
    }

    auto pop = populations[0];
    pop->dispatcher<
        direct_virtual_ptr_dispatch, arity_1, ordinary_inheritance>()();

#if !defined(NDEBUG)
    mp_for_each<dispatch_types>([](auto D_value) {
        using Dispatch = decltype(D_value);
        if constexpr (true || !std::is_same_v<Dispatch, no_dispatch>) {
            mp_for_each<arity_types>([](auto A_value) {
                using Arity = decltype(A_value);
                mp_for_each<inheritance_types>([](auto I_value) {
                    using Inheritance = decltype(I_value);
                    std::cout << Dispatch::name() << ", " << Arity::value
                              << ", " << Inheritance::name() << "\n";
                    auto pf = populations[0]
                                  ->dispatcher<Dispatch, Arity, Inheritance>();
                    pf();
                });
            });
        }
    });
#endif

    benchmark::Initialize(&argc, argv);

    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}

// void call_hash_factors_in_globals_1(population<std::integral_constant<std::size_t, 0>, ordinary_base, orthogonal_dispatch<use_basic_policy>, no_work>::Animal& a) {
//     population<std::integral_constant<std::size_t, 0>, ordinary_base, orthogonal_dispatch<use_basic_policy>, no_work>::dispatcher::kick(a);
// }

// void call_direct_intrusive_1(population<std::integral_constant<std::size_t, 0>, ordinary_base, direct_intrusive_dispatch, no_work>::Animal& a) {
//     population<std::integral_constant<std::size_t, 0>, ordinary_base, direct_intrusive_dispatch, no_work>::dispatcher::kick(a);
// 	// movq	  8(%rdi),                       %rax
// 	// movslq method.fn.slots_strides(%rip), %rcx
// 	// jmpq	*(%rax,%rcx,8)
// }

using _0 = std::integral_constant<std::size_t, 0>;
using leaf = population<_0>::leaf0<_0>;

void call_project_1(leaf& obj) {
    //pop.dispatcher<std_map_policy, arity_1, ordinary_inheritance>()();
    population<_0>::ref_methods<
        use_basic_policy, ordinary_inheritance>::method1::fn(obj);
    // mov	rax, qword ptr [rdi + 8]
    // add	rdi, 8
    // mov	rdx, qword ptr [rip + fast_perfect_hash<policy>::hash_mult]
    // imul	rdx, qword ptr [rax - 8]
    // movzx	ecx, byte ptr [rip + fast_perfect_hash<policy>::hash_shift]
    // shr	rdx, cl
    // mov	rax, qword ptr [rip + vptr_vector<policy>::vptrs]
    // mov	rax, qword ptr [rax + 8*rdx]
    // mov	rcx, qword ptr [rip + method<policy, population<integral_constant<unsigned long, 0ul> >, void (virtual_<orthogonal_base<ordinary_inheritance>&>)>::fn+80]
    // jmp	qword ptr [rax + 8*rcx]         # TAILCALL
}

void call_unordered_map_1(leaf& obj) {
    //pop.dispatcher<std_map_policy, arity_1, ordinary_inheritance>()();
    population<_0>::ref_methods<
        std_map_policy, ordinary_inheritance>::method1::fn(obj);

    // 	mov	rax, qword ptr [rdi + 8]
    // 	add	rdi, 8
    // 	mov	rcx, qword ptr [rax - 8]
    // 	cmp	qword ptr [rip + vptr_map<policy>::vptrs+24], 0
    // 	je	.LBB9_1
    // 	mov	rsi, qword ptr [rip + vptr_map<policy>::vptrs+8]
    // 	mov	rax, rcx
    // 	or	rax, rsi
    // 	shr	rax, 32
    // 	je	.LBB9_4
    // 	mov	rax, rcx
    // 	xor	edx, edx
    // 	div	rsi
    // 	jmp	.LBB9_6
    // .LBB9_1:                                # %.preheader.i.i.i.i.i.preheader
    // 	lea	rax, [rip + vptr_map<policy>::vptrs+16]
    // .LBB9_2:                                # %.preheader.i.i.i.i.i
    //                                         # =>This Inner Loop Header: Depth=1
    // 	mov	rax, qword ptr [rax]
    // 	cmp	qword ptr [rax + 8], rcx
    // 	jne	.LBB9_2
    // 	jmp	.LBB9_8
    // .LBB9_4:
    // 	mov	eax, ecx
    // 	xor	edx, edx
    // 	div	esi
    // .LBB9_6:
    // 	mov	rax, qword ptr [rip + vptr_map<policy>::vptrs]
    // 	mov	rax, qword ptr [rax + 8*rdx]
    // 	.p2align	4, 0x90
    // .LBB9_7:                                # %.lr.ph.i.i.i.i.i.i.i.i.i
    //                                         # =>This Inner Loop Header: Depth=1
    // 	mov	rax, qword ptr [rax]
    // 	cmp	qword ptr [rax + 8], rcx
    // 	jne	.LBB9_7
    // .LBB9_8:                                # %method<policy, population<integral_constant<unsigned long, 0ul> >, void (virtual_<orthogonal_base<ordinary_inheritance>&>)>::operator()(orthogonal_base<ordinary_inheritance>&) const [clone .exit]
    // 	mov	rax, qword ptr [rax + 16]
    // 	mov	rcx, qword ptr [rip + method<policy, population<integral_constant<unsigned long, 0ul> >, void (virtual_<orthogonal_base<ordinary_inheritance>&>)>::fn+80]
    // 	jmp	qword ptr [rax + 8*rcx]         # TAILCALL
}

#endif // exclude gcc
