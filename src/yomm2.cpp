// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>

#include <iostream>

namespace yorel {
namespace yomm2 {

void update_methods(const class_registry_t& classes,
                    const method_registry_t& methods) {
    //_YOMM2_DEBUG(std::cerr << name() << " += " << description << "\n");
    using std::cout;

    for (auto cls : classes) {
        //cout << cls->description << ": \n";
    }

    for (auto meth : methods) {
        cout << meth->description << ": \n";
        for (auto spec : meth->specs) {
            cout << "  " << spec->description << "\n";
        }
    }

}

method_registry_t& global_method_registry() {
    static method_registry_t registry;
    return registry;
}

class_registry_t& global_class_registry() {
    static class_registry_t registry;
    return registry;
}

} // namespace yomm2
} // namespace yorel
