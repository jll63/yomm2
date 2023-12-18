// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <algorithm> // for max, transform, copy
#include <cassert>   // for assert
#include <chrono>    // for operator-, duration
#include <cstdint>   // for uintptr_t
#include <cstdio>
#include <cstdlib> // for abort, getenv
#include <deque>
#include <iomanip>       // for operator<<, setw
#include <iterator>      // for back_insert_iterator
#include <list>          // for list, _List_iterator
#include <map>           // for map
#include <memory>        // for allocator_traits<...
#include <random>        // for default_random_en...
#include <stdexcept>     // for runtime_error
#include <string>        // for char_traits, allo...
#include <unordered_map> // for _Node_iterator
#include <unordered_set> // for unordered_set<>::...
#include <utility>       // for pair
#include <vector>        // for vector, vector<>:...

#include <boost/dynamic_bitset.hpp>

#include <yorel/yomm2/core.hpp>

#pragma push_macro("max")
#undef max

namespace yorel {
namespace yomm2 {
namespace detail {

struct rt_method;

struct rt_arg {
    rt_method* method;
    size_t param;
};

struct rt_class {
    bool is_abstract{false};
    std::vector<type_id> ti_ptrs;
    std::vector<rt_class*> transitive_bases;
    std::vector<rt_class*> direct_bases;
    std::vector<rt_class*> direct_derived;
    std::unordered_set<rt_class*> compatible_classes;
    std::vector<rt_arg> used_by_vp;
    int next_slot{0};
    int first_used_slot{-1};
    int layer{0};
    size_t mark{0};   // temporary mark to detect cycles
    size_t weight{0}; // number of proper direct or indirect bases
    std::vector<size_t> mtbl;
    detail::word** method_table;
};

struct rt_spec {
    const definition_info* info;
    std::vector<rt_class*> vp;
};

using bitvec = boost::dynamic_bitset<>;

struct group {
    std::vector<rt_class*> classes;
    bool has_concrete_classes{false};
};

using group_map = std::map<bitvec, group>;

struct dispatch_stats_t {
    size_t cells{0};
    size_t concrete_cells{0};
    size_t not_implemented{0};
    size_t concrete_not_implemented{0};
    size_t ambiguous{0};
    size_t concrete_ambiguous{0};

    void accumulate(const dispatch_stats_t& other);
};

struct rt_method {
    method_info* info;
    std::vector<rt_class*> vp;
    std::vector<rt_spec> specs;
    std::vector<size_t> slots;
    std::vector<size_t> strides;
    std::vector<void*> dispatch_table;
    const detail::word* gv_dispatch_table{nullptr};
    auto arity() const {
        return vp.size();
    }
    dispatch_stats_t stats;
};

struct metrics_t : dispatch_stats_t {
    size_t method_table_size, dispatch_table_size;
    size_t hash_search_attempts;
    std::chrono::duration<double> hash_search_time;
};

inline std::ostream* log_on(std::ostream* os) {
    auto prev = logs;
    logs = os;
    return prev;
}

inline std::ostream* log_off() {
    auto prev = logs;
    logs = nullptr;
    return prev;
}

struct type_name {
    type_name(type_id type) : type(type) {
    }
    type_id type;
};

template<class Policy>
struct runtime {
    using policy_type = Policy;
    using type_index_type = decltype(Policy::type_index(0));
    static constexpr bool trace_enabled =
        policy::has_facet<Policy, policy::output>;

    std::unordered_map<type_index_type, rt_class*> class_map;
    std::deque<rt_class> classes;
    std::vector<rt_method> methods;
    size_t class_visit{0};
    metrics_t metrics;

    runtime();

    void update();

    void resolve_static_type_ids();
    void augment_classes();
    void calculate_compatible_classes(rt_class& cls);
    void augment_methods();
    std::vector<rt_class*> layer_classes();
    void allocate_slots();
    void allocate_slot_down(rt_class* cls, size_t slot);
    void allocate_slot_up(rt_class* cls, size_t slot);
    void build_dispatch_tables();
    void build_dispatch_table(
        rt_method& m, size_t dim, std::vector<group_map>::const_iterator group,
        const bitvec& candidates, bool concrete);
    void install_gv(size_t type_ids);
    void optimize();
    size_t
    find_hash_function(const std::deque<rt_class>& classes, metrics_t& metrics);
    void print(const dispatch_stats_t& stats);
    static std::vector<const rt_spec*>
    best(std::vector<const rt_spec*>& candidates);
    static bool is_more_specific(const rt_spec* a, const rt_spec* b);
    static bool is_base(const rt_spec* a, const rt_spec* b);

