// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#include <algorithm>
#include <list>
#include <iostream>

namespace yorel {
namespace yomm2 {

void update_methods(const registry& reg) {
    //_YOMM2_DEBUG(std::cerr << name() << " += " << description << "\n");
    using std::cerr;

    for (auto cls : reg.classes) {
        cerr << "class " << cls->description;
        const char* sep = ": ";
        for (auto base : cls->bases) {
            cerr << sep << base->description;
            sep = ", ";
        }
        cerr << "\n";
    }

    for (auto meth : reg.methods) {
        cerr << "method " << meth->description << ":\n";
        cerr << "  vargs:";
        for (auto varg : meth->vargs) {
            cerr << " " << varg->description;
        }
        cerr << "\n  specs:\n";
        for (auto spec : meth->specs) {
            cerr << "    " << spec->description << "\n";
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
            (*class_iter)->bases.begin(),
            (*class_iter)->bases.end(),
            std::back_inserter(rt_class.bases),
            [this](const class_info* ci) { return class_map[ci]; });

        for (auto rt_base : rt_class.bases) {
            rt_base->specs.push_back(&rt_class);
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
    auto info_iter = reg.methods.begin(), info_end = reg.methods.end();
    auto meth_iter = methods.begin();

    for (; info_iter != info_end; ++info_iter, ++meth_iter) {
        meth_iter->info = *info_iter;
        std::transform(
            (*info_iter)->vargs.begin(), (*info_iter)->vargs.end(),
            std::back_inserter(meth_iter->vargs),
            [this](const class_info* ci) {
                return class_map[ci];
            });
        meth_iter->specs.resize((*info_iter)->specs.size());
    }
}

void runtime::layer_classes() {

    _YOMM2_DEBUG(std::cerr << "Layering...");
    _YOMM2_DEBUG(const char* sep = "\n  ");

    std::list<rt_class*> input;
    std::unordered_set<rt_class*> previous_layer;

    layered_classes.reserve(classes.size());

    for (auto& cls : classes) {
        if (cls.bases.empty()) {
            layered_classes.push_back(&cls);
            previous_layer.insert(&cls);
            _YOMM2_DEBUG(std::cerr << sep << cls.info->description);
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
                    cls->bases.begin(), cls->bases.end(),
                    [&previous_layer](rt_class* base) {
                        return previous_layer.find(base) != previous_layer.end();
                    })
                ) {
                current_layer.insert(cls);
                layered_classes.push_back(cls);
                class_iter = input.erase(class_iter);
                _YOMM2_DEBUG(std::cerr << sep << cls->info->description);
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
        for (auto s : c->specs) {
            c->confs.insert(s);
            std::copy(
                s->confs.begin(), s->confs.end(),
                std::inserter(c->confs, c->confs.end()));
        }
    }
}

} // namespace yomm2
} // namespace yorel
