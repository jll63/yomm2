// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>

namespace yorel {
namespace yomm2 {

using namespace details;

#if YOMM2_DEBUG

struct indent {
    indent(int n) : n(n) {
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

void update_methods() {
    update_methods(registry::get<void>(), dispatch_data::instance<void>);
}

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
    find_hash_factor();
    install_gv();
    optimize();
}

runtime::runtime(const registry& reg, struct dispatch_data& dd) : reg(reg), dd(dd) {
}

void runtime::augment_classes() {
    classes.resize(reg.classes.size());
    auto class_iter = reg.classes.begin();

    for (auto& rt_class : classes) {
        rt_class.info = *class_iter;
        class_map[*class_iter] = &rt_class;
        std::transform(
            (*class_iter)->direct_bases.begin(),
            (*class_iter)->direct_bases.end(),
            std::back_inserter(rt_class.direct_bases),
            [this, rt_class](const class_info* ci) {
                auto base = class_map[ci];
                if (!base) {
                    throw std::runtime_error(
                        std::string("yomm2: derived class ")
                        _YOMM2_DEBUG(+ rt_class.info->name)
                        + " registered before its base "
                        _YOMM2_DEBUG(+ ci->name));
                }
                return base;
            });

        for (auto rt_base : rt_class.direct_bases) {
            rt_base->direct_derived.push_back(&rt_class);
        }

        ++class_iter;
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
            std::transform(
                (*spec_info_iter)->vp.begin(), (*spec_info_iter)->vp.end(),
                spec_iter->vp.begin(),
                [this](const class_info* ci) {
                    return class_map[ci];
                });
        }
    }
}

void runtime::layer_classes() {

    _YOMM2_DEBUG(log() << "Layering...\n");

    layered_classes.reserve(classes.size());
    std::list<rt_class*> input;
    std::transform(
        classes.begin(), classes.end(), std::inserter(input, input.begin()),
        [](rt_class& cls) { return &cls; });

    for (int layer = 1; !input.empty(); ++layer) {
        _YOMM2_DEBUG(const char* sep = "");

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
                _YOMM2_DEBUG(log() << sep << (*class_iter)->info->name);
                _YOMM2_DEBUG(sep = " ");
                class_iter = input.erase(class_iter);
            } else {
                ++class_iter;
            }
        }
        _YOMM2_DEBUG(log() << "\n");
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
    _YOMM2_DEBUG(log() << "Allocating slots...\n");