    static type_id static_type(type_id type) {
        if constexpr (std::is_base_of_v<
                          policy::deferred_static_rtti, policy::rtti>) {
            return reinterpret_cast<type_id (*)()>(type)();
        } else {
            return type;
        }
    }

    struct rflush {
        size_t width;
        size_t value;
        explicit rflush(size_t width, size_t value)
            : width(width), value(value) {
        }
    };

    struct trace_type {
        bool on = false;
        size_t indentation_level{0};

        trace_type& operator++() {
            if constexpr (trace_enabled) {
                if (on) {
                    for (int i = 0; i < indentation_level; ++i) {
                        Policy::stream << "  ";
                    }
                }
            }

            return *this;
        }

        trace_type& operator<<(const rflush& rf) {
            if constexpr (trace_enabled) {
                if (on) {
                    auto pad = rf.width;
                    auto remain = rf.value;

                    do {
                        remain /= 10;
                        --pad;

                        if (pad < 0) {
                            return *this;
                        }
                    } while (remain);

                    while (pad--) {
                        *this << " ";
                    }

                    *this << rf.value;
                }
            }

            return *this;
        }

        trace_type& operator<<(const boost::dynamic_bitset<>& bits) {
            if constexpr (trace_enabled) {
                if (on) {
                    auto i = bits.size();
                    while (i != 0) {
                        --i;
                        Policy::stream << bits[i];
                    }
                }
            }
            return *this;
        }

        template<typename T>
        trace_type& operator<<(const T& value) {
            if constexpr (trace_enabled) {
                if (on) {
                    Policy::stream << value;
                }
            }
            return *this;
        }

        trace_type& operator<<(type_range<type_id*> tips) {
            if constexpr (trace_enabled) {
                if (on) {
                    *this << "(";
                    const char* sep = "";
                    for (auto t : tips) {
                        *this << sep << type_name(t);
                        sep = ", ";
                    }

                    *this << ")";
                }
            }

            return *this;
        }

        template<template<typename...> typename Container, typename... T>
        trace_type& operator<<(Container<rt_class*, T...>& classes) {
            if constexpr (trace_enabled) {
                if (on) {
                    *this << "(";
                    const char* sep = "";
                    for (auto cls : classes) {
                        *this << sep << *cls;
                        sep = ", ";
                    }

                    *this << ")";
                }
            }

            return *this;
        }

        trace_type& operator<<(type_name manip) {
            if constexpr (trace_enabled) {
                if (on) {
                    *this << manip.type;
                }
            }

            return *this;
        }

        trace_type& operator<<(const rt_class& cls) {
            if constexpr (trace_enabled) {
                if (on) {
                    *this << type_name(cls.ti_ptrs[0]);
                }
            }

            return *this;
        }
    };

    trace_type trace;

    struct indent {
        trace_type& trace;
        int by;

        explicit indent(trace_type& trace, int by = 2) : trace(trace), by(by) {
            trace.indentation_level += by;
        }

