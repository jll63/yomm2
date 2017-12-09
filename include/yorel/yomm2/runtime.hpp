#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <unordered_map>
#include <vector>

namespace yorel {
namespace yomm2 {

struct rt_class {
    const class_info* info;
    std::vector<rt_class*> bases;
    std::vector<rt_class*> specs;
    std::unordered_set<rt_class*> confs; // all the classes that conform to this one,
                                         // = the class itself and all its subclasses
};

struct rt_method {

};

struct runtime {

    const registry& reg;
    std::vector<rt_class> classes;
    std::vector<rt_class*> layered_classes;

    explicit runtime(const registry& reg);

    void augment_classes();
    void layer_classes();
    void calculate_conforming_classes();
};

} // namespace yomm2
} // namespace yorel

#endif
