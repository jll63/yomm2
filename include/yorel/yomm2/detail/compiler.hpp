// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_DETAIL_COMPILER_INCLUDED
#define YOREL_YOMM2_DETAIL_COMPILER_INCLUDED

#include <algorithm> // for max, transform, copy
#include <cassert>   // for assert
#include <cstdint>   // for uintptr_t
#include <cstdio>
#include <cstdlib> // for abort, getenv
#include <deque>
#include <list>          // for list, _List_iterator
#include <map>           // for map
#include <memory>        // for allocator_traits<...
#include <numeric>       // for accumulate
#include <stdexcept>     // for runtime_error
#include <string>        // for char_traits, allo...
#include <unordered_map> // for _Node_iterator
#include <unordered_set> // for unordered_set<>::...
#include <utility>       // for pair
#include <vector>        // for vector, vector<>:...

#include <boost/dynamic_bitset.hpp>

#include <yorel/yomm2/core.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

inline void merge_into(boost::dynamic_bitset<>& a, boost::dynamic_bitset<>& b) {
    if (b.size() < a.size()) {
        b.resize(a.size());
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i]) {
            b[i] = true;
        }
    }
}

inline void set_bit(boost::dynamic_bitset<>& mask, std::size_t bit) {
    if (bit >= mask.size()) {
        mask.resize(bit + 1);
    }

    mask[bit] = true;
}

struct generic_compiler {

    struct method;

    struct parameter {
        struct method* method;
        std::size_t param;
    };

    struct vtbl_entry {
        std::size_t method_index, vp_index, group_index;
    };

    struct class_ {
        bool is_abstract = false;
        std::vector<type_id> type_ids;
        std::vector<class_*> transitive_bases;
        std::vector<class_*> direct_bases;
        std::vector<class_*> direct_derived;
        std::unordered_set<class_*> covariant_classes;
        std::vector<parameter> used_by_vp;
        boost::dynamic_bitset<> used_slots;
        boost::dynamic_bitset<> reserved_slots;
        std::size_t first_slot = 0;
        std::size_t mark = 0;   // temporary mark to detect cycles
        std::size_t weight = 0; // number of proper direct or indirect bases
        std::vector<vtbl_entry> vtbl;
        std::uintptr_t** static_vptr;

        const std::uintptr_t* vptr() const {
            return *static_vptr;
        }

        const std::uintptr_t* const* indirect_vptr() const {
            return static_vptr;
        }

        auto type_id_begin() const {
            return type_ids.begin();
        }

        auto type_id_end() const {
            return type_ids.end();
        }
    };

    struct definition {
        const detail::definition_info* info;
        std::vector<class_*> vp;
        std::uintptr_t pf;
        std::size_t method_index, spec_index;
    };

    using bitvec = boost::dynamic_bitset<>;

    struct group {
        std::vector<class_*> classes;
        bool has_concrete_classes{false};
    };

    using group_map = std::map<bitvec, group>;

    static void
    accumulate(const update_method_report& partial, update_report& total);

    struct method {
        detail::method_info* info;
        std::vector<class_*> vp;
        std::vector<definition> specs;
        std::vector<std::size_t> slots;
        std::vector<std::size_t> strides;
        std::vector<const definition*> dispatch_table;
        // following two are dummies, when converting to a function pointer, we will
        // get the corresponding pointer from method_info
        definition not_implemented;
        definition ambiguous;
        const std::uintptr_t* gv_dispatch_table{nullptr};
        auto arity() const {
            return vp.size();
        }
        update_method_report report;
    };

    std::deque<class_> classes;
    std::vector<method> methods;
    std::size_t class_mark = 0;
    bool compilation_done = false;
};

template<class Policy>
trace_type<Policy>&
operator<<(trace_type<Policy>& trace, const generic_compiler::class_& cls) {
    if constexpr (Policy::template has_facet<policy::trace_output>) {
        trace << type_name(cls.type_ids[0]);
    }

    return trace;
}

