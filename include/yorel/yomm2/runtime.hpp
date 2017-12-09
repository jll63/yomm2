#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <unordered_map>
#include <deque>

namespace yorel {
namespace yomm2 {

struct rt_class {
    const class_info* info;
    std::vector<rt_class*> direct_bases;
    std::vector<rt_class*> direct_specs;
};

struct rt_method {

};

struct runtime {

    const registry& reg;
    std::deque<rt_class> classes;

    explicit runtime(const registry& reg);

    void augment_classes();
};

} // namespace yomm2
} // namespace yorel

#endif
