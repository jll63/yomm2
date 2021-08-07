// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>                                // for max, transform, copy
#include <cassert>                                  // for assert
#include <chrono>                                   // for operator-, duration
#include <cstdint>                                  // for uintptr_t
#include <cstdlib>                                 // for abort, getenv
#include <iomanip>                                  // for operator<<, setw
#include <iostream>                                 // for operator<<, ostream
#include <iterator>                                 // for back_insert_iterator
#include <list>                                     // for list, _List_iterator
#include <map>                                      // for map
#include <memory>                                   // for allocator_traits<...
#include <random>                                   // for default_random_en...
#include <stdexcept>                                // for runtime_error
#include <string>                                   // for char_traits, allo...
#include <typeinfo>                                 // for type_info
#include <unordered_map>                            // for _Node_iterator
#include <unordered_set>                            // for unordered_set<>::...
#include <utility>                                  // for pair
#include <vector>                                   // for vector, vector<>:...

#include <boost/dynamic_bitset/dynamic_bitset.hpp>  // for operator<<, dynam...

#include <yorel/yomm2.hpp>                          // for word, YOMM2_TRACE
#include <yorel/yomm2/runtime.hpp>                  // for rt_class, runtime

namespace yorel {
namespace yomm2 {

struct default_registry;

namespace detail {

#if YOMM2_ENABLE_TRACE

struct indent {
    explicit indent(int n) : n(n) {
        assert(n >= 0);
    }
    int n;
};

std::ostream& operator <<(std::ostream& os, const indent& i) {
    for (int n = i.n; n--; ) os << "  ";
    return os;
}

template<typename ITER, typename FUN>
struct outseq_t {
    outseq_t(ITER first, ITER last, FUN fun, int indent = 0)
    : first(first), last(last), fun(fun), indent(indent) { }
    ITER first, last;
    FUN fun;
    int indent;
};

template<typename ITER, typename FUN>
outseq_t<ITER, FUN> outseq(ITER first, ITER last, FUN fun) {
    return { first, last, fun };
}

template<typename ITER, typename FUN>
std::ostream& operator <<(std::ostream& os, const outseq_t<ITER, FUN>& s) {
    const char* sep = "";
    ITER iter = s.first;
    os << indent(s.indent);
    while (iter != s.last) {
        os << sep << s.fun(*iter++);
        sep = " ";
    }
    return os;
}

#endif

void update_methods(const registry& reg, dispatch_data& ht) {
    runtime rt(reg, ht);
    rt.update();
}

void runtime::update() {
    augment_classes();
    layer_classes();
    calculate_conforming_classes();
    augment_methods();
    allocate_slots();
    build_dispatch_tables();
    find_hash_function(classes, dd.hash, metrics);
    install_gv();
    optimize();
    YOMM2_TRACE(log() << "Finished\n");
}

runtime::runtime(const registry& reg, struct dispatch_data& dd) : reg(reg), dd(dd) {
}

void runtime::augment_classes() {
    classes.resize(reg.classes.size());

    // scope
    {
        auto class_iter = reg.classes.begin();

        for (auto& rt_class : classes) {
            rt_class.info = *class_iter++;
            class_map[rt_class.info] = &rt_class;
        }
    }

    for (auto& rt_class : classes) {
        rt_class.direct_bases.resize(rt_class.info->direct_bases.size());
        auto base_iter = rt_class.info->direct_bases.begin();

        for (auto& rt_base : rt_class.direct_bases) {
            rt_base = class_map[*base_iter++];
            if (!rt_base) {
                throw std::runtime_error(
                    std::string("yomm2: base class of ")
                    YOMM2_TRACE(+ rt_class.info->name)
                    + " not registered");
            }
        }

        for (auto rt_base : rt_class.direct_bases) {
            rt_base->direct_derived.push_back(&rt_class);
        }
    }
}

void runtime::augment_methods() {
    methods.resize(reg.methods.size());
    auto meth_info_iter = reg.methods.begin(),
        meth_info_iter_end = reg.methods.end();
    auto meth_iter = methods.begin();

    for (; meth_info_iter != meth_info_iter_end; ++meth_info_iter, ++meth_iter) {
        meth_iter->info = *meth_info_iter;
        meth_iter->vp.resize((*meth_info_iter)->vp.size());
        int param_index = 0;
        std::transform(
            (*meth_info_iter)->vp.begin(), (*meth_info_iter)->vp.end(),
            meth_iter->vp.begin(),
            [this, meth_iter, &param_index](const class_info* ci) {
                auto rt_class = class_map[ci];
                if (!rt_class) {
                    YOMM2_TRACE(
                        std::cerr << meth_iter->info->name
                        << " parameter " << (param_index + 1)
                        << ": ");
                    std::cerr << "\nUnregistered class\n";
                    abort();
                }
                rt_arg param = { &*meth_iter,  param_index++ };
                rt_class->vp.push_back(param);
                return class_map[ci];
            });

        meth_iter->specs.resize((*meth_info_iter)->specs.size());
        auto spec_info_iter = (*meth_info_iter)->specs.begin(),
            spec_info_end = (*meth_info_iter)->specs.end();
        auto spec_iter = meth_iter->specs.begin();

        for (; spec_info_iter != spec_info_end; ++spec_info_iter, ++spec_iter) {
            spec_iter->info = *spec_info_iter;
            spec_iter->vp.resize((*spec_info_iter)->vp.size());
            int param_index = 0;
            std::transform(
                (*spec_info_iter)->vp.begin(), (*spec_info_iter)->vp.end(),
                spec_iter->vp.begin(),
                [this, meth_iter, spec_iter, &param_index](const class_info* ci) {
                    auto rt_class = class_map[ci];
                    if (!rt_class) {
                        YOMM2_TRACE(
                            std::cerr << meth_iter->info->name
                            << ": spec " << spec_iter->info->name
                            << ": parameter " << (param_index + 1) << ": ");
                        std::cerr << "\nUnregistered class\n";
                        abort();
                    }
                    ++param_index;
                    return rt_class;
                });
        }
    }
}

void runtime::layer_classes() {

    YOMM2_TRACE(log() << "Layering...\n");

    layered_classes.reserve(classes.size());
    std::list<rt_class*> input;
    std::transform(
        classes.begin(), classes.end(), std::inserter(input, input.begin()),
        [](rt_class& cls) { return &cls; });

    for (int layer = 1; !input.empty(); ++layer) {
        YOMM2_TRACE(const char* sep = "");

        for (auto class_iter = input.begin(); class_iter != input.end(); ) {
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
                layered_classes.push_back(*class_iter);
                (*class_iter)->layer = layer;
                YOMM2_TRACE(log() << sep << (*class_iter)->info->name);
                YOMM2_TRACE(sep = " ");
                class_iter = input.erase(class_iter);
            } else {
                ++class_iter;
            }
        }
        YOMM2_TRACE(log() << "\n");
    }
}

void runtime::calculate_conforming_classes() {
    for (auto class_iter = layered_classes.rbegin();
         class_iter != layered_classes.rend();
         ++class_iter) {
        auto c = *class_iter;
        c->conforming.insert(c);
        for (auto s : c->direct_derived) {
            c->conforming.insert(s);
            std::copy(
                s->conforming.begin(), s->conforming.end(),
                std::inserter(c->conforming, c->conforming.end()));
        }
    }
}

void runtime::allocate_slots() {
    YOMM2_TRACE(log() << "Allocating slots...\n");

    for (auto& c : classes) {
        if (!c.vp.empty()) {
            YOMM2_TRACE(log() << c.info->name << "...\n");
        }

        for (const auto& mp : c.vp) {
            int slot = c.next_slot++;

            YOMM2_TRACE(
                log()
                << "  for " << mp.method->info->name << "#" << mp.param
                << ": "
                << slot << "  also in");

            if (mp.method->slots.size() <= mp.param) {
                mp.method->slots.resize(mp.param + 1);
            }

            mp.method->slots[mp.param] = slot;

            if (c.first_used_slot == -1) {
                c.first_used_slot = slot;
            }

            c.visited = ++class_visit;

            for (auto derived : c.direct_derived) {
                allocate_slot_down(derived, slot);
            }

            YOMM2_TRACE(log() << "\n");
        }
    }

    for (auto& c : classes) {
        c.mtbl.resize(c.next_slot);
    }
}

void runtime::allocate_slot_down(rt_class* cls, int slot) {

    if (cls->visited == class_visit)
        return;

    cls->visited = class_visit;

    YOMM2_TRACE(log() << "\n    " << cls->info->name);

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

void runtime::allocate_slot_up(rt_class* cls, int slot) {

    if (cls->visited == class_visit)
        return;

    cls->visited = class_visit;

    YOMM2_TRACE(log() << "\n    " << cls->info->name);

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
        YOMM2_TRACE(
            log() << "Building dispatch table for " << m.info->name << "\n");

        auto dims = m.vp.size();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            int dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];

                YOMM2_TRACE(log()
                             << indent(1)
                             << "make groups for param #" << dim
                             << ", class " << vp->info->name
                             << "\n");

                for  (auto conforming : vp->conforming) {
                    YOMM2_TRACE(log()
                                 << indent(2)
                                 << "specs applicable to "
                                 << conforming->info->name
                                 << "\n");
                    bitvec mask;
                    mask.resize(m.specs.size());

                    int spec_index = 0;

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->conforming.find(conforming)
                            != spec.vp[dim]->conforming.end()) {
                            YOMM2_TRACE(log()
                                         << indent(3)
                                         << spec.info->name << "\n");
                            mask[spec_index] = 1;
                        }
                        ++spec_index;
                    }