template<class Policy, template<typename...> typename Container, typename... T>
trace_type<Policy>& operator<<(
    trace_type<Policy>& trace,
    Container<generic_compiler::class_*, T...>& classes) {
    if constexpr (Policy::template has_facet<policy::trace_output>) {
        trace << "(";
        const char* sep = "";
        for (auto cls : classes) {
            trace << sep << *cls;
            sep = ", ";
        }

        trace << ")";
    }

    return trace;
}

struct spec_name {
    spec_name(
        const generic_compiler::method& method,
        const generic_compiler::definition* def)
        : method(method), def(def) {
    }
    const generic_compiler::method& method;
    const generic_compiler::definition* def;
};

template<class Policy>
trace_type<Policy>& operator<<(trace_type<Policy>& trace, const spec_name& sn) {
    if (sn.def == &sn.method.ambiguous) {
        trace << "ambiguous";
    } else if (sn.def == &sn.method.not_implemented) {
        trace << "not implemented";
    } else {
        trace << type_name(sn.def->info->type);
    }

    return trace;
}

template<class Policy>
struct compiler : generic_compiler {
    using policy_type = Policy;
    using type_index_type = decltype(Policy::type_index(0));

    typename aggregate_reports<
        types<update_report>, typename Policy::facets>::type report;

    std::unordered_map<type_index_type, class_*> class_map;

    compiler();

    auto compile();
    auto update();
    void install_global_tables();

    void resolve_static_type_ids();
    void augment_classes();
    void calculate_covariant_classes(class_& cls);
    void augment_methods();
    void assign_slots();
    void assign_tree_slots(class_& cls, std::size_t base_slot);
    void assign_lattice_slots(class_& cls);
    void build_dispatch_tables();
    void build_dispatch_table(
        method& m, std::size_t dim,
        std::vector<group_map>::const_iterator group, const bitvec& candidates,
        bool concrete);
    void install_gv();
    void print(const update_method_report& report) const;
    static std::vector<const definition*>
    best(std::vector<const definition*>& candidates);
    static bool is_more_specific(const definition* a, const definition* b);
    static bool is_base(const definition* a, const definition* b);

    static type_id static_type(type_id type) {
        if constexpr (std::is_base_of_v<
                          policy::deferred_static_rtti, policy::rtti>) {
            return reinterpret_cast<type_id (*)()>(type)();
        } else {
            return type;
        }
    }

    mutable trace_type<Policy> trace;
    static constexpr bool trace_enabled =
        Policy::template has_facet<policy::trace_output>;
    using indent = typename trace_type<Policy>::indent;
};

compiler() -> compiler<default_policy>;

template<class Policy>
void compiler<Policy>::install_global_tables() {
    if (!compilation_done) {
        abort();
    }

    install_gv();

    print(report);
    ++trace << "Finished\n";
}

template<class Policy>
auto compiler<Policy>::compile() {
    resolve_static_type_ids();
    augment_classes();
    augment_methods();
    assign_slots();
    build_dispatch_tables();

    compilation_done = true;

    return report;
}

template<class Policy>
auto compiler<Policy>::update() {
    compile();
    install_global_tables();

    return *this;
}

template<class Policy>
compiler<Policy>::compiler() {
}

template<class Policy>
void compiler<Policy>::resolve_static_type_ids() {
    auto resolve = [](type_id* p) {
        auto pf = reinterpret_cast<type_id (*)()>(*p);
        *p = pf();
    };

    if constexpr (std::is_base_of_v<policy::deferred_static_rtti, Policy>) {
        if (!Policy::classes.empty())
            for (auto& ci : Policy::classes) {
                resolve(&ci.type);

                if (*ci.last_base == 0) {
                    for (auto& ti : range{ci.first_base, ci.last_base}) {
                        resolve(&ti);
                    }

                    *ci.last_base = 1;
                }
            }

        if (!Policy::methods.empty())
            for (auto& method : Policy::methods) {
                for (auto& ti : range{method.vp_begin, method.vp_end}) {
                    if (*method.vp_end == 0) {
                        resolve(&ti);
                        *method.vp_end = 1;
                    }

                    if (!method.specs.empty())
                        for (auto& definition : method.specs) {
                            if (*definition.vp_end == 0) {
                                for (auto& ti : range{
                                         definition.vp_begin,
                                         definition.vp_end}) {
                                    resolve(&ti);
                                }

                                *definition.vp_end = 1;
                            }
                        }
                }
            }
    }
}

