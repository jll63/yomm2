#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <unordered_map>
#include <vector>

namespace yorel {
namespace yomm2 {

struct rt_method;

struct rt_arg
{
    rt_method* method;
    int param;
};

struct rt_class {
    const class_info* info;
    std::vector<rt_class*> direct_bases;
    std::vector<rt_class*> direct_derived;
    std::unordered_set<rt_class*> confs; // all the classes that conform to this one,
                                         // = the class itself and all its subclasses
    std::vector<rt_arg> method_params;
    int next_slot{0};
    int first_used_slot{-1};

    int visited{0};
};

struct rt_spec
{
    const spec_info* info;
    std::vector<rt_class*> params;
};

struct rt_method {
    const method_info* info;
    std::vector<rt_class*> params;
    std::vector<rt_spec> specs;
    std::vector<int> slots;
    std::vector<int> strides;
    std::vector<void*> dispatch_table;
    //GroupMap firstDim;
};

struct runtime {

    const registry& reg;
    std::unordered_map<const class_info*, rt_class*> class_map;
    std::vector<rt_class> classes;
    std::vector<rt_class*> layered_classes;
    std::vector<rt_method> methods;
    int class_visit{0};

    explicit runtime(const registry& reg);

    void augment_classes();
    void layer_classes();
    void calculate_conforming_classes();
    void augment_methods();
    void allocate_slots();
    void allocate_slot_down(rt_class* cls, int slot);
    void allocate_slot_up(rt_class* cls, int slot);
};

} // namespace yomm2
} // namespace yorel

#endif