                    dim_group[mask].push_back(conforming);

                    YOMM2_TRACE(
                        log() << "      bit mask = " << mask
                        // << " group = "
                        // << std::distance(dim_group.begin(), dim_group.find(mask))
                        << "\n");

                }

                ++dim;
            }
        }

        {
            int stride = 1;
            m.strides.reserve(dims - 1);

            for (int dim = 1; dim < m.vp.size(); ++dim) {
                stride *= groups[dim - 1].size();
                YOMM2_TRACE(
                    log() << "    stride for dim " <<  dim
                    << " = " << stride << "\n");
                m.strides.push_back(stride);
            }
        }

        m.first_dim = groups[0];

        for (int dim = 0; dim < m.vp.size(); ++dim) {
            YOMM2_TRACE(log()<< indent(1) << "groups for dim " << dim  << ":\n");
            int group_num = 0;
            for (auto& group_pair : groups[dim]) {
                auto& group = group_pair.second;
                for (auto cls : group) {
                    cls->mtbl[m.slots[dim]] = group_num;
                }
#if YOMM2_ENABLE_TRACE
                {
                auto mask = group_pair.first;
                log()
                    << indent(2)
                    << "group " << dim << "/" << group_num << " mask " << mask
                    << " " << outseq(
                        group.begin(), group.end(),
                        [](const rt_class* c) { return c->info->name; })
                    << "\n";
                }
#endif
                ++group_num;
            }
        }

        {
            YOMM2_TRACE(log() << indent(1) << "assign specs\n");
            bitvec all(m.specs.size());
            all = ~all;
            build_dispatch_table(m, dims - 1, groups, all);
        }

        {
            YOMM2_TRACE(log() << indent(1) << "assign next\n");

            std::vector<const rt_spec*> specs;
            std::transform(
                m.specs.begin(), m.specs.end(),
                std::back_inserter(specs),
                [](const rt_spec& spec) { return &spec; });

            for (auto& spec : m.specs) {
                YOMM2_TRACE(log() << indent(2) << spec.info->name  << ":\n");
                std::vector<const rt_spec*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(),
                    std::back_inserter(candidates),
                    [spec](const rt_spec* other) {
                        return is_base(other, &spec);
                    });

#if YOMM2_ENABLE_TRACE
                log()
                    << indent(3)
                    << "select best of:\n";

                for (auto& candidate : candidates) {
                    log() << indent(4)
                          << candidate->info->name << "\n";
                }
#endif
                auto nexts = best(candidates);

                if (nexts.size() == 1) {
                    auto next = nexts.front()->info;
                    YOMM2_TRACE(log() << indent(3) << "-> " << next->name << "\n");
                    *spec.info->next = next->pf;
                } else if (nexts.empty()) {
                    YOMM2_TRACE(log() << indent(3) << "-> none\n");
                    *spec.info->next = m.info->not_implemented;
                } else if (nexts.empty()) {
                    YOMM2_TRACE(log() << indent(3) << "->  ambiguous\n");
                    *spec.info->next = m.info->ambiguous;
                }
            }
        }
    }
}