template<class Policy>
void compiler<Policy>::augment_classes() {
    using namespace detail;

    // scope
    {
        ++trace << "Static class info:\n";

        // The standard does not guarantee that there is exactly one
        // type_info object per class. However, it guarantees that the
        // type_index for a class has a unique value.
        for (auto& cr : Policy::classes) {
            {
                indent _(trace);
                ++trace << type_name(cr.type) << ": "
                        << range{cr.first_base, cr.last_base} << "\n";
            }

            auto& rtc = class_map[Policy::type_index(cr.type)];

            if (rtc == nullptr) {
                rtc = &classes.emplace_back();
                rtc->is_abstract = cr.is_abstract;
                rtc->static_vptr = cr.static_vptr;
            }

            // In the unlikely case that a class does have more than one
            // associated  ti*, collect them in a vector. We don't use an
            // unordered_set because, again, this situation is highly
            // unlikely, and, were it to occur, the number of distinct ti*s
            // would probably be small.
            if (std::find(
                    rtc->type_ids.begin(), rtc->type_ids.end(), cr.type) ==
                rtc->type_ids.end()) {
                rtc->type_ids.push_back(cr.type);
            }
        }
    }

    // All known classes now have exactly one associated class_* in the
    // map. Collect the bases.

    for (auto& cr : Policy::classes) {
        auto& rtc = class_map[Policy::type_index(cr.type)];

        for (auto base_iter = cr.first_base; base_iter != cr.last_base;
             ++base_iter) {
            auto rtb = class_map[Policy::type_index(*base_iter)];

            if (!rtb) {
                unknown_class_error error;
                error.type = *base_iter;

                if constexpr (Policy::template has_facet<
                                  policy::error_handler>) {
                    Policy::error(error_type(error));
                }

                abort();
            }

            if (rtc != rtb) {
                // At compile time we collected the class as its own
                // improper base, as per std::is_base_of. Eliminate that.
                rtc->transitive_bases.push_back(rtb);
            }
        }
    }

    // At this point bases may contain duplicates, and also indirect
    // bases. Clean that up.

    std::size_t mark = ++class_mark;

    for (auto& rtc : classes) {
        decltype(rtc.transitive_bases) bases;
        mark = ++class_mark;

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark != mark) {
                bases.push_back(rtb);
                rtb->mark = mark;
            }
        }

        // Record the "weight" of the class, i.e. the total number of direct
        // and indirect proper bases it has.
        rtc.weight = bases.size();
        rtc.transitive_bases.swap(bases);
    }

    for (auto& rtc : classes) {
        // Sort base classes by weight. This ensures that a base class is
        // never preceded by one if its own bases classes.
        std::sort(
            rtc.transitive_bases.begin(), rtc.transitive_bases.end(),
            [](auto a, auto b) { return a->weight > b->weight; });
        mark = ++class_mark;

        // Collect the direct base classes. The first base is certainly a
        // direct one. Remove *its* bases from the candidates, by marking
        // them. Continue with the next base that is not marked. It is the
        // next direct base. And so on...

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark == mark) {
                continue;
            }

            rtc.direct_bases.push_back(rtb);

            for (auto rtbb : rtb->transitive_bases) {
                rtbb->mark = mark;
            }
        }
    }

    for (auto& rtc : classes) {
        for (auto rtb : rtc.direct_bases) {
            rtb->direct_derived.push_back(&rtc);
        }
    }

    for (auto& rtc : classes) {
        calculate_covariant_classes(rtc);
    }

    if constexpr (trace_enabled) {
        ++trace << "Inheritance lattice:\n";

        for (auto& rtc : classes) {
            indent _(trace);
            ++trace << rtc << "\n";

            {
                indent _(trace);
                ++trace << "bases:      " << rtc.direct_bases << "\n";
                ++trace << "derived:    " << rtc.direct_derived << "\n";
                ++trace << "covariant: " << rtc.covariant_classes << "\n";
            }
        }
    }
}

