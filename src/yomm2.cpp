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
    using std::cout;

    for (auto cls : reg.classes) {
        cout << cls.second->description << ": \n";
    }

    for (auto meth : reg.methods) {
        cout << meth->description << ": \n";
        for (auto spec : meth->specs) {
            cout << "  " << spec->description << "\n";
        }
    }

}

} // namespace yomm2
} // namespace yorel