void runtime::build_dispatch_table(
    rt_method& m, size_t dim, const std::vector<group_map>& groups,
    const bitvec& candidates) {

    int group_index = 0;

    for (auto& group_pair : groups[dim]) {
        auto mask = candidates & group_pair.first;
        auto& group = group_pair.second;

#if YOMM2_ENABLE_TRACE
        log()
            << indent(m.vp.size() - dim + 1)
            << "group " << dim << "/" << group_index
            << " mask " << mask
            << " " << outseq(
                group.begin(), group.end(),
                [](const rt_class* c) { return c->info->name; })
            << "\n";
#endif
        if (dim == 0) {
            std::vector<const rt_spec*> applicable;

            int i = 0;

            for (const auto& spec : m.specs) {
                if (mask[i]) {
                    applicable.push_back(&spec);
                }
                ++i;
            }

#if YOMM2_ENABLE_TRACE
            log()
                << indent(m.vp.size() - dim + 2)
                << "select best of:\n";

            for (auto& app : applicable) {
                log() << indent(m.vp.size() - dim + 3)
                      << app->info->name << "\n";
            }
#endif

            auto specs = best(applicable);

            if (specs.size() > 1) {
                YOMM2_TRACE(log() << indent(m.vp.size() - dim + 2) << "ambiguous\n");
                m.dispatch_table.push_back(m.info->ambiguous);
            } else if (specs.empty()) {
                YOMM2_TRACE(log() << indent(m.vp.size() - dim + 2) << "not implemented\n");
                m.dispatch_table.push_back(m.info->not_implemented);
            } else {
                m.dispatch_table.push_back(specs[0]->info->pf);
#if YOMM2_ENABLE_TRACE
                log()
                    << indent(m.vp.size() - dim + 2)
                    << outseq(
                        specs.begin(), specs.end(),
                        [](const rt_spec* spec) { return spec->info->name; })
                    << ": pf = " << specs[0]->info->pf
                    << "\n";
#endif
            }
        } else {
            build_dispatch_table(m, dim - 1, groups, mask);
        }
        ++group_index;
    }

}