template<class Policy>
void compiler<Policy>::calculate_covariant_classes(class_& cls) {
    if (!cls.covariant_classes.empty()) {
        return;
    }

    cls.covariant_classes.insert(&cls);

    for (auto derived : cls.direct_derived) {
        if (derived->covariant_classes.empty()) {
            calculate_covariant_classes(*derived);
        }

        std::copy(
            derived->covariant_classes.begin(),
            derived->covariant_classes.end(),
            std::inserter(cls.covariant_classes, cls.covariant_classes.end()));
    }
}

template<class Policy>
void compiler<Policy>::augment_methods() {
    using namespace policy;
    using namespace detail;

    methods.resize(Policy::methods.size());

    ++trace << "Methods:\n";
    indent _(trace);

    auto meth_iter = methods.begin();

    for (auto& meth_info : Policy::methods) {
        ++trace << meth_info.name << " "
                << range{meth_info.vp_begin, meth_info.vp_end} << "\n";

        indent _(trace);

        meth_iter->info = &meth_info;
        meth_iter->vp.reserve(meth_info.arity());
        meth_iter->slots.resize(meth_info.arity());
        std::size_t param_index = 0;

        for (auto ti : range{meth_info.vp_begin, meth_info.vp_end}) {
            auto class_ = class_map[Policy::type_index(ti)];
            if (!class_) {
                ++trace << "unkown class " << ti << "(" << type_name(ti)
                        << ") for parameter #" << (param_index + 1) << "\n";
                unknown_class_error error;
                error.type = ti;

                if constexpr (has_facet<Policy, error_handler>) {
                    Policy::error(error_type(error));
                }

                abort();
            }
            parameter param = {&*meth_iter, param_index++};
            meth_iter->vp.push_back(class_);
        }

        // initialize the function pointer in the dummy specs
        const auto method_index = meth_iter - methods.begin();
        meth_iter->ambiguous.pf =
            reinterpret_cast<uintptr_t>(meth_iter->info->ambiguous);
        meth_iter->ambiguous.method_index = method_index;
        auto spec_size = meth_info.specs.size();
        meth_iter->ambiguous.spec_index = spec_size;
        meth_iter->not_implemented.pf =
            reinterpret_cast<uintptr_t>(meth_iter->info->not_implemented);
        meth_iter->not_implemented.method_index = method_index;
        meth_iter->not_implemented.spec_index = spec_size + 1;

        meth_iter->specs.resize(spec_size);
        auto spec_iter = meth_iter->specs.begin();

        for (auto& definition_info : meth_info.specs) {
            spec_iter->method_index = meth_iter - methods.begin();
            spec_iter->spec_index = spec_iter - meth_iter->specs.begin();

            ++trace << type_name(definition_info.type) << " ("
                    << definition_info.pf << ")\n";
            spec_iter->info = &definition_info;
            spec_iter->vp.reserve(meth_info.arity());
            std::size_t param_index = 0;

            for (auto type :
                 range{definition_info.vp_begin, definition_info.vp_end}) {
                indent _(trace);
                auto class_ = class_map[Policy::type_index(type)];
                if (!class_) {
                    ++trace << "error for *virtual* parameter #"
                            << (param_index + 1) << "\n";
                    unknown_class_error error;
                    error.type = type;

                    if constexpr (has_facet<Policy, error_handler>) {
                        Policy::error(error_type(error));
                    }

                    abort();
                }
                spec_iter->pf =
                    reinterpret_cast<uintptr_t>(spec_iter->info->pf);
                spec_iter->vp.push_back(class_);
            }

            ++spec_iter;
        }

        ++meth_iter;
    }

    for (auto& method : methods) {
        std::size_t param_index = 0;

        for (auto vp : method.vp) {
            vp->used_by_vp.push_back({&method, param_index++});
        }
    }
}

