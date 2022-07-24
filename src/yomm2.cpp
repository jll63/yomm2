// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm> // for max, transform, copy
#include <cassert>   // for assert
#include <chrono>    // for operator-, duration
#include <cstdint>   // for uintptr_t
#include <cstdlib>   // for abort, getenv
#include <iomanip>   // for operator<<, setw
#include <iostream>  // for operator<<, ostream
#include <iterator>  // for back_insert_iterator
#include <list>      // for list, _List_iterator
#include <map>       // for map
#include <memory>    // for allocator_traits<...
#include <random>    // for default_random_en...
#include <stdexcept> // for runtime_error
#include <string>    // for char_traits, allo...
#include <string_view>
#include <typeinfo>      // for type_info
#include <unordered_map> // for _Node_iterator
#include <unordered_set> // for unordered_set<>::...
#include <utility>       // for pair
#include <vector>        // for vector, vector<>:...

#if defined(YOMM2_TRACE) && (YOMM2_TRACE & 1) || !defined(NDEBUG)
#include <iostream>
#endif

#include <boost/dynamic_bitset/dynamic_bitset.hpp> // for operator<<, dynam...

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

std::ostream* logs;
unsigned trace_flags;

template<unsigned Flags>
inline trace_type<Flags>& trace_type<Flags>::operator++() {
    if constexpr (bool(trace_enabled & Flags)) {
        if (trace_flags & Flags) {
            for (size_t n = indent; n--;)
                *logs << " ";
        }
    }

    return *this;
}

template<unsigned Flags>
struct with_indent {
    trace_type<Flags>& trace;
    int by;

    explicit with_indent(trace_type<Flags>& trace, int by = 2)
        : trace(trace), by(by) {
        trace.indent += by;
    }

    ~with_indent() {
        trace.indent -= by;
    }
};

template<typename Iterator>
struct range {
    Iterator first, last;
    Iterator begin() const {
        return first;
    }
    Iterator end() const {
        return last;
    }
};

template<typename Iterator>
range(Iterator b, Iterator e) -> range<Iterator>;

struct tip {
    const ti_ptr ptr;
};

std::ostream& operator<<(std::ostream& os, tip t) {
    return os << t.ptr->name() << "(" << t.ptr << ")";
}

std::ostream& operator<<(std::ostream& os, const range<const ti_ptr*>& tips) {
    os << "(";
    const char* sep = "";
    for (auto t : tips) {
        os << sep << tip{t};
        sep = ", ";
    }

    return os << ")";
}

std::ostream* log_on(std::ostream* os) {
    auto prev = logs;
    logs = os;
    return prev;
}

std::ostream* log_off() {
    auto prev = logs;
    logs = nullptr;
    return prev;
}

template<template<typename...> typename Container, typename... T>
std::ostream&
operator<<(std::ostream& os, Container<rt_class*, T...>& classes) {
    os << "(";
    const char* sep = "";
    for (auto cls : classes) {
        os << sep << cls->info->name();
        sep = ", ";
    }

    return os << ")";
}

void dispatch_stats_t::accumulate(const dispatch_stats_t& other) {
    cells += other.cells;
    concrete_cells += other.concrete_cells;
    not_implemented += other.not_implemented;
    concrete_not_implemented += other.concrete_not_implemented;
    ambiguous += other.ambiguous;
    concrete_ambiguous += other.concrete_ambiguous;
}

void runtime::update() {
    augment_classes();
    augment_methods();
    allocate_slots();
    build_dispatch_tables();
    find_hash_function(classes, ctx.hash, metrics);
    install_gv();
    optimize();
    print(metrics);
    ++trace << "Finished\n";
}

runtime::runtime(catalog& cat, struct context& ctx) : cat(cat), ctx(ctx) {
    if constexpr (bool(trace_enabled)) {
        if (auto env_trace = getenv("YOMM2_TRACE")) {
            log_on(&std::cerr);
            trace_flags = std::atoi(env_trace);
        }
    }
}