        ~indent() {
            trace.indentation_level -= by;
        }
    };
};

template<class Policy>
void runtime<Policy>::update() {
    using namespace policy;

    if constexpr (has_facet<Policy, error_handler>) {
        if (!Policy::error) {
            Policy::error = Policy::default_error_handler;
        }
    }

    resolve_static_type_ids();
    augment_classes();
    augment_methods();
    allocate_slots();
    build_dispatch_tables();
    auto type_ids = find_hash_function(classes, metrics);
    install_gv(type_ids);
    optimize();

    print(metrics);

    ++trace << "Finished\n";
}

template<class Policy>
runtime<Policy>::runtime() {
    if constexpr (trace_enabled) {
        trace.on = Policy::stream.is_on();
    }
}

template<class Policy>
void runtime<Policy>::resolve_static_type_ids() {
    auto resolve = [](type_id* p) {
        auto pf = reinterpret_cast<type_id (*)()>(*p);
        *p = pf();
    };

    if constexpr (std::is_base_of_v<policy::deferred_static_rtti, Policy>) {
        if (!Policy::catalog.classes.empty())
            for (auto& ci : Policy::catalog.classes) {
                resolve(&ci.ti);

                if (*ci.last_base == 0) {
                    for (auto& ti :
                         detail::type_range{ci.first_base, ci.last_base}) {
                        resolve(&ti);
                    }

                    *ci.last_base = 1;
                }
            }

        if (!Policy::catalog.methods.empty())
            for (auto& method : Policy::catalog.methods) {
                for (auto& ti :
                     detail::type_range{method.vp_begin, method.vp_end}) {
                    if (*method.vp_end == 0) {
                        resolve(&ti);
                        *method.vp_end = 1;
                    }

                    if (!method.specs.empty())
                        for (auto& definition : method.specs) {
                            if (*definition.vp_end == 0) {
                                for (auto& ti : detail::type_range{
                                         definition.vp_begin,
                                         definition.vp_end}) {
                                    resolve(&ti);
                                }

                                *definition.vp_end = 1;
                            }
                        }
                }
            }
    }
}

template<class Policy>
void runtime<Policy>::augment_classes() {
    // scope
    {
        ++trace << "Static class info:\n";

        // The standard does not guarantee that there is exactly one
        // type_info object per class. However, it guarantees that the
        // type_index for a class has a unique value.
        for (auto& cr : Policy::catalog.classes) {
            if constexpr (trace_enabled) {
                {
                    indent YOMM2_GENSYM(trace);
                    ++trace << type_name(cr.ti) << ": "
                            << type_range{cr.first_base, cr.last_base};

                    ++trace << "\n";
                }
            }

            auto& rtc = class_map[Policy::type_index(cr.ti)];

            if (rtc == nullptr) {
                rtc = &classes.emplace_back();
                rtc->is_abstract = cr.is_abstract;
                rtc->method_table = cr.method_table;
            }

            // In the unlikely case that a class does have more than one
            // associated  ti*, collect them in a vector. We don't use an
            // unordered_set because, again, this situation is highly
            // unlikely, and, were it to occur, the number of distinct ti*s
            // would probably be small.
            if (std::find(rtc->ti_ptrs.begin(), rtc->ti_ptrs.end(), cr.ti) ==
                rtc->ti_ptrs.end()) {
                rtc->ti_ptrs.push_back(cr.ti);
            }
        }
    }

    // All known classes now have exactly one associated rt_class* in the
    // map. Collect the bases.

    for (auto& cr : Policy::catalog.classes) {
        auto& rtc = class_map[Policy::type_index(cr.ti)];

        for (auto base_iter = cr.first_base; base_iter != cr.last_base;
             ++base_iter) {
            auto rtb = class_map[Policy::type_index(*base_iter)];

            if (!rtb) {
                unknown_class_error error;
                error.ti = *base_iter;
                Policy::error(error_type(error));
                abort();
            }

            if (rtc != rtb) {
                // At compile time we collected the class as its own
                // improper base, as per std::is_base_of. Eliminate that.
                rtc->transitive_bases.push_back(rtb);
            }
        }
    }

    // At this point bases may contain duplicates, and also indirect
    // bases. Clean that up.

    size_t mark = ++class_visit;

    for (auto& rtc : classes) {
        decltype(rtc.transitive_bases) bases;
        mark = ++class_visit;

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark != mark) {
                bases.push_back(rtb);
                rtb->mark = mark;
            }
        }

        // Record the "weight" of the class, i.e. the total number of direct
        // and indirect proper bases it has.
        rtc.weight = bases.size();
        rtc.transitive_bases.swap(bases);
    }

    for (auto& rtc : classes) {
        // Sort base classes by weight. This ensures that a base class is
        // never preceded by one if its own bases classes.
        std::sort(
            rtc.transitive_bases.begin(), rtc.transitive_bases.end(),
            [](auto a, auto b) { return a->weight > b->weight; });
        mark = ++class_visit;

        // Collect the direct base classes. The first base is certainly a
        // direct one. Remove *its* bases from the candidates, by marking
        // them. Continue with the next base that is not marked. It is the
        // next direct base. And so on...

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark == mark) {
                continue;
            }

            rtc.direct_bases.push_back(rtb);

            for (auto rtbb : rtb->transitive_bases) {
                rtbb->mark = mark;
            }
        }
    }

    for (auto& rtc : classes) {
        for (auto rtb : rtc.direct_bases) {
            rtb->direct_derived.push_back(&rtc);
        }
    }

    for (auto& rtc : classes) {
        calculate_compatible_classes(rtc);
    }

    if constexpr (trace_enabled) {
        ++trace << "Inheritance lattice:\n";
        for (auto& rtc : classes) {
            indent YOMM2_GENSYM(trace);
            ++trace << rtc << "\n";

            {
                indent YOMM2_GENSYM(trace);
                ++trace << "bases:      " << rtc.direct_bases << "\n";
                ++trace << "derived:    " << rtc.direct_derived << "\n";
                ++trace << "compatible: " << rtc.compatible_classes << "\n";
            }
        }
    }
}