void runtime::find_hash_function(
    const std::vector<rt_class>& classes,
        hash_function& hash,
        metrics_t& metrics) {
    std::vector<const void*> keys;
    auto start_time = std::chrono::steady_clock::now();

    for (auto& cls : classes) {
        std::copy(
            cls.info->ti_ptrs.begin(), cls.info->ti_ptrs.end(),
            std::back_inserter(keys));
    }

    const auto N = keys.size();

    YOMM2_TRACE(log() << "Finding hash factor for " << N << " ti*\n");

    std::default_random_engine rnd(13081963);
    int total_attempts = 0;
    int M = 1;

    for (auto size = N * 5 / 4; size >>= 1; ) {
        ++M;
    }

    std::uniform_int_distribution<std::uintptr_t> uniform_dist;

    for (int pass = 0; pass < 4; ++pass, ++M) {

        hash.shift = 8 * sizeof(std::uintptr_t) - M;
        auto hash_size = 1 << M;

        YOMM2_TRACE(
            log() << indent(1) << "trying with M = " << M
            << ", " << hash_size << " buckets\n");

        bool found = false;
        int attempts = 0;
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
        metrics.hash_search_time = std::chrono::steady_clock::now() - start_time;
        metrics.hash_table_size = hash_size;

        if (found) {
            YOMM2_TRACE(
                log() << indent(1) << "found " << hash.mult
                << " after " << total_attempts << " attempts and "
                << metrics.hash_search_time.count() * 1000 << " msecs\n");
            return;
        }
    }

    throw std::runtime_error("cannot find hash factor");
}