void runtime::augment_classes() {
    // scope
    {
        ++trace << "Static class info:\n";

        // The standard does not guarantee that there is exactly one type_info
        // object per class. However, it guarantees that the type_index for a
        // class has a unique value.
        for (auto& cr : cat.classes) {
            if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                {
                    with_indent YOMM2_GENSYM(trace);
                    ++trace << cr.ti << " " << cr.name() << " "
                            << range{cr.first_base, cr.last_base};

                    ++trace << "\n";
                }
            }

            auto& entry = class_map[std::type_index(*cr.ti)];

            if (entry == nullptr) {
                classes.emplace_front(&cr);
                entry = &classes.front();
            }

            // In the unlikely case that a class does have more than one
            // associated  ti*, collect them in a vector. We don't use an
            // unordered_set because, again, this situation is highly unlikely,
            // and, were it to occurr, the number of distinct ti*s would
            // probably be small.
            if (std::find(
                    entry->ti_ptrs.begin(), entry->ti_ptrs.end(), cr.ti) ==
                entry->ti_ptrs.end()) {
                entry->ti_ptrs.push_back(cr.ti);
            }
        }
    }

    // All known classes now have exactly one associated rt_class* in the map.
    // Collect the bases.

    for (auto& rtc : classes) {
        for (auto base_iter = rtc.info->first_base;
             base_iter != rtc.info->last_base; ++base_iter) {
            auto rtb = class_map[std::type_index(**base_iter)];

            if (!rtb) {
                unknown_class_error error;
                error.ti = *base_iter;
                error_handler(error_type(error));
                abort();
            }

            if (&rtc != rtb) {
                // At compile time we collected the class as its own improper
                // base, as per std::is_base_of. Eliminate that.
                rtc.transitive_bases.push_back(rtb);
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

        // Record the "weight" of the class, i.e. the total number of direct and
        // indirect proper bases it has.
        rtc.weight = bases.size();
        rtc.transitive_bases.swap(bases);
    }

    for (auto& rtc : classes) {
        // Sort base classes by weight. This ensures that a base class is never
        // preceded by one if its own bases classes.
        std::sort(
            rtc.transitive_bases.begin(), rtc.transitive_bases.end(),
            [](auto a, auto b) { return a->weight > b->weight; });
        mark = ++class_visit;

        // Collect the direct base classes. The first base is certainly a direct
        // one. Remove *its* bases from the candidates, by marking them.
        // Continue with the next base that is not marked. It is the next direct
        // base. And so on...

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
        calculate_conforming_classes(rtc);
    }

    if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
        ++trace << "Inheritance:\n";
        for (auto& rtc : classes) {
            with_indent YOMM2_GENSYM(trace);
            ++trace << rtc.name() << "\n";
            {
                with_indent YOMM2_GENSYM(trace);
                ++trace << "bases:      " << rtc.direct_bases << "\n";
                ++trace << "derived:    " << rtc.direct_derived << "\n";
                ++trace << "covariant_classes: " << rtc.covariant_classes
                        << "\n";
            }
        }
    }
}

// void runtime::calculate_conforming_classes() {
//     ++trace << "Conforming classes...\n";
//     with_indent YOMM2_GENSYM(trace);

//     for (auto& cls : classes) {
//         if (cls.covariant_classes.empty()) {
//             calculate_conforming_classes(cls);
// #if YOMM2_TRACE
//             ++trace << cls.info->name() << ":\n";
//             with_indent YOMM2_GENSYM(trace);
//             for (auto conf : cls.covariant_classes) {
//                 ++trace << tip{conf->info->ti} << "\n";
//             }
// #endif
//         }
//     }
// }

void runtime::calculate_conforming_classes(rt_class& cls) {
    if (!cls.covariant_classes.empty()) {
        return;
    }

    cls.covariant_classes.insert(&cls);

    for (auto derived : cls.direct_derived) {
        if (derived->covariant_classes.empty()) {
            calculate_conforming_classes(*derived);
        }

        std::copy(
            derived->covariant_classes.begin(),
            derived->covariant_classes.end(),
            std::inserter(cls.covariant_classes, cls.covariant_classes.end()));
    }
}