template<class Policy>
void runtime<Policy>::calculate_compatible_classes(rt_class& cls) {
    if (!cls.compatible_classes.empty()) {
        return;
    }

    cls.compatible_classes.insert(&cls);

    for (auto derived : cls.direct_derived) {
        if (derived->compatible_classes.empty()) {
            calculate_compatible_classes(*derived);
        }

        std::copy(
            derived->compatible_classes.begin(),
            derived->compatible_classes.end(),
            std::inserter(
                cls.compatible_classes, cls.compatible_classes.end()));
    }
}

template<class Policy>
void runtime<Policy>::augment_methods() {
    methods.resize(Policy::catalog.methods.size());

    ++trace << "Methods:\n";
    indent YOMM2_GENSYM(trace);

    auto meth_iter = methods.rbegin();
    // reverse the registration order reversed by 'chain'.

    for (auto& meth_info : Policy::catalog.methods) {
        if constexpr (trace_enabled) {
            ++trace << meth_info.name << " "
                    << type_range{meth_info.vp_begin, meth_info.vp_end} << "\n";
        }

        indent YOMM2_GENSYM(trace);

        meth_iter->info = &meth_info;
        meth_iter->vp.reserve(meth_info.arity());
        size_t param_index = 0;

        for (auto ti : type_range{meth_info.vp_begin, meth_info.vp_end}) {
            auto rt_class = class_map[Policy::type_index(ti)];
            if (!rt_class) {
                ++trace << "unkown class " << ti << "(" << type_name(ti)
                        << ") for parameter #" << (param_index + 1) << "\n";
                unknown_class_error error;
                error.ti = ti;
                Policy::error(error_type(error));
                abort();
            }
            rt_arg param = {&*meth_iter, param_index++};
            meth_iter->vp.push_back(rt_class);
        }

        meth_iter->specs.resize(meth_info.specs.size());
        auto spec_iter = meth_iter->specs.rbegin();
        // reverse the reversed order from 'chain'

        for (auto& definition_info : meth_info.specs) {
            ++trace << definition_info.name << " (" << definition_info.pf
                    << ")\n";
            spec_iter->info = &definition_info;
            spec_iter->vp.reserve(meth_info.arity());
            size_t param_index = 0;

            for (auto ti :
                 type_range{definition_info.vp_begin, definition_info.vp_end}) {
                indent YOMM2_GENSYM(trace);
                auto rt_class = class_map[Policy::type_index(ti)];
                if (!rt_class) {
                    ++trace << "error for *virtual* parameter #"
                            << (param_index + 1) << "\n";
                    unknown_class_error error;
                    error.ti = ti;
                    Policy::error(error_type(error));
                    abort();
                }
                spec_iter->vp.push_back(rt_class);
                ++param_index;
            }
            ++spec_iter;
        }

        ++meth_iter;
    }

    for (auto& method : methods) {
        size_t param_index = 0;

        for (auto vp : method.vp) {
            vp->used_by_vp.push_back({&method, param_index++});
        }
    }
}

template<class Policy>
std::vector<rt_class*> runtime<Policy>::layer_classes() {
    ++trace << "Layering classes...\n";

    std::vector<rt_class*> input;
    input.reserve(classes.size());
    std::transform(
        classes.begin(), classes.end(), std::back_inserter(input),
        [](rt_class& cls) { return &cls; });

    std::vector<rt_class*> layered;
    layered.reserve(classes.size());

    for (int layer = 1; !input.empty(); ++layer) {
        indent YOMM2_GENSYM(trace, 1);
        ++trace;

        for (auto class_iter = input.begin(); class_iter != input.end();) {
            auto seen_all_bases = true;
            auto in_this_layer = (*class_iter)->direct_bases.empty();

            for (auto base : (*class_iter)->direct_bases) {
                if (!base->layer) {
                    seen_all_bases = false;
                    break;
                } else if (base->layer == layer) {
                    in_this_layer = false;
                    break;
                }
                if (base->layer == layer - 1) {
                    in_this_layer = true;
                }
            }

            if (seen_all_bases && in_this_layer) {
                layered.push_back(*class_iter);
                (*class_iter)->layer = layer;

                if constexpr (trace_enabled) {
                    trace << " " << **class_iter;
                }

                class_iter = input.erase(class_iter);
            } else {
                ++class_iter;
            }
        }
        trace << "\n";
    }

    return std::move(layered);
}

