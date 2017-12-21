// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#include <algorithm>
#include <cassert>
#include <list>
#include <iostream>

namespace yorel {
namespace yomm2 {

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

void update_methods(const registry& reg) {
    // //_YOMM2_DEBUG(std::log() << name() << " += " << name << "\n");
    // using std::cerr;

    // for (auto cls : reg.classes) {
    //     log() << "class " << cls->name;
    //     const char* sep = ": ";
    //     for (auto base : cls->direct_bases) {
    //         log() << sep << base->name;
    //         sep = ", ";
    //     }
    //     log() << "\n";
    // }

    // for (auto meth : reg.methods) {
    //     log() << "method " << meth->name << ":\n";
    //     log() << "  vp:";
    //     for (auto param : meth->vp) {
    //         log() << " " << param->name;
    //     }
    //     log() << "\n  specs:\n";
    //     for (auto spec : meth->specs) {
    //         log() << "    " << spec->name << "\n";
    //     }
    // }
}

runtime::runtime(const registry& reg) : reg(reg) {
    //_YOMM2_DEBUG(discard_log.)
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
                        + rt_class.info->name
                        + " registered before its base "
                        + ci->name);
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
    // methods.reserve(reg.methods.size());
    // std::transform(
    //     reg.methods.begin(), reg.methods.end(),
    //     std::back_inserter(methods),
    //     [](const method_info* mi) { return { mi }; });
    methods.resize(reg.methods.size());
    auto meth_info_iter = reg.methods.begin(), meth_info_iter_end = reg.methods.end();
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
                rt_class->method_vp.push_back(param);
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


    // std::list<rt_class*> input;
    // std::unordered_set<rt_class*> previous_layer;

    // for (auto& cls : classes) {
    //     if (cls.direct_bases.empty()) {
    //         layered_classes.push_back(&cls);
    //         previous_layer.insert(&cls);
    //         _YOMM2_DEBUG(log() << sep << cls.info->name);
    //         _YOMM2_DEBUG(sep = " ");
    //     } else {
    //         input.push_back(&cls);
    //     }
    // }

    // _YOMM2_DEBUG(sep = "\n  ");

    // while (input.size()) {
    //     std::unordered_set<rt_class*> current_layer;

    //     for (auto class_iter = input.begin(); class_iter != input.end(); ) {
    //         auto cls = *class_iter;
    //         if (std::any_of(
    //                 cls->direct_bases.begin(), cls->direct_bases.end(),
    //                 [&previous_layer](rt_class* base) {
    //                     return previous_layer.find(base) != previous_layer.end();
    //                 })
    //             ) {
    //             current_layer.insert(cls);
    //             layered_classes.push_back(cls);
    //             class_iter = input.erase(class_iter);
    //             _YOMM2_DEBUG(log() << sep << cls->info->name);
    //             _YOMM2_DEBUG(sep = " ");
    //         } else {
    //             ++class_iter;
    //         }
    //     }
    //     previous_layer.swap(current_layer);
    //     _YOMM2_DEBUG(sep = "\n  ");
    // }
    // _YOMM2_DEBUG(log() << "\n");
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
        if (!c.method_vp.empty()) {
            _YOMM2_DEBUG(log() << c.info->name << "...\n");
        }

        for (const auto& mp : c.method_vp) {
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

#if YOMM2_DEBUG
        for (int dim = 0; dim < m.vp.size(); ++dim) {
            log()<< indent(1) << "groups for dim " << dim  << ":\n";
            int group_num = 0;
            for (auto& group_pair : groups[dim]) {
                auto mask = group_pair.first;
                auto& group = group_pair.second;
                log()
                    << indent(2)
                    << "group " << dim << "/" << group_num << " mask " << mask
                    << " " << outseq(
                        group.begin(), group.end(),
                        [](const rt_class* c) { return c->info->name; })
                    << "\n";
                ++group_num;
            }
        }
#endif

        _YOMM2_DEBUG(log() << indent(1) << "assign specs\n");

        bitvec all(m.specs.size());
        all = ~all;
        build_dispatch_table(m, dims - 1, groups, all);
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
                m.dispatch_table.push_back(m.info->ambiguous_call);
            } else if (specs.empty()) {
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

std::vector<const rt_spec*> runtime::best(std::vector<const rt_spec*> candidates) {
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

        if (spec) {
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

std::ostream& runtime::log() {
    return *active_log;
}

std::ostream* runtime::log_on(std::ostream* os) {
    auto prev = active_log;
    active_log = os;
    return prev;
}

std::ostream* runtime::log_off() {
    auto prev = active_log;
    active_log = nullptr;
    return prev;
}

#endif

} // namespace yomm2
} // namespace yorel