void runtime::augment_methods() {
    methods.resize(cat.methods.size());

    ++trace << "Methods:\n";
    with_indent YOMM2_GENSYM(trace);

    auto meth_iter = methods.rbegin();
    // reverse the registration order reversed by 'chain'.

    for (auto& meth_info : cat.methods) {
        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            ++trace << meth_info.name << " "
                    << range{meth_info.vp_begin, meth_info.vp_end} << "\n";
        }

        with_indent YOMM2_GENSYM(trace);

        meth_iter->info = &meth_info;
        meth_iter->vp.reserve(meth_info.arity());
        size_t param_index = 0;

        for (auto ti : range{meth_info.vp_begin, meth_info.vp_end}) {
            auto rt_class = class_map[std::type_index(*ti)];
            if (!rt_class) {
                ++trace << "unkown class " << ti << "(" << ti->name()
                        << ") for parameter #" << (param_index + 1) << "\n";
                unknown_class_error error;
                error.ti = ti;
                error_handler(error_type(error));
                abort();
            }
            rt_arg param = {&*meth_iter, param_index++};
            meth_iter->vp.push_back(rt_class);
        }

        meth_iter->specs.resize(meth_info.specs.size());
        auto spec_iter = meth_iter->specs.rbegin();
        // reverse the reversed order from 'chain'

        for (auto& definition_info : meth_info.specs) {
            ++trace << definition_info.name << "\n";
            spec_iter->info = &definition_info;
            spec_iter->vp.reserve(meth_info.arity());
            size_t param_index = 0;

            for (auto ti :
                 range{definition_info.vp_begin, definition_info.vp_end}) {
                with_indent YOMM2_GENSYM(trace);
                auto rt_class = class_map[std::type_index(*ti)];
                if (!rt_class) {
                    ++trace << "error for *virtual* parameter #"
                            << (param_index + 1) << "\n";
                    unknown_class_error error;
                    error.ti = ti;
                    error_handler(error_type(error));
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

std::vector<rt_class*> runtime::layer_classes() {
    ++trace << "Layering classes...\n";

    std::vector<rt_class*> input;
    input.reserve(classes.size());
    std::transform(
        classes.begin(), classes.end(), std::back_inserter(input),
        [](rt_class& cls) { return &cls; });

    std::vector<rt_class*> layered;
    layered.reserve(classes.size());

    for (int layer = 1; !input.empty(); ++layer) {
        with_indent YOMM2_GENSYM(trace, 1);
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

                if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                    trace << " " << (*class_iter)->info->name();
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

void runtime::allocate_slots() {
    auto layered = layer_classes();

    ++trace << "Allocating slots...\n";
    with_indent YOMM2_GENSYM(trace);

    for (auto cls : layered) {
        for (const auto& mp : cls->used_by_vp) {
            size_t slot = cls->next_slot++;

            ++trace << mp.method->info->name << "#" << mp.param << ": slot "
                    << slot << "\n";
            with_indent YOMM2_GENSYM(trace);
            ++trace << cls->info->name();

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

void runtime::allocate_slot_down(rt_class* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << cls->info->name();

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

void runtime::allocate_slot_up(rt_class* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << cls->info->name();

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

void runtime::build_dispatch_tables() {
    for (auto& m : methods) {
        ++trace << "Building dispatch table for " << m.info->name << "\n";
        with_indent YOMM2_GENSYM(trace);

        auto dims = m.arity();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            size_t dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];
                ++trace << "make groups for param #" << dim << ", class "
                        << vp->info->name() << "\n";
                with_indent YOMM2_GENSYM(trace);

                for (auto covariant_classes : vp->covariant_classes) {
                    ++trace << "specs applicable to "
                            << covariant_classes->info->name() << "\n";
                    bitvec mask;
                    mask.resize(m.specs.size());

                    size_t spec_index = 0;
                    with_indent YOMM2_GENSYM(trace);

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->covariant_classes.find(
                                covariant_classes) !=
                            spec.vp[dim]->covariant_classes.end()) {
                            ++trace << spec.info->name << "\n";
                            mask[spec_index] = 1;
                        }
                        ++spec_index;
                    }

                    auto& group = dim_group[mask];
                    group.classes.push_back(covariant_classes);
                    group.has_concrete_classes = group.has_concrete_classes ||
                        !covariant_classes->info->is_abstract;

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
            with_indent YOMM2_GENSYM(trace);
            size_t group_num = 0;
            for (auto& [mask, group] : groups[dim]) {
                for (auto cls : group.classes) {
                    cls->mtbl[m.slots[dim]] = group_num;
                }
                if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                    ++trace << "group " << dim << "/" << group_num << " mask "
                            << mask << "\n";
                    with_indent YOMM2_GENSYM(trace);
                    for (auto cls :
                         range{group.classes.begin(), group.classes.end()}) {
                        ++trace << tip{cls->info->ti} << "\n";
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
                with_indent YOMM2_GENSYM(trace);
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
                with_indent YOMM2_GENSYM(trace);
                ++trace << spec.info->name << ":\n";
                std::vector<const rt_spec*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(), std::back_inserter(candidates),
                    [spec](const rt_spec* other) {
                        return is_base(other, &spec);
                    });

                if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                    with_indent YOMM2_GENSYM(trace);
                    ++trace << "for next, select best:\n";

                    for (auto& candidate : candidates) {
                        with_indent YOMM2_GENSYM(trace);
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

void runtime::build_dispatch_table(
    rt_method& m, size_t dim, std::vector<group_map>::const_iterator group_iter,
    const bitvec& candidates, bool concrete) {
    with_indent YOMM2_GENSYM(trace);
    size_t group_index = 0;

    for (const auto& [group_mask, group] : *group_iter) {
        auto mask = candidates & group_mask;

        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            ++trace << "group " << dim << "/" << group_index << " mask " << mask
                    << "\n";
            with_indent YOMM2_GENSYM(trace);
            for (auto cls : range{group.classes.begin(), group.classes.end()}) {
                ++trace << tip{cls->info->ti} << "\n";
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

            if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                ++trace << "select best of:\n";
                with_indent YOMM2_GENSYM(trace);

                for (auto& app : applicable) {
                    ++trace << app->info->name << "\n";
                }
            }

            auto specs = best(applicable);

            if (specs.size() > 1) {
                with_indent YOMM2_GENSYM(trace);
                ++trace << "ambiguous\n";
                m.dispatch_table.push_back(m.info->ambiguous);
                ++m.stats.ambiguous;
                if (concrete) {
                    ++m.stats.concrete_ambiguous;
                }
            } else if (specs.empty()) {
                with_indent YOMM2_GENSYM(trace);
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

void runtime::find_hash_function(
    const std::deque<rt_class>& classes, hash_function& hash,
    metrics_t& metrics) {
    std::vector<const void*> keys;
    auto start_time = std::chrono::steady_clock::now();

    for (auto& cls : classes) {
        std::copy(
            cls.ti_ptrs.begin(), cls.ti_ptrs.end(), std::back_inserter(keys));
    }

    const auto N = keys.size();

    ++trace << "Finding hash factor for " << N << " ti*\n";

    std::default_random_engine rnd(13081963);
    size_t total_attempts = 0;
    size_t M = 1;

    for (auto size = N * 5 / 4; size >>= 1;) {
        ++M;
    }

    std::uniform_int_distribution<std::uintptr_t> uniform_dist;

    for (size_t pass = 0; pass < 4; ++pass, ++M) {

        hash.shift = 8 * sizeof(std::uintptr_t) - M;
        auto hash_size = 1 << M;

        ++trace << "trying with M = " << M << ", " << hash_size << " buckets\n";

        bool found = false;
        size_t attempts = 0;
        std::vector<int> buckets(hash_size);

        while (!found && attempts < 100000) {
            ++attempts;
            ++total_attempts;
            found = true;
            hash.mult = uniform_dist(rnd) | 1;

            for (auto key : keys) {
                auto h = hash(key);
                if (buckets[h]++) {
                    found = false;
                    break;
                }
            }

            std::fill(buckets.begin(), buckets.end(), 0);
        }

        metrics.hash_search_attempts = total_attempts;
        metrics.hash_search_time =
            std::chrono::steady_clock::now() - start_time;
        metrics.hash_table_size = hash_size;

        if (found) {
            ++trace << "found " << hash.mult << " after " << total_attempts
                    << " attempts and "
                    << metrics.hash_search_time.count() * 1000 << " msecs\n";
            return;
        }
    }

    hash_search_error error;
    error.attempts = total_attempts;
    error.duration = std::chrono::steady_clock::now() - start_time;
    error.buckets = 1 << M;
    error_handler(error_type(error));
    abort();
}

void operator+=(std::vector<word>& words, const std::vector<int>& ints) {
    words.reserve(words.size() + ints.size());
    for (auto i : ints) {
        word w;
        w.i = i;
        words.push_back(w);
    }
}

void runtime::install_gv() {

    for (size_t pass = 0; pass != 2; ++pass) {
        ctx.gv.resize(0);

        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            if (pass) {
                ++trace << "Initializing global vector at " << ctx.gv.data()
                        << "\n"
                        << std::setw(4) << ctx.gv.size()
                        << " pointer to control table\n";
            }
        }

        // reserve a work for control table
        ctx.gv.emplace_back(make_word(nullptr));

        auto hash_table = ctx.gv.data() + 1;
        ctx.hash_table = hash_table;

        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            if (pass) {
                ++trace << std::setw(4) << ctx.gv.size() << " hash table\n";
            }
        }

        ctx.gv.resize(ctx.gv.size() + metrics.hash_table_size);

        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            ++trace << std::setw(4) << ctx.gv.size() << " control table\n";
        }

        ctx.gv.resize(ctx.gv.size() + metrics.hash_table_size);

        for (auto& m : methods) {
            if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                if (pass) {
                    ++trace << std::setw(4) << ctx.gv.size() << ' '
                            << m.info->name << "\n";
                }
            }

            m.info->slots_strides.pw = ctx.gv.data() + ctx.gv.size();
            m.info->install_hash_factors(*this);

            if (m.info->arity() == 1) {
                // Uni-methods just need an index in the method table.
                m.info->slots_strides.i = m.slots[0];
                continue;
            }

            // multi-methods only

            // TODO: experiment with putting the slots and strides in the
            // method object instead of the global vector. This should speed
            // up multi-methods a bit (one indirection), but not help
            // uni-methods.

            auto slot_iter = m.slots.begin();
            auto stride_iter = m.strides.begin();
            ctx.gv.emplace_back(make_word(*slot_iter++));

            while (slot_iter != m.slots.end()) {
                ctx.gv.emplace_back(make_word(*slot_iter++));
                ctx.gv.emplace_back(make_word(*stride_iter++));
            }

            if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                ++trace << std::setw(4) << ctx.gv.size() << ' ' << m.info->name
                        << " dispatch table\n";
            }

            m.gv_dispatch_table = ctx.gv.data() + ctx.gv.size();
            std::transform(
                m.dispatch_table.begin(), m.dispatch_table.end(),
                std::back_inserter(ctx.gv),
                [](void* pf) { return make_word(pf); });
        }

        auto control_table = hash_table + metrics.hash_table_size;
        ctx.gv[0].pw = control_table;

        for (auto& cls : classes) {
            cls.mptr = ctx.gv.data() + ctx.gv.size() - cls.first_used_slot;

            if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                ++trace << std::setw(4) << ctx.gv.size() << " mtbl for "
                        << cls.info->name() << ": " << cls.mptr << "\n";
            }

            if (cls.first_used_slot != -1) {
                std::transform(
                    cls.mtbl.begin() + cls.first_used_slot, cls.mtbl.end(),
                    std::back_inserter(ctx.gv),
                    [](size_t i) { return make_word(i); });
            }

            if (pass) {
                for (auto ti : cls.ti_ptrs) {
                    auto index = ctx.hash(ti);
                    hash_table[index].pw = cls.mptr;
                    control_table[index].ti = ti;
                }
            }
        }
    }

    ++trace << std::setw(4) << ctx.gv.size() << " end\n";
}

void runtime::optimize() {
    ++trace << "Optimizing\n";

    for (auto& m : methods) {
        ++trace << "  " << m.info->name << "\n";
        auto slot = m.slots[0];
        if (m.arity() == 1) {
            for (auto cls : m.vp[0]->covariant_classes) {
                auto pf = m.dispatch_table[cls->mptr[slot].i];
                if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                    ++trace << cls->info->name() << " mtbl[" << slot
                            << "] = " << pf << " (function)"
                            << "\n";
                }
                cls->mptr[slot].pf = pf;
            }
        } else {
            for (auto cls : m.vp[0]->covariant_classes) {
                auto pw = m.gv_dispatch_table + cls->mptr[slot].i;

                if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
                    ++trace << "    " << cls->info->name() << " mtbl[" << slot
                            << "] = gv+" << (pw - ctx.hash_table) << "\n";
                }

                cls->mptr[slot].pw = pw;
            }
        }
    }
}

std::vector<const rt_spec*>
runtime::best(std::vector<const rt_spec*>& candidates) {
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

bool runtime::is_more_specific(const rt_spec* a, const rt_spec* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*b_iter)->covariant_classes.find(*a_iter) !=
                (*b_iter)->covariant_classes.end()) {
                result = true;
            } else if (
                (*a_iter)->covariant_classes.find(*b_iter) !=
                (*a_iter)->covariant_classes.end()) {
                return false;
            }
        }
    }

    return result;
}

bool runtime::is_base(const rt_spec* a, const rt_spec* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*a_iter)->covariant_classes.find(*b_iter) ==
                (*a_iter)->covariant_classes.end()) {
                return false;
            } else {
                result = true;
            }
        }
    }

    return result;
}

void runtime::print(const dispatch_stats_t& stats) const {
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

void default_method_call_error_handler(
    const method_call_error& error, size_t arity, const ti_ptr ti_ptrs[]) {
    if constexpr (bool(trace_enabled)) {
        const char* explanation[] = {
            "no applicable definition", "ambiguous call"};
        std::cerr << explanation[error.code] << " for " << error.method_name
                  << "(";
        auto comma = "";
        for (auto ti : range{ti_ptrs, ti_ptrs + arity}) {
            std::cerr << comma << ti->name();
            comma = ", ";
        }
        std::cerr << ")\n" << std::flush;
    }
    abort();
}

method_call_error_handler method_call_error_handler_p =
    default_method_call_error_handler;

void default_error_handler(const error_type& error_v) {
    if (auto error = std::get_if<resolution_error>(&error_v)) {
        method_call_error old_error;
        old_error.code = error->status;
        old_error.method_name = error->method->name();
        method_call_error_handler_p(
            std::move(old_error), error->arity, error->tis);
        return;
    }

    if (auto error = std::get_if<unknown_class_error>(&error_v)) {
        if constexpr (bool(trace_enabled)) {
            std::cerr << "unknown class " << error->ti->name();
        }
        return;
    }

    if (auto error = std::get_if<hash_search_error>(&error_v)) {
        if constexpr (bool(trace_enabled & TRACE_RUNTIME)) {
            std::cerr << "could not find hash factors after " << error->attempts
                      << " in " << error->duration.count() << "s using "
                      << error->buckets << " buckets\n";
        }
        return;
    }
}

error_handler_type error_handler = default_error_handler;

void update_methods(catalog& cat, context& ht) {
    runtime rt(cat, ht);
    rt.update();
}

} // namespace detail

using namespace detail;

namespace policy {

catalog global_catalog::catalog;
context global_context::context;

void hash_factors_in_method::method_info_type::install_hash_factors(
    runtime& rt) {
    this->hash_table = rt.ctx.hash_table;
    this->hash = rt.ctx.hash;
}

} // namespace policy

void update_methods() {
    detail::update_methods(
        policy::default_policy::catalog, policy::default_policy::context);
}

error_handler_type set_error_handler(error_handler_type handler) {
    auto prev = detail::error_handler;
    detail::error_handler = handler;
    return prev;
}

method_call_error_handler
set_method_call_error_handler(method_call_error_handler handler) {
    auto prev = detail::method_call_error_handler_p;
    detail::method_call_error_handler_p = handler;
    return prev;
}

} // namespace yomm2
} // namespace yorel
