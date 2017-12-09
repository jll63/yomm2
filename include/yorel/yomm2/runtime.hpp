#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <unordered_map>

namespace yorel {
namespace yomm2 {

struct rt_class {

};

struct rt_method {

};

struct runtime {
    std::unordered_map<const class_info*, const rt_class*> classes;

    void augment_classes() {

    }
};

} // namespace yomm2
} // namespace yorel

#endif