void operator +=(std::vector<word>& words, const std::vector<int>& ints) {
    words.reserve(words.size() + ints.size());
    for (auto i : ints) {
        word w;
        w.i = i;
        words.push_back(w);
    }
}

inline word make_word(int i) {
    word w;
    w.i = i;
    return w;
}

inline word make_word(uintptr_t value) {
    word w;
    w.ul = value;
    return w;
}

inline word make_word(void* pf) {
    word w;
    w.pf = pf;
    return w;
}

void runtime::install_gv() {

    for (int pass = 0; pass != 2; ++pass) {
        dd.gv.resize(0);

        YOMM2_TRACE(
            if (pass) {
                log()
                    << "Initializing global vector at " << dd.gv.data() << "\n"
                    << std::setw(4) << dd.gv.size()
                    << " pointer to control table\n";
            }
        );

        // reserve a work for control table
        dd.gv.emplace_back(make_word(nullptr));

        auto hash_table = dd.gv.data() + 1;
        dd.hash_table = hash_table;

        YOMM2_TRACE(
            if (pass)
                log()
                    << std::setw(4) << dd.gv.size()
                    << " hash table\n");

        dd.gv.resize(dd.gv.size() + metrics.hash_table_size);

        YOMM2_TRACE(
            if (pass)
                log()
                    << std::setw(4) << dd.gv.size()
                    << " control table\n");

        dd.gv.resize(dd.gv.size() + metrics.hash_table_size);

        for (auto& m : methods) {
            YOMM2_TRACE(
                if (pass)
                    log() << std::setw(4) << dd.gv.size()
                          << ' ' << m.info->name << "\n");

            m.info->slots_strides_p->pw = dd.gv.data() + dd.gv.size();

            if (*m.info->hash_factors_placement == typeid(policy::hash_factors_in_vector)) {
                dd.gv.emplace_back(make_word(hash_table));
                dd.gv.emplace_back(make_word(dd.hash.mult));
                dd.gv.emplace_back(make_word(dd.hash.shift));
                // 1-methods with co-located
            } else if (m.info->vp.size() == 1) {
                m.info->slots_strides_p->i = m.slots[0];
                continue;
            }

            // multi-methods only

            auto slot_iter = m.slots.begin();
            auto stride_iter = m.strides.begin();
            dd.gv.emplace_back(make_word(*slot_iter++));

            while (slot_iter != m.slots.end()) {
                dd.gv.emplace_back(make_word(*slot_iter++));
                dd.gv.emplace_back(make_word(*stride_iter++));
            }
            YOMM2_TRACE(
                if (pass)
                    log() << std::setw(4) << dd.gv.size()
                            << ' ' << m.info->name << " dispatch table\n");
            m.gv_dispatch_table = dd.gv.data() + dd.gv.size();
            std::transform(
                m.dispatch_table.begin(), m.dispatch_table.end(),
                std::back_inserter(dd.gv), [](void* pf) {
                    return make_word(pf); });
        }

        auto control_table = hash_table + metrics.hash_table_size;
        dd.gv[0].pw = control_table;

        for (auto& cls : classes) {
            cls.mptr = dd.gv.data() + dd.gv.size() - cls.first_used_slot;
            YOMM2_TRACE(
                if (pass)
                    log() << std::setw(4) << dd.gv.size()
                          << " mtbl for " << cls.info->name
                          << ": " << cls.mptr << "\n");
            if (cls.first_used_slot != -1) {
                std::transform(
                    cls.mtbl.begin() + cls.first_used_slot, cls.mtbl.end(),
                    std::back_inserter(dd.gv), [](int i) {
                        return make_word(i); });
            }
            if (pass) {
                for (auto tid : cls.info->ti_ptrs) {
                    auto index = dd.hash(tid);
                    hash_table[index].pw = cls.mptr;
                    control_table[index].ti = tid;
                }
            }
        }
    }

    YOMM2_TRACE(log() << std::setw(4) << dd.gv.size() << " end\n");
}

