// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_RUNTIME_INCLUDED
#define YOREL_YOMM2_RUNTIME_INCLUDED

#include <yorel/yomm2.hpp>

#include <chrono>
#include <deque>
#include <map>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

struct rt_method;

struct rt_arg {
    rt_method* method;
    size_t param;
};

struct rt_class {
    bool is_abstract{false};
    std::vector<const std::type_info*> ti_ptrs;
    std::vector<rt_class*> transitive_bases;
    std::vector<rt_class*> direct_bases;
    std::vector<rt_class*> direct_derived;
    std::unordered_set<rt_class*> covariant_classes;
    std::vector<rt_arg> used_by_vp;
    int next_slot{0};
    int first_used_slot{-1};
    int layer{0};
    size_t mark{0};   // to detect cycles, aka temporary mark
    size_t weight{0}; // number of proper direct or indirect bases
    std::vector<int> mtbl;
    detail::word* mptr;

    auto name() const {
        return ti_ptrs[0]->name();
    }
};

struct rt_spec {
    const definition_info* info;
    std::vector<rt_class*> vp;
};

using bitvec = boost::dynamic_bitset<>;

struct group {
    std::vector<rt_class*> classes;
    bool has_concrete_classes{false};
};

using group_map = std::map<bitvec, group>;

struct dispatch_stats_t {
    size_t cells{0};
    size_t concrete_cells{0};
    size_t not_implemented{0};
    size_t concrete_not_implemented{0};
    size_t ambiguous{0};
    size_t concrete_ambiguous{0};

    void accumulate(const dispatch_stats_t& other);
};

struct rt_method {
    method_info* info;
    std::vector<rt_class*> vp;
    std::vector<rt_spec> specs;
    std::vector<int> slots;
    std::vector<int> strides;
    std::vector<void*> dispatch_table;
    const detail::word* gv_dispatch_table{nullptr};
    auto arity() const {
        return vp.size();
    }
    dispatch_stats_t stats;
};

struct yOMM2_API runtime {

    catalog& cat;
    context& ctx;

    // work
    std::unordered_map<std::type_index, rt_class*> class_map;
    std::deque<rt_class> classes;
    std::vector<rt_method> methods;
    size_t class_visit{0};
    mutable detail::trace_type<detail::TRACE_RUNTIME> trace;

    struct metrics_t : dispatch_stats_t {
        size_t method_table_size, dispatch_table_size, hash_table_size;
        size_t hash_search_attempts;
        std::chrono::duration<double> hash_search_time;
    } metrics;

    runtime(catalog& cat, struct context& ctx);

    void update();

    void augment_classes();
    void calculate_conforming_classes(rt_class& cls);
    void augment_methods();
    std::vector<rt_class*> layer_classes();
    void allocate_slots();
    void allocate_slot_down(rt_class* cls, size_t slot);
    void allocate_slot_up(rt_class* cls, size_t slot);
    void build_dispatch_tables();
    void build_dispatch_table(
        rt_method& m, size_t dim, std::vector<group_map>::const_iterator group,
        const bitvec& candidates, bool concrete);
    void install_gv();
    void optimize();
    void find_hash_function(
        const std::deque<rt_class>& classes, hash_function& hash,
        metrics_t& metrics);
    void print(const dispatch_stats_t& stats) const;
    static std::vector<const rt_spec*>
    best(std::vector<const rt_spec*>& candidates);
    static bool is_more_specific(const rt_spec* a, const rt_spec* b);
    static bool is_base(const rt_spec* a, const rt_spec* b);
};

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
