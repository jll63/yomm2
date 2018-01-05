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
    std::vector<rt_arg> method_vp;
    int next_slot{0};
    int first_used_slot{-1};
    int layer{0};
    int visited{0};
    std::vector<int> mtbl;
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
    std::vector<const void*> dispatch_table;
    group_map first_dim;
};

struct runtime {

    // input & output
    registry& reg;

    // work
    std::unordered_map<const class_info*, rt_class*> class_map;
    std::vector<rt_class> classes;
    std::vector<rt_class*> layered_classes;
    std::vector<rt_method> methods;
    int class_visit{0};
    std::vector<const void*> hash_table;

    struct metrics_t
    {
        size_t method_table_size, dispatch_table_size, hash_table_size;
        ulong hash_search_attempts;
        std::chrono::duration<double> hash_search_time;
    };

    metrics_t metrics;

    runtime(registry& reg);

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

    static std::vector<const rt_spec*> best(std::vector<const rt_spec*> candidates);
    static bool is_more_specific(const rt_spec* a, const rt_spec* b);

    _YOMM2_DEBUG(std::ostream& log());
    _YOMM2_DEBUG(std::ostream* log_on(std::ostream* os));
    _YOMM2_DEBUG(std::ostream* log_off());
    _YOMM2_DEBUG(std::ostringstream discard_log);
    _YOMM2_DEBUG(std::ostream* active_log);
};

} // namespace yomm2
} // namespace yorel

#endif