template<class Policy>
void compiler<Policy>::assign_slots() {
    ++trace << "Allocating slots...\n";

    {
        indent _(trace);

        ++class_mark;

        for (auto& cls : classes) {
            if (cls.direct_bases.size() == 0) {
                if (std::find_if(
                        cls.covariant_classes.begin(),
                        cls.covariant_classes.end(), [](auto cls) {
                            return cls->direct_bases.size() > 1;
                        }) == cls.covariant_classes.end()) {
                    assign_tree_slots(cls, 0);
                } else {
                    assign_lattice_slots(cls);
                }
            }
        }
    }

    ++trace << "Allocating MI v-tables...\n";

    {
        indent _(trace);

        for (auto& cls : classes) {
            if (cls.used_slots.empty()) {
                // not involved in multiple inheritance
                continue;
            }

            auto first_slot = cls.used_slots.find_first();
            cls.first_slot =
                first_slot == boost::dynamic_bitset<>::npos ? 0 : first_slot;
            cls.vtbl.resize(cls.used_slots.size() - cls.first_slot);
            ++trace << cls << " vtbl: " << cls.first_slot << "-"
                    << cls.used_slots.size() << " slots " << cls.used_slots
                    << "\n";
        }
    }
}

template<class Policy>
void compiler<Policy>::assign_tree_slots(class_& cls, std::size_t base_slot) {
    auto next_slot = base_slot;

    for (const auto& mp : cls.used_by_vp) {
        mp.method->slots[mp.param] = next_slot++;
    }

    cls.first_slot = 0;
    cls.vtbl.resize(next_slot);

    for (auto pd : cls.direct_derived) {
        assign_tree_slots(*pd, next_slot);
    }
}

template<class Policy>
void compiler<Policy>::assign_lattice_slots(class_& cls) {
    if (cls.mark == class_mark) {
        return;
    }

    cls.mark = class_mark;

    if (!cls.used_by_vp.empty()) {
        for (const auto& mp : cls.used_by_vp) {
            ++trace << " in " << cls << " for " << mp.method->info->name
                    << " parameter " << mp.param << "\n";

            indent _(trace);

            ++trace << "reserved slots: " << cls.reserved_slots
                    << " used slots: " << cls.used_slots << "\n";

            auto unavailable_slots = cls.used_slots;
            detail::merge_into(cls.reserved_slots, unavailable_slots);

            ++trace << "unavailable slots: " << unavailable_slots << "\n";

            std::size_t slot = 0;

            for (; slot < unavailable_slots.size(); ++slot) {
                if (!unavailable_slots[slot]) {
                    break;
                }
            }

            ++trace << "first available slot: " << slot << "\n";

            mp.method->slots[mp.param] = slot;
            detail::set_bit(cls.used_slots, slot);
            detail::set_bit(cls.reserved_slots, slot);

            {
                ++trace << "reserve slots " << cls.used_slots << " in:\n";
                indent _(trace);

                for (auto base : cls.transitive_bases) {
                    ++trace << *base << "\n";
                    detail::merge_into(cls.used_slots, base->reserved_slots);
                }
            }

            {
                ++trace << "assign slots " << cls.used_slots << " in:\n";
                indent _(trace);

                for (auto covariant : cls.covariant_classes) {
                    if (&cls != covariant) {
                        ++trace << *covariant << "\n";
                        detail::merge_into(
                            cls.used_slots, covariant->used_slots);

                        for (auto base : covariant->transitive_bases) {
                            ++trace << *base << "\n";
                            detail::merge_into(
                                cls.used_slots, base->reserved_slots);
                        }
                    }
                }
            }
        }
    }

    for (auto pd : cls.direct_derived) {
        assign_lattice_slots(*pd);
    }
}

