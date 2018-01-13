#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <chrono>
#include <map>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#if YOMM2_DEBUG
#include <sstream>
#endif

namespace yorel {
namespace yomm2 {
namespace detail {

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
    std::unordered_set<rt_class*> conforming; // all the classes that conform to this one,
                                         // = the class itself and all its subclasses
    std::vector<rt_arg> vp;
    int next_slot{0};
    int first_used_slot{-1};
    int layer{0};
    int visited{0};
    std::vector<int> mtbl;
    word* mptr;
};

struct rt_spec
{
    const spec_info* info;
    std::vector<rt_class*> vp;
};

using bitvec = boost::dynamic_bitset<>;
using group_map = std::map< bitvec, std::vector<rt_class*> >;

struct rt_method {
    method_info* info;
    std::vector<rt_class*> vp;
    std::vector<rt_spec> specs;
    std::vector<int> slots;
    std::vector<int> strides;
    std::vector<void*> dispatch_table;
    group_map first_dim;
    const word* gv_dispatch_table{nullptr};
};

struct runtime {

    const registry& reg;
    dispatch_data& dd;

    // work
    std::unordered_map<const class_info*, rt_class*> class_map;
    std::vector<rt_class> classes;
    std::vector<rt_class*> layered_classes;
    std::vector<rt_method> methods;
    int class_visit{0};

    struct metrics_t
    {
        size_t method_table_size, dispatch_table_size, hash_table_size;
        ulong hash_search_attempts;
        std::chrono::duration<double> hash_search_time;
    };

    metrics_t metrics;

    runtime(const registry& reg, struct dispatch_data& dd);

    void update();

    void augment_classes();
    void layer_classes();
    void calculate_conforming_classes();
    void augment_methods();
    void allocate_slots();
    void allocate_slot_down(rt_class* cls, int slot);
    void allocate_slot_up(rt_class* cls, int slot);
    void build_dispatch_tables();
    void build_dispatch_table(
        rt_method& m, size_t dim, const std::vector<group_map>& groups,
        const bitvec& candidates);
    void find_hash_factor();
    void install_gv();
    void optimize();

    static std::vector<const rt_spec*> best(std::vector<const rt_spec*>& candidates);
    static bool is_more_specific(const rt_spec* a, const rt_spec* b);
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