void runtime::optimize() {
    YOMM2_TRACE(log() << "Optimizing\n");

    for (auto& m : methods) {
        YOMM2_TRACE(
            log() << "  "
            << m.info->name
            << "\n");
        auto slot = m.slots[0];
        if (m.vp.size() == 1) {
            for (auto cls : m.vp[0]->conforming) {
                auto pf = m.dispatch_table[cls->mptr[slot].i];
                YOMM2_TRACE(
                    log() << "    " << cls->info->name
                    << ".mtbl[" << slot << "] = " << pf << " (function)"
                    << "\n");
                cls->mptr[slot].pf = pf;
            }
        } else {
            for (auto cls : m.vp[0]->conforming) {
                auto pw = m.gv_dispatch_table + cls->mptr[slot].i;
                YOMM2_TRACE(
                    log() << "    " << cls->info->name
                    << ".mtbl[" << slot << "] = gv+" << (pw - dd.hash_table)
                    << "\n");
                cls->mptr[slot].pw = pw;
            }
        }
    }
}

std::vector<const rt_spec*> runtime::best(std::vector<const rt_spec*>& candidates) {
    std::vector<const rt_spec*> best;

    for (auto spec : candidates) {
        const rt_spec* candidate = spec;

        for (auto iter = best.begin(); iter != best.end(); ) {
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

bool runtime::is_more_specific(const rt_spec* a, const rt_spec* b)
{
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*b_iter)->conforming.find(*a_iter) != (*b_iter)->conforming.end()) {
                result = true;
            } else if ((*a_iter)->conforming.find(*b_iter) != (*a_iter)->conforming.end()) {
                return false;
            }
        }
    }

    return result;
}

bool runtime::is_base(const rt_spec* a, const rt_spec* b)
{
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*a_iter)->conforming.find(*b_iter) == (*a_iter)->conforming.end()) {
                return false;
            } else {
                result = true;
            }
        }
    }

    return result;
}

std::ostream* active_log = nullptr;

std::ostream& log() {
    static struct null_streambuf : std::streambuf {
        int_type overflow(int_type c) override {
            return 0;
        }
    } null;

    static std::ostream discard_log(&null);

    if (getenv("YOMM2_ENABLE_TRACE")) {
        log_on(&std::cerr);
    }

    return active_log ? *active_log : discard_log;
}

std::ostream* log_on(std::ostream* os) {
    auto prev = active_log;
    active_log = os;
    return prev;
}

std::ostream* log_off() {
    auto prev = active_log;
    active_log = nullptr;
    return prev;
}

void default_method_call_error_handler(const method_call_error& error) {
#if YOMM2_ENABLE_TRACE
    const char* explanation[] = { "no applicable definition", "ambiguous call" };
    std::cerr << explanation[error.code] << "while calling " << error.method_name << "\n";
#endif
    abort();
}

method_call_error_handler call_error_handler;

void unregistered_class_error(const std::type_info* pt) {
    std::cerr << "\nUnregistered class: " << pt->name() << "\n";
    abort();
}


} // namespace detail

void update_methods() {
    update_methods(detail::registry::get<default_registry>(), detail::dispatch_data::instance<default_registry>::_);
}

method_call_error_handler set_method_call_error_handler(method_call_error_handler handler) {
    auto prev = detail::call_error_handler;
    detail::call_error_handler = handler;
    return prev;
}

} // namespace yomm2
} // namespace yorel