template<class Policy>
void runtime<Policy>::allocate_slots() {
    auto layered = layer_classes();

    ++trace << "Allocating slots...\n";
    indent YOMM2_GENSYM(trace);

    for (auto cls : layered) {
        for (const auto& mp : cls->used_by_vp) {
            size_t slot = cls->next_slot++;

            ++trace << mp.method->info->name << "#" << mp.param << ": slot "
                    << slot << "\n";
            indent YOMM2_GENSYM(trace);
            ++trace << *cls;

            if (mp.method->slots.size() <= mp.param) {
                mp.method->slots.resize(mp.param + 1);
            }

            mp.method->slots[mp.param] = slot;

            if (cls->first_used_slot == -1) {
                cls->first_used_slot = slot;
            }

            cls->mark = ++class_visit;

            for (auto derived : cls->direct_derived) {
                allocate_slot_down(derived, slot);
            }

            ++trace << "\n";
        }
    }

    for (auto& c : classes) {
        c.mtbl.resize(c.next_slot);
    }
}

template<class Policy>
void runtime<Policy>::allocate_slot_down(rt_class* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << *cls;

    assert(slot >= cls->next_slot);

    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto b : cls->direct_bases) {
        allocate_slot_up(b, slot);
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_down(d, slot);
    }
}

template<class Policy>
void runtime<Policy>::allocate_slot_up(rt_class* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << *cls;

    assert(slot >= cls->next_slot);
    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto b : cls->direct_bases) {
        allocate_slot_up(b, slot);
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_down(d, slot);
    }
}

template<class Policy>
void runtime<Policy>::build_dispatch_tables() {
    for (auto& m : methods) {
        ++trace << "Building dispatch table for " << m.info->name << "\n";
        indent YOMM2_GENSYM(trace);

        auto dims = m.arity();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            size_t dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];
                ++trace << "make groups for param #" << dim << ", class " << *vp
                        << "\n";
                indent YOMM2_GENSYM(trace);

                for (auto compatible_classes : vp->compatible_classes) {
                    ++trace << "specs applicable to " << *compatible_classes
                            << "\n";
                    bitvec mask;
                    mask.resize(m.specs.size());

                    size_t spec_index = 0;
                    indent YOMM2_GENSYM(trace);

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->compatible_classes.find(
                                compatible_classes) !=
                            spec.vp[dim]->compatible_classes.end()) {
                            ++trace << spec.info->name << "\n";
                            mask[spec_index] = 1;
                        }
                        ++spec_index;
                    }

                    auto& group = dim_group[mask];
                    group.classes.push_back(compatible_classes);
                    group.has_concrete_classes = group.has_concrete_classes ||
                        !compatible_classes->is_abstract;

                    ++trace << "-> mask: " << mask << "\n";
                }

                ++dim;
            }
        }

        {
            size_t stride = 1;
            m.strides.reserve(dims - 1);

            for (size_t dim = 1; dim < m.arity(); ++dim) {
                stride *= groups[dim - 1].size();
                ++trace << "    stride for dim " << dim << " = " << stride
                        << "\n";
                m.strides.push_back(stride);
            }
        }

        for (size_t dim = 0; dim < m.arity(); ++dim) {
            ++trace << "groups for dim " << dim << ":\n";
            indent YOMM2_GENSYM(trace);
            size_t group_num = 0;
            for (auto& [mask, group] : groups[dim]) {
                for (auto cls : group.classes) {
                    cls->mtbl[m.slots[dim]] = group_num;
                }
                if constexpr (trace_enabled) {
                    ++trace << "group " << dim << "/" << group_num << " mask "
                            << mask << "\n";
                    indent YOMM2_GENSYM(trace);
                    for (auto cls : type_range{
                             group.classes.begin(), group.classes.end()}) {
                        ++trace << cls->ti_ptrs[0] << "\n";
                    }
                }
                ++group_num;
            }
        }

        {
            ++trace << "assigning specs\n";
            bitvec all(m.specs.size());
            all = ~all;
            build_dispatch_table(m, dims - 1, groups.end() - 1, all, false);

            if (m.arity() > 1) {
                indent YOMM2_GENSYM(trace);
                m.stats.cells = 1;
                ++trace << "dispatch table rank: ";
                const char* prefix = "";
                for (const auto& dim_groups : groups) {
                    m.stats.cells *= dim_groups.size();
                    trace << prefix << dim_groups.size();
                    prefix = " x ";
                }

                m.stats.concrete_cells = 1;
                prefix = ", concrete only: ";
                for (const auto& dim_groups : groups) {
                    auto cells = std::count_if(
                        dim_groups.begin(), dim_groups.end(),
                        [](const auto& group) {
                            return group.second.has_concrete_classes;
                        });
                    m.stats.concrete_cells *= cells;
                    trace << prefix << cells;
                    prefix = " x ";
                }
                trace << "\n";
            }

            print(m.stats);
            metrics.accumulate(m.stats);
            ++trace << "assigning next\n";

            std::vector<const rt_spec*> specs;
            std::transform(
                m.specs.begin(), m.specs.end(), std::back_inserter(specs),
                [](const rt_spec& spec) { return &spec; });

            for (auto& spec : m.specs) {
                indent YOMM2_GENSYM(trace);
                ++trace << spec.info->name << ":\n";
                std::vector<const rt_spec*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(), std::back_inserter(candidates),
                    [spec](const rt_spec* other) {
                        return is_base(other, &spec);
                    });

                if constexpr (trace_enabled) {
                    indent YOMM2_GENSYM(trace);
                    ++trace << "for next, select best:\n";

                    for (auto& candidate : candidates) {
                        indent YOMM2_GENSYM(trace);
                        ++trace << candidate->info->name << "\n";
                    }
                }

                auto nexts = best(candidates);
                void* next;

                if (nexts.size() == 1) {
                    const definition_info* next_info = nexts.front()->info;
                    next = next_info->pf;
                    ++trace << "-> " << next_info->name << "\n";
                } else if (nexts.empty()) {
                    ++trace << "-> none\n";
                    next = m.info->not_implemented;
                } else if (nexts.size() > 1) {
                    ++trace << "->  ambiguous\n";
                    next = m.info->ambiguous;
                }

                if (spec.info->next) {
                    *spec.info->next = next;
                }
            }
        }
    }
}

