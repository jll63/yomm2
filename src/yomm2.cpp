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

void update_methods(const registry& reg) {
    //_YOMM2_DEBUG(std::cerr << name() << " += " << name << "\n");
    using std::cerr;

    for (auto cls : reg.classes) {
        cerr << "class " << cls->name;
        const char* sep = ": ";
        for (auto base : cls->direct_bases) {
            cerr << sep << base->name;
            sep = ", ";
        }
        cerr << "\n";
    }

    for (auto meth : reg.methods) {
        cerr << "method " << meth->name << ":\n";
        cerr << "  params:";
        for (auto param : meth->params) {
            cerr << " " << param->name;
        }
        cerr << "\n  specs:\n";
        for (auto spec : meth->specs) {
            cerr << "    " << spec->name << "\n";
        }
    }
}

runtime::runtime(const registry& reg) : reg(reg) {

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
            [this](const class_info* ci) { return class_map[ci]; });

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
        meth_iter->params.resize((*meth_info_iter)->params.size());
        int param_index = 0;
        std::transform(
            (*meth_info_iter)->params.begin(), (*meth_info_iter)->params.end(),
            meth_iter->params.begin(),
            [this, meth_iter, &param_index](const class_info* ci) {
                auto rt_class = class_map[ci];
                rt_arg param = { &*meth_iter,  param_index++ };
                rt_class->method_params.push_back(param);
                return class_map[ci];
            });

        meth_iter->specs.resize((*meth_info_iter)->specs.size());
        auto spec_info_iter = (*meth_info_iter)->specs.begin(),
            spec_info_end = (*meth_info_iter)->specs.end();
        auto spec_iter = meth_iter->specs.begin();

        for (; spec_info_iter != spec_info_end; ++spec_info_iter, ++spec_iter) {
            spec_iter->info = *spec_info_iter;
            spec_iter->params.resize((*spec_info_iter)->params.size());
            std::transform(
                (*spec_info_iter)->params.begin(), (*spec_info_iter)->params.end(),
                spec_iter->params.begin(),
                [this](const class_info* ci) {
                    return class_map[ci];
                });
        }
    }
}

void runtime::layer_classes() {

    _YOMM2_DEBUG(std::cerr << "Layering...");
    _YOMM2_DEBUG(const char* sep = "\n  ");

    std::list<rt_class*> input;
    std::unordered_set<rt_class*> previous_layer;

    layered_classes.reserve(classes.size());

    for (auto& cls : classes) {
        if (cls.direct_bases.empty()) {
            layered_classes.push_back(&cls);
            previous_layer.insert(&cls);
            _YOMM2_DEBUG(std::cerr << sep << cls.info->name);
            _YOMM2_DEBUG(sep = " ");
        } else {
            input.push_back(&cls);
        }
    }

    _YOMM2_DEBUG(sep = "\n  ");

    while (input.size()) {
        std::unordered_set<rt_class*> current_layer;

        for (auto class_iter = input.begin(); class_iter != input.end(); ) {
            auto cls = *class_iter;
            if (std::any_of(
                    cls->direct_bases.begin(), cls->direct_bases.end(),
                    [&previous_layer](rt_class* base) {
                        return previous_layer.find(base) != previous_layer.end();
                    })
                ) {
                current_layer.insert(cls);
                layered_classes.push_back(cls);
                class_iter = input.erase(class_iter);
                _YOMM2_DEBUG(std::cerr << sep << cls->info->name);
                _YOMM2_DEBUG(sep = " ");
            } else {
                ++class_iter;
            }
        }
        previous_layer.swap(current_layer);
        _YOMM2_DEBUG(sep = "\n  ");
    }
    _YOMM2_DEBUG(std::cerr << "\n");
}

void runtime::calculate_conforming_classes() {
    for (auto class_iter = layered_classes.rbegin();
         class_iter != layered_classes.rend();
         ++class_iter) {
        auto c = *class_iter;
        c->confs.insert(c);
        for (auto s : c->direct_derived) {
            c->confs.insert(s);
            std::copy(
                s->confs.begin(), s->confs.end(),
                std::inserter(c->confs, c->confs.end()));
        }
    }
}

void runtime::allocate_slots() {
    _YOMM2_DEBUG(std::cerr << "Allocating slots...\n");

    for (auto& c : classes) {
        if (!c.method_params.empty()) {
            _YOMM2_DEBUG(std::cerr << c.info->name << "...\n");
        }

        for (const auto& mp : c.method_params) {
            int slot = c.next_slot++;

            _YOMM2_DEBUG(
                std::cerr
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

            _YOMM2_DEBUG(std::cerr << "\n");
        }
    }
}

void runtime::allocate_slot_down(rt_class* cls, int slot) {

    if (cls->visited == class_visit)
        return;

    cls->visited = class_visit;

    _YOMM2_DEBUG(std::cerr << "\n    " << cls->info->name);

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

    _YOMM2_DEBUG(std::cerr << "\n    " << cls->info->name);

    assert(slot >= cls->next_slot);

    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_up(d, slot);
    }
}

} // namespace yomm2
} // namespace yorel
