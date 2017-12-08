// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>

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

} // namespace yomm2
} // namespace yorel