template<class Policy>
void compiler<Policy>::build_dispatch_tables() {
    using namespace detail;

    for (auto& m : methods) {
        ++trace << "Building dispatch table for " << m.info->name << "\n";
        indent _(trace);

        auto dims = m.arity();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            std::size_t dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];
                ++trace << "make groups for param #" << dim << ", class " << *vp
                        << "\n";
                indent _(trace);

                for (auto covariant_class : vp->covariant_classes) {
                    ++trace << "specs applicable to " << *covariant_class
                            << "\n";
                    bitvec mask;
                    mask.resize(m.specs.size());

                    std::size_t group_index = 0;
                    indent _(trace);

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->covariant_classes.find(
                                covariant_class) !=
                            spec.vp[dim]->covariant_classes.end()) {
                            ++trace << type_name(spec.info->type) << "\n";
                            mask[group_index] = 1;
                        }
                        ++group_index;
                    }

                    auto& group = dim_group[mask];
                    group.classes.push_back(covariant_class);
                    group.has_concrete_classes = group.has_concrete_classes ||
                        !covariant_class->is_abstract;

                    ++trace << "-> mask: " << mask << "\n";
                }

                ++dim;
            }
        }

        {
            std::size_t stride = 1;
            m.strides.reserve(dims - 1);

            for (std::size_t dim = 1; dim < m.arity(); ++dim) {
                stride *= groups[dim - 1].size();
                ++trace << "    stride for dim " << dim << " = " << stride
                        << "\n";
                m.strides.push_back(stride);
            }
        }

        for (std::size_t dim = 0; dim < m.arity(); ++dim) {
            indent _(trace);
            std::size_t group_num = 0;

            for (auto& [mask, group] : groups[dim]) {
                ++trace << "groups for dim " << dim << ":\n";
                indent _(trace);
                ++trace << group_num << " mask " << mask << ":\n";

                for (auto cls : group.classes) {
                    indent _(trace);
                    ++trace << type_name(cls->type_ids[0]) << "\n";
                    auto& entry = cls->vtbl[m.slots[dim] - cls->first_slot];
                    entry.method_index = &m - &methods[0];
                    entry.vp_index = dim;
                    entry.group_index = group_num;
                }

                ++group_num;
            }
        }

        {
            ++trace << "assigning specs\n";
            bitvec all(m.specs.size());
            all = ~all;
            build_dispatch_table(m, dims - 1, groups.end() - 1, all, true);

            if (m.arity() > 1) {
                indent _(trace);
                m.report.cells = 1;
                ++trace << "dispatch table rank: ";
                const char* prefix = "";

                for (const auto& dim_groups : groups) {
                    m.report.cells *= dim_groups.size();
                    trace << prefix << dim_groups.size();
                    prefix = " x ";
                }

                m.report.concrete_cells = 1;
                prefix = ", concrete only: ";

                for (const auto& dim_groups : groups) {
                    auto cells = std::count_if(
                        dim_groups.begin(), dim_groups.end(),
                        [](const auto& group) {
                            return group.second.has_concrete_classes;
                        });
                    m.report.concrete_cells *= cells;
                    trace << prefix << cells;
                    prefix = " x ";
                }

                trace << "\n";
            }

            print(m.report);
            accumulate(m.report, report);
            ++trace << "assigning next\n";

            std::vector<const definition*> specs;
            std::transform(
                m.specs.begin(), m.specs.end(), std::back_inserter(specs),
                [](const definition& spec) { return &spec; });

            for (auto& spec : m.specs) {
                indent _(trace);
                ++trace << type_name(spec.info->type) << ":\n";
                std::vector<const definition*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(), std::back_inserter(candidates),
                    [&spec](const definition* other) {
                        return is_base(other, &spec);
                    });

                if constexpr (trace_enabled) {
                    indent _(trace);
                    ++trace << "for next, select best:\n";

                    for (auto& candidate : candidates) {
                        indent _(trace);
                        ++trace << "#" << candidate->spec_index
                                << type_name(candidate->info->type) << "\n";
                    }
                }

                auto nexts = best(candidates);
                void* next;

                if (nexts.size() == 1) {
                    const definition_info* next_info = nexts.front()->info;
                    next = next_info->pf;
                    ++trace << "-> "
                            << "#" << nexts.front()->spec_index
                            << type_name(next_info->type) << "\n";
                } else if (nexts.empty()) {
                    ++trace << "-> none\n";
                    next = m.info->not_implemented;
                } else if (nexts.size() > 1) {
                    ++trace << "->  ambiguous\n";
                    next = m.info->ambiguous;
                }

                if (spec.info->next) {
                    *spec.info->next = next;
                }
            }
        }
    }
}