template<class Policy>
void runtime<Policy>::build_dispatch_table(
    rt_method& m, size_t dim, std::vector<group_map>::const_iterator group_iter,
    const bitvec& candidates, bool concrete) {
    indent YOMM2_GENSYM(trace);
    size_t group_index = 0;

    for (const auto& [group_mask, group] : *group_iter) {
        auto mask = candidates & group_mask;

        if constexpr (trace_enabled) {
            ++trace << "group " << dim << "/" << group_index << " mask " << mask
                    << "\n";
            indent YOMM2_GENSYM(trace);
            for (auto cls :
                 type_range{group.classes.begin(), group.classes.end()}) {
                ++trace << cls->ti_ptrs[0] << "\n";
            }
        }

        if (dim == 0) {
            std::vector<const rt_spec*> applicable;
            size_t i = 0;

            for (const auto& spec : m.specs) {
                if (mask[i]) {
                    applicable.push_back(&spec);
                }
                ++i;
            }

            if constexpr (trace_enabled) {
                ++trace << "select best of:\n";
                indent YOMM2_GENSYM(trace);

                for (auto& app : applicable) {
                    ++trace << app->info->name << "\n";
                }
            }

            auto specs = best(applicable);

            if (specs.size() > 1) {
                indent YOMM2_GENSYM(trace);
                ++trace << "ambiguous\n";
                m.dispatch_table.push_back(m.info->ambiguous);
                ++m.stats.ambiguous;
                if (concrete) {
                    ++m.stats.concrete_ambiguous;
                }
            } else if (specs.empty()) {
                indent YOMM2_GENSYM(trace);
                ++trace << "not implemented\n";
                m.dispatch_table.push_back(m.info->not_implemented);
                ++m.stats.not_implemented;
                if (concrete) {
                    ++m.stats.concrete_not_implemented;
                }
            } else {
                m.dispatch_table.push_back(specs[0]->info->pf);
                ++trace << "-> " << specs[0]->info->name
                        << " pf = " << specs[0]->info->pf << "\n";
            }
        } else {
            build_dispatch_table(
                m, dim - 1, group_iter - 1, mask,
                concrete && group.has_concrete_classes);
        }
        ++group_index;
    }
}

template<class Policy>
size_t runtime<Policy>::find_hash_function(
    const std::deque<rt_class>& classes, metrics_t& metrics) {
    using namespace policy;

    if constexpr (has_facet<Policy, projection>) {
        std::vector<type_id> type_ids;

        for (auto& cls : classes) {
            std::copy(
                cls.ti_ptrs.begin(), cls.ti_ptrs.end(),
                std::back_inserter(type_ids));
        }

        return Policy::project_type_ids(type_ids);
    } else {
        type_id max_id = 0;

        for (auto& cls : classes) {
            for (auto type : cls.ti_ptrs) {
                max_id = std::max(max_id, type);
            }
        }

        return max_id + 1;
    }
}