    for (auto& c : classes) {
        if (!c.vp.empty()) {
            _YOMM2_DEBUG(log() << c.info->name << "...\n");
        }

        for (const auto& mp : c.vp) {
            int slot = c.next_slot++;

            _YOMM2_DEBUG(
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

            _YOMM2_DEBUG(log() << "\n");
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

    _YOMM2_DEBUG(log() << "\n    " << cls->info->name);

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

    _YOMM2_DEBUG(log() << "\n    " << cls->info->name);

    assert(slot >= cls->next_slot);

    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_up(d, slot);
    }
}

void runtime::build_dispatch_tables() {
    for (auto& m : methods) {
        _YOMM2_DEBUG(
            log() << "Building dispatch table for " << m.info->name << "\n");

        auto dims = m.vp.size();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            int dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];

                _YOMM2_DEBUG(log()
                             << indent(1)
                             << "make groups for param #" << dim
                             << ", class " << vp->info->name
                             << "\n");

                for  (auto conforming : vp->conforming) {
                    _YOMM2_DEBUG(log()
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
                            _YOMM2_DEBUG(log()
                                         << indent(3)
                                         << spec.info->name << "\n");
                            mask[spec_index] = 1;
                        }
                        ++spec_index;
                    }

                    dim_group[mask].push_back(conforming);

                    _YOMM2_DEBUG(
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
                _YOMM2_DEBUG(
                    log() << "    stride for dim " <<  dim
                    << " = " << stride << "\n");
                m.strides.push_back(stride);
            }
        }

        m.first_dim = groups[0];

        for (int dim = 0; dim < m.vp.size(); ++dim) {
            _YOMM2_DEBUG(log()<< indent(1) << "groups for dim " << dim  << ":\n");
            int group_num = 0;
            for (auto& group_pair : groups[dim]) {
                auto& group = group_pair.second;
                for (auto cls : group) {
                    cls->mtbl[m.slots[dim]] = group_num;
                }
#if YOMM2_DEBUG
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
            _YOMM2_DEBUG(log() << indent(1) << "assign specs\n");
            bitvec all(m.specs.size());
            all = ~all;
            build_dispatch_table(m, dims - 1, groups, all);
        }

        {
            _YOMM2_DEBUG(log() << indent(1) << "assign next\n");
            std::vector<const rt_spec*> specs;
            std::transform(
                m.specs.begin(), m.specs.end(),
                std::back_inserter(specs),
                [](const rt_spec& spec) { return &spec; });

            for (auto& spec : m.specs) {
                _YOMM2_DEBUG(log() << indent(2) << spec.info->name  << ":\n");
                std::vector<const rt_spec*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(),
                    std::back_inserter(candidates),
                    [spec](const rt_spec* other) { return is_more_specific(&spec, other); });

#if YOMM2_DEBUG
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
                    _YOMM2_DEBUG(log() << indent(3) << "-> " << next->name << "\n");
                    *spec.info->next = next->pf;
                } else if (nexts.empty()) {
                    _YOMM2_DEBUG(log() << indent(3) << "-> none\n");
                } else if (nexts.empty()) {
                    _YOMM2_DEBUG(log() << indent(3) << "->  ambiguous\n");

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

#if YOMM2_DEBUG
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

#if YOMM2_DEBUG
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
                _YOMM2_DEBUG(log() << indent(m.vp.size() - dim + 2) << "ambiguous\n");
                m.dispatch_table.push_back(m.info->ambiguous_call);
            } else if (specs.empty()) {
                _YOMM2_DEBUG(log() << indent(m.vp.size() - dim + 2) << "not implemented\n");
                m.dispatch_table.push_back(m.info->not_implemented);
            } else {
                m.dispatch_table.push_back(specs[0]->info->pf);
#if YOMM2_DEBUG
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

void runtime::find_hash_factor() {
    std::vector<const void*> keys;
    auto start_time = std::chrono::steady_clock::now();

    for (auto& cls : classes) {
        std::copy(
            cls.info->ti_ptrs.begin(), cls.info->ti_ptrs.end(),
            std::back_inserter(keys));
    }

    const auto N = keys.size();

    _YOMM2_DEBUG(log() << "Finding hash factor for " << N << " ti*\n");

    std::default_random_engine rnd(13081963);
    int total_attempts = 0;
    int M = 0;
    std::uniform_int_distribution<std::uintptr_t> uniform_dist;

    for (int room = 2; room <= 6; ++room) {
        M = 1;

        while ((1 << M) < room * N / 2) {
            ++M;
        }

        dd.hash_shift = 64 - M;
        auto hash_size = 1 << M;
        dd.gv.resize(hash_size);

        _YOMM2_DEBUG(
            log() << indent(1) << "trying with M = " << M
            << ", " << hash_size << " buckets\n");

        bool found = false;
        int attempts = 0;
        std::vector<int> buckets(hash_size);

        while (!found && attempts < 100000) {
            ++attempts;
            ++total_attempts;
            found = true;
            dd.hash_mult = uniform_dist(rnd) | 1;

            for (auto key : keys) {
                auto h = hash(dd, key);
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
            _YOMM2_DEBUG(
                log() << indent(1) << "found " << dd.hash_mult
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

inline word make_word(void* pf) {
    word w;
    w.pf = pf;
    return w;
}

void runtime::install_gv() {

    _YOMM2_DEBUG(log() << "Initializing global vector\n");
    _YOMM2_DEBUG(log() << "   0 hash table\n");

    for (int pass = 0; pass != 2; ++pass) {
        dd.gv.resize(metrics.hash_table_size);

        for (auto& m : methods) {
            *m.info->slots_strides_p = dd.gv.data() + dd.gv.size();
            auto slot_iter = m.slots.begin();
            _YOMM2_DEBUG(
                if (pass)
                    log() << std::setw(4) << dd.gv.size()
                          << ' ' << m.info->name << " slots and strides\n");
            auto stride_iter = m.strides.begin();
            dd.gv.emplace_back(make_word(*slot_iter++));

            while (slot_iter != m.slots.end()) {
                dd.gv.emplace_back(make_word(*slot_iter++));
                dd.gv.emplace_back(make_word(*stride_iter++));
            }

            if (m.info->vp.size() > 1) {
                _YOMM2_DEBUG(
                    if (pass)
                        log() << std::setw(4) << dd.gv.size()
                              << ' ' << m.info->name << " dispatch table\n");
                m.gv_dispatch_table = dd.gv.data() + dd.gv.size();
                std::transform(
                    m.dispatch_table.begin(), m.dispatch_table.end(),
                    std::back_inserter(dd.gv), [](void* pf) {
                        return make_word(pf); });
            }
        }

        for (auto& cls : classes) {
            _YOMM2_DEBUG(
                if (pass)
                    log() << std::setw(4) << dd.gv.size()
                          << " mtbl for " << cls.info->name << "\n");
            cls.mptr = dd.gv.data() + dd.gv.size();
            std::transform(
                cls.mtbl.begin(), cls.mtbl.end(),
                std::back_inserter(dd.gv), [](int i) {
                    return make_word(i); });

            for (auto tid : cls.info->ti_ptrs) {
                dd.gv[hash(dd, tid)].pw = cls.mptr;

            }
        }
    }

    _YOMM2_DEBUG(log() << std::setw(4) << dd.gv.size() << " end\n");
}

void runtime::optimize() {
    _YOMM2_DEBUG(log() << "Optimizing\n");

    for (auto& m : methods) {
        _YOMM2_DEBUG(
            log() << "  "
            << m.info->name
            << "\n");
        auto slot = m.slots[0];
        if (m.vp.size() == 1) {
            for (auto cls : m.vp[0]->conforming) {
                auto pf = m.dispatch_table[cls->mptr[slot].i];
                _YOMM2_DEBUG(
                    log() << "    " << cls->info->name
                    << ".mtbl[" << slot << "] = " << pf << " (function)"
                    << "\n");
                cls->mptr[slot].pf = pf;
            }
        } else {
            for (auto cls : m.vp[0]->conforming) {
                auto pw = m.gv_dispatch_table + cls->mptr[slot].i;
                _YOMM2_DEBUG(
                    log() << "    " << cls->info->name
                    << ".mtbl[" << slot << "] = gv+" << (pw - dd.gv.data())
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

#if YOMM2_DEBUG

std::ostream* active_log = nullptr;

namespace details {
std::ostream& log() {
    static std::ostringstream discard_log;
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

} // namespace details
#endif

} // namespace yomm2
} // namespace yorel