template<class Policy>
void compiler<Policy>::build_dispatch_table(
    method& m, std::size_t dim,
    std::vector<group_map>::const_iterator group_iter, const bitvec& candidates,
    bool concrete) {
    using namespace detail;

    indent _(trace);
    std::size_t group_index = 0;

    for (const auto& [group_mask, group] : *group_iter) {
        auto mask = candidates & group_mask;

        if constexpr (trace_enabled) {
            ++trace << "group " << dim << "/" << group_index << " mask " << mask
                    << "\n";
            indent _(trace);
            for (auto cls : range{group.classes.begin(), group.classes.end()}) {
                ++trace << type_name(cls->type_ids[0]) << "\n";
            }
        }

        if (dim == 0) {
            std::vector<const definition*> applicable;
            std::size_t i = 0;

            for (const auto& spec : m.specs) {
                if (mask[i]) {
                    applicable.push_back(&spec);
                }
                ++i;
            }

            if constexpr (trace_enabled) {
                ++trace << "select best of:\n";
                indent _(trace);

                for (auto& app : applicable) {
                    ++trace << "#" << app->spec_index << " "
                            << type_name(app->info->type) << "\n";
                }
            }

            auto specs = best(applicable);

            if (specs.size() > 1) {
                indent _(trace);
                ++trace << "ambiguous\n";
                m.dispatch_table.push_back(&m.ambiguous);
                ++m.report.ambiguous;
                if (concrete) {
                    ++m.report.concrete_ambiguous;
                }
            } else if (specs.empty()) {
                indent _(trace);
                ++trace << "not implemented\n";
                m.dispatch_table.push_back(&m.not_implemented);
                ++m.report.not_implemented;
                if (concrete && group.has_concrete_classes) {
                    ++m.report.concrete_not_implemented;
                }
            } else {
                auto spec = specs[0];
                m.dispatch_table.push_back(spec);
                ++trace << "-> #" << spec->spec_index << " "
                        << type_name(spec->info->type)
                        << " pf = " << spec->info->pf << "\n";
            }
        } else {
            build_dispatch_table(
                m, dim - 1, group_iter - 1, mask,
                concrete && group.has_concrete_classes);
        }
        ++group_index;
    }
}

inline void generic_compiler::accumulate(
    const update_method_report& partial, update_report& total) {
    total.cells += partial.cells;
    total.concrete_cells += partial.concrete_cells;
    total.not_implemented += partial.not_implemented != 0;
    total.concrete_not_implemented += partial.concrete_not_implemented != 0;
    total.ambiguous += partial.ambiguous != 0;
    total.concrete_ambiguous += partial.concrete_ambiguous != 0;
}