inline void dispatch_stats_t::accumulate(const dispatch_stats_t& other) {
    cells += other.cells;
    concrete_cells += other.concrete_cells;
    not_implemented += other.not_implemented;
    concrete_not_implemented += other.concrete_not_implemented;
    ambiguous += other.ambiguous;
    concrete_ambiguous += other.concrete_ambiguous;
}

template<class Policy>
void operator+=(std::vector<word>& words, const std::vector<int>& ints) {
    words.reserve(words.size() + ints.size());
    for (auto i : ints) {
        word w;
        w.i = i;
        words.push_back(w);
    }
}

template<class Policy>
void runtime<Policy>::install_gv(size_t type_ids) {
    Policy::context.mptrs.resize(type_ids);
    Policy::context.indirect_mptrs.resize(type_ids);

    for (size_t pass = 0; pass != 2; ++pass) {
        Policy::context.gv.resize(0);

        if constexpr (trace_enabled) {
            if (pass) {
                ++trace << "Initializing dispatch tables at "
                        << Policy::context.gv.data() << "\n";
            }
        }

        for (auto& m : methods) {
            if (m.info->arity() == 1) {
                // Uni-methods just need an index in the method table.
                *m.info->slots_strides_p = m.slots[0];
                continue;
            }

            // multi-methods only

            auto slot_iter = m.slots.begin();
            auto stride_iter = m.strides.begin();
            auto offsets_iter = m.info->slots_strides_p;
            *offsets_iter++ = *slot_iter++;

            while (slot_iter != m.slots.end()) {
                *offsets_iter++ = *slot_iter++;
                *offsets_iter++ = *stride_iter++;
            }

            if constexpr (trace_enabled) {
                if (pass) {
                    ++trace
                        << rflush(4, Policy::context.gv.size()) << " "
                        << Policy::context.gv.data() + Policy::context.gv.size()
                        << " dispatch table for " << m.info->name << "\n";
                }
            }

            m.gv_dispatch_table =
                Policy::context.gv.data() + Policy::context.gv.size();
            std::transform(
                m.dispatch_table.begin(), m.dispatch_table.end(),
                std::back_inserter(Policy::context.gv),
                [](void* pf) { return make_word(pf); });
        }

        for (auto& cls : classes) {
            if (cls.first_used_slot == -1) {
                // corner case: no methods for this class
                *cls.method_table =
                    Policy::context.gv.data() + Policy::context.gv.size();
            } else {
                *cls.method_table = Policy::context.gv.data() +
                    Policy::context.gv.size() - cls.first_used_slot;
            }
            if constexpr (trace_enabled) {
                if (pass) {
                    ++trace << rflush(4, Policy::context.gv.size()) << " "
                            << *cls.method_table << " mtbl for " << cls << "\n";
                }
            }

            if (cls.first_used_slot != -1) {
                std::transform(
                    cls.mtbl.begin() + cls.first_used_slot, cls.mtbl.end(),
                    std::back_inserter(Policy::context.gv),
                    [](size_t i) { return make_word(i); });
            }

            if (pass) {
                for (auto type : cls.ti_ptrs) {
                    using namespace policy;
                    auto index = type;

                    if constexpr (has_facet<Policy, projection>) {
                        index = Policy::project_type_id(index);
                    }

                    Policy::context.mptrs[index] = *cls.method_table;
                    Policy::context.indirect_mptrs[index] = cls.method_table;
                }
            }
        }
    }

    ++trace << rflush(4, Policy::context.gv.size()) << " "
            << Policy::context.gv.data() + Policy::context.gv.size()
            << " end\n";
}

template<class Policy>
void runtime<Policy>::optimize() {
    ++trace << "Optimizing\n";

    for (auto& m : methods) {
        ++trace << "  " << m.info->name << "\n";
        indent YOMM2_GENSYM(trace);
        auto slot = m.slots[0];

        if (m.arity() == 1) {
            for (auto cls : m.vp[0]->compatible_classes) {
                auto pf = m.dispatch_table[(*cls->method_table)[slot].i];
                if constexpr (trace_enabled) {
                    ++trace << *cls << " mtbl[" << slot
                            << "] = " << pf << " (function)"
                            << "\n";
                }
                (*cls->method_table)[slot].pf = pf;
            }
        } else {
            for (auto cls : m.vp[0]->compatible_classes) {
                auto pw = m.gv_dispatch_table + (*cls->method_table)[slot].i;

                if constexpr (trace_enabled) {
                    ++trace << *cls << " mtbl[" << slot << "] = gv+"
                            << (pw - Policy::context.gv.data()) << "\n";
                }

                (*cls->method_table)[slot].pw = pw;
            }
        }
    }
}

