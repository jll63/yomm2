// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/runtime.hpp>

#include <algorithm>
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
    std::unordered_map<const class_info*, rt_class*> class_map;

    classes.resize(reg.classes.size());
    auto class_iter = reg.classes.begin();

    for (auto& rt_class : classes) {
        rt_class.info = *class_iter;
        class_map[*class_iter] = &rt_class;
        std::transform(
            (*class_iter)->bases.begin(),
            (*class_iter)->bases.end(),
            std::back_inserter(rt_class.direct_bases),
            [&class_map](const class_info* ci) { return class_map[ci]; });

        for (auto rt_base : rt_class.direct_bases) {
            rt_base->direct_specs.push_back(&rt_class);
        }

        ++class_iter;
    }
}


} // namespace yomm2
} // namespace yorel