template<class Policy>
void compiler<Policy>::install_gv() {
    using namespace policy;

    auto dispatch_data_size = std::accumulate(
        methods.begin(), methods.end(), std::size_t(0),
        [](auto sum, auto& m) { return sum + m.dispatch_table.size(); });
    dispatch_data_size = std::accumulate(
        classes.begin(), classes.end(), dispatch_data_size,
        [](auto sum, auto& cls) { return sum + cls.vtbl.size(); });

    Policy::dispatch_data.resize(dispatch_data_size);
    auto gv_first = Policy::dispatch_data.data();
    auto gv_last = gv_first + Policy::dispatch_data.size();
    auto gv_iter = gv_first;

    ++trace << "Initializing multi-method dispatch tables at " << gv_iter
            << "\n";

    for (auto& m : methods) {
        if (m.info->arity() == 1) {
            // Uni-methods just need an index in the method table.
            m.info->slots_strides_ptr[0] = m.slots[0];
            continue;
        }

        // multi-methods only

        auto strides_iter = std::copy(
            m.slots.begin(), m.slots.end(), m.info->slots_strides_ptr);
        std::copy(m.strides.begin(), m.strides.end(), strides_iter);

        if constexpr (trace_enabled) {
            ++trace << rflush(4, Policy::dispatch_data.size()) << " "
                    << " method #" << m.dispatch_table[0]->method_index << " "
                    << m.info->name << "\n";
            indent _(trace);

            for (auto& entry : m.dispatch_table) {
                ++trace << "spec #" << entry->spec_index << " "
                        << spec_name(m, entry) << "\n";
            }
        }

        m.gv_dispatch_table = gv_iter;
        assert(gv_iter + m.dispatch_table.size() <= gv_last);
        gv_iter = std::transform(
            m.dispatch_table.begin(), m.dispatch_table.end(), gv_iter,
            [](auto spec) { return spec->pf; });
    }

    ++trace << "Initializing v-tables at " << gv_iter << "\n";

    for (auto& cls : classes) {
        if (cls.first_slot == -1) {
            // corner case: no methods for this class
            *cls.static_vptr = gv_iter;
            continue;
        }

        *cls.static_vptr = gv_iter - cls.first_slot;

        ++trace << rflush(4, gv_iter - gv_first) << " " << gv_iter
                << " vtbl for " << cls << " slots " << cls.first_slot << "-"
                << (cls.first_slot + cls.vtbl.size() - 1) << "\n";
        indent _(trace);

        for (auto& entry : cls.vtbl) {
            ++trace << "method #" << entry.method_index << " ";
            auto& method = methods[entry.method_index];

            if (method.arity() == 1) {
                auto spec = method.dispatch_table[entry.group_index];
                trace << "spec #" << spec->spec_index << "\n";
                indent _(trace);
                ++trace << type_name(method.info->method_type) << "\n";
                ++trace << spec_name(method, spec);
                assert(gv_iter + 1 <= gv_last);
                *gv_iter++ = spec->pf;
            } else {
                trace << "vp #" << entry.vp_index << " group #"
                      << entry.group_index << "\n";
                indent _(trace);
                ++trace << type_name(method.info->method_type);
                assert(gv_iter + 1 <= gv_last);

                if (entry.vp_index == 0) {
                    *gv_iter++ = std::uintptr_t(
                        method.gv_dispatch_table + entry.group_index);
                } else {
                    *gv_iter++ = entry.group_index;
                }
            }

            trace << "\n";
        }
    }

    ++trace << rflush(4, Policy::dispatch_data.size()) << " " << gv_iter
            << " end\n";

    if constexpr (has_facet<Policy, external_vptr>) {
        Policy::publish_vptrs(classes.begin(), classes.end());
    }
}

template<class Policy>
std::vector<const generic_compiler::definition*>
compiler<Policy>::best(std::vector<const definition*>& candidates) {
    std::vector<const definition*> best;

    for (auto spec : candidates) {
        const definition* candidate = spec;

        for (auto iter = best.begin(); iter != best.end();) {
            if (is_more_specific(spec, *iter)) {
                iter = best.erase(iter);
            } else if (is_more_specific(*iter, spec)) {
                candidate = nullptr;
                break;
            } else {
                ++iter;
            }
        }

        if (candidate) {
            best.push_back(candidate);
        }
    }

    return best;
}

template<class Policy>
bool compiler<Policy>::is_more_specific(
    const definition* a, const definition* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*b_iter)->covariant_classes.find(*a_iter) !=
                (*b_iter)->covariant_classes.end()) {
                result = true;
            } else if (
                (*a_iter)->covariant_classes.find(*b_iter) !=
                (*a_iter)->covariant_classes.end()) {
                return false;
            }
        }
    }

    return result;
}

template<class Policy>
bool compiler<Policy>::is_base(const definition* a, const definition* b) {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*a_iter)->covariant_classes.find(*b_iter) ==
                (*a_iter)->covariant_classes.end()) {
                return false;
            } else {
                result = true;
            }
        }
    }

    return result;
}

template<class Policy>
void compiler<Policy>::print(const update_method_report& report) const {
    ++trace;

    if (report.cells) {
        // only for multi-methods, uni-methods don't have dispatch tables
        ++trace << report.cells << " dispatch table cells, ";
    }

    trace << report.not_implemented << " not implemented, ";
    trace << report.ambiguous << " ambiguities, concrete only: ";

    if (report.cells) {
        trace << report.concrete_cells << ", ";
    }

    trace << report.concrete_not_implemented << ", ";
    trace << report.concrete_ambiguous << "\n";
}

} // namespace detail
} // namespace yomm2
} // namespace yorel

#endif