template<class Policy>
std::vector<const rt_spec*>
runtime<Policy>::best(std::vector<const rt_spec*>& candidates) {
    std::vector<const rt_spec*> best;

    for (auto spec : candidates) {
        const rt_spec* candidate = spec;

        for (auto iter = best.begin(); iter != best.end();) {
            if (is_more_specific(spec, *iter)) {
                iter = best.erase(iter);
            } else if (is_more_specific(*iter, spec)) {
                candidate = nullptr;
                break;
            } else {
                ++iter;
            }
        }

        if (candidate) {
            best.push_back(candidate);
        }
    }

    return best;
}

template<class Policy>
bool runtime<Policy>::is_more_specific(const rt_spec* a, const rt_spec* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*b_iter)->compatible_classes.find(*a_iter) !=
                (*b_iter)->compatible_classes.end()) {
                result = true;
            } else if (
                (*a_iter)->compatible_classes.find(*b_iter) !=
                (*a_iter)->compatible_classes.end()) {
                return false;
            }
        }
    }

    return result;
}

template<class Policy>
bool runtime<Policy>::is_base(const rt_spec* a, const rt_spec* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*a_iter)->compatible_classes.find(*b_iter) ==
                (*a_iter)->compatible_classes.end()) {
                return false;
            } else {
                result = true;
            }
        }
    }

    return result;
}

template<class Policy>
void runtime<Policy>::print(const dispatch_stats_t& stats) {
    ++trace;
    if (stats.cells) {
        // only for multi-methods, uni-methods don't have dispatch tables
        ++trace << stats.cells << " dispatch table cells, ";
    }
    trace << stats.not_implemented << " not implemented, ";
    trace << stats.ambiguous << " ambiguities, concrete only: ";
    if (stats.cells) {
        trace << stats.concrete_cells << ", ";
    }
    trace << stats.concrete_not_implemented << ", ";
    trace << stats.concrete_ambiguous << "\n";
}

} // namespace detail

namespace policy {

template<class Policy>
template<typename Container>
size_t fast_projection<Policy>::project_type_ids(
    const Container& type_ids, std::vector<type_id>& buckets) {
    using namespace policy;

    const auto N = type_ids.size();

    if constexpr (has_facet<Policy, output>) {
        Policy::stream << "Finding hash factor for " << N << " types\n";
    }

    std::default_random_engine rnd(13081963);
    size_t total_attempts = 0;
    size_t M = 1;

    for (auto size = N * 5 / 4; size >>= 1;) {
        ++M;
    }

    std::uniform_int_distribution<type_id> uniform_dist;

    for (size_t pass = 0; pass < 4; ++pass, ++M) {
        shift = 8 * sizeof(type_id) - M;
        auto hash_size = 1 << M;

        if constexpr (has_facet<Policy, output>) {
            Policy::stream << "  trying with M = " << M << ", " << hash_size
                           << " buckets\n";
        }

        bool found = false;
        size_t attempts = 0;
        buckets.resize(hash_size);

        while (!found && attempts < 100000) {
            std::fill(buckets.begin(), buckets.end(), invalid);
            ++attempts;
            ++total_attempts;
            found = true;
            mult = uniform_dist(rnd) | 1;

            for (auto type : type_ids) {
                auto index = (type * mult) >> shift;

                if (buckets[index] != invalid) {
                    found = false;
                    break;
                }

                buckets[index] = type;
            }
        }

        // metrics.hash_search_attempts = total_attempts;
        // metrics.hash_search_time =
        //     std::chrono::steady_clock::now() - start_time;
        // metrics.hash_table_size = hash_size;

        if (found) {
            if constexpr (has_facet<Policy, output>) {
                Policy::stream << "  found " << mult << " after "
                               << total_attempts << " attempts\n";
            }
            return buckets.size();
        }
    }

    hash_search_error error;
    error.attempts = total_attempts;
    // error.duration = std::chrono::steady_clock::now() - start_time;
    error.buckets = 1 << M;
    Policy::error(error_type(error));

    abort();
}

} // namespace policy

template<class Policy>
void update() {
    detail::runtime<Policy> rt;
    rt.update();
}

} // namespace yomm2
} // namespace yorel

#pragma pop_macro("max")

#endif
