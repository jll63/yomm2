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

struct generic_compiler {

    struct method;

    struct parameter {
        struct method* method;
        size_t param;
    };

    struct vtbl_entry {
        size_t method_index, vp_index, group_index;
    };

    struct class_ {
        bool is_abstract{false};
        std::vector<type_id> type_ids;
        std::vector<class_*> transitive_bases;
        std::vector<class_*> direct_bases;
        std::vector<class_*> direct_derived;
        std::unordered_set<class_*> compatible_classes;
        std::vector<parameter> used_by_vp;
        int next_slot{0};
        int first_used_slot{-1};
        int layer{0};
        size_t mark{0};   // temporary mark to detect cycles
        size_t weight{0}; // number of proper direct or indirect bases
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
        size_t method_index, spec_index;
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
        std::vector<size_t> slots;
        std::vector<size_t> strides;
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
    size_t class_visit = 0;
    bool compilation_done = false;
};

template<class Policy>
auto& operator<<(
    trace_type<Policy>& trace, generic_compiler::vtbl_entry entry) {
    return trace << entry.method_index << "/" << entry.vp_index << "/"
                 << entry.group_index;
}

template<class Policy>
trace_type<Policy>&
operator<<(trace_type<Policy>& trace, const generic_compiler::definition* def) {
    return trace << def->method_index << "/" << def->spec_index;
}

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

template<class Policy>
auto& operator<<(trace_type<Policy>& trace, const type_name& manip) {
    if constexpr (Policy::template has_facet<policy::trace_output>) {
        Policy::type_name(manip.type, trace);
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
    void calculate_compatible_classes(class_& cls);
    void augment_methods();
    std::vector<class_*> layer_classes();
    void allocate_slots();
    void allocate_slot_down(class_* cls, size_t slot);
    void allocate_slot_up(class_* cls, size_t slot);
    void build_dispatch_tables();
    void build_dispatch_table(
        method& m, size_t dim, std::vector<group_map>::const_iterator group,
        const bitvec& candidates, bool concrete);
    void install_gv();
    void optimize();
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
    optimize();

    print(report);
    ++trace << "Finished\n";
}

template<class Policy>
auto compiler<Policy>::compile() {
    resolve_static_type_ids();
    augment_classes();
    augment_methods();
    allocate_slots();
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
                    for (auto& ti :
                         detail::range{ci.first_base, ci.last_base}) {
                        resolve(&ti);
                    }

                    *ci.last_base = 1;
                }
            }

        if (!Policy::methods.empty())
            for (auto& method : Policy::methods) {
                for (auto& ti : detail::range{method.vp_begin, method.vp_end}) {
                    if (*method.vp_end == 0) {
                        resolve(&ti);
                        *method.vp_end = 1;
                    }

                    if (!method.specs.empty())
                        for (auto& definition : method.specs) {
                            if (*definition.vp_end == 0) {
                                for (auto& ti : detail::range{
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
            if constexpr (trace_enabled) {
                {
                    indent YOMM2_GENSYM(trace);
                    ++trace << type_name(cr.type) << ": "
                            << range{cr.first_base, cr.last_base};

                    ++trace << "\n";
                }
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

    size_t mark = ++class_visit;

    for (auto& rtc : classes) {
        decltype(rtc.transitive_bases) bases;
        mark = ++class_visit;

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
        mark = ++class_visit;

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
        calculate_compatible_classes(rtc);
    }

    if constexpr (trace_enabled) {
        ++trace << "Inheritance lattice:\n";
        for (auto& rtc : classes) {
            indent YOMM2_GENSYM(trace);
            ++trace << rtc << "\n";

            {
                indent YOMM2_GENSYM(trace);
                ++trace << "bases:      " << rtc.direct_bases << "\n";
                ++trace << "derived:    " << rtc.direct_derived << "\n";
                ++trace << "compatible: " << rtc.compatible_classes << "\n";
            }
        }
    }
}

template<class Policy>
void compiler<Policy>::calculate_compatible_classes(class_& cls) {
    if (!cls.compatible_classes.empty()) {
        return;
    }

    cls.compatible_classes.insert(&cls);

    for (auto derived : cls.direct_derived) {
        if (derived->compatible_classes.empty()) {
            calculate_compatible_classes(*derived);
        }

        std::copy(
            derived->compatible_classes.begin(),
            derived->compatible_classes.end(),
            std::inserter(
                cls.compatible_classes, cls.compatible_classes.end()));
    }
}

template<class Policy>
void compiler<Policy>::augment_methods() {
    using namespace policy;
    using namespace detail;

    methods.resize(Policy::methods.size());

    ++trace << "Methods:\n";
    indent YOMM2_GENSYM(trace);

    auto meth_iter = methods.begin();

    for (auto& meth_info : Policy::methods) {
        if constexpr (trace_enabled) {
            ++trace << meth_info.name << " "
                    << range{meth_info.vp_begin, meth_info.vp_end} << "\n";
        }

        indent YOMM2_GENSYM(trace);

        meth_iter->info = &meth_info;
        meth_iter->vp.reserve(meth_info.arity());
        size_t param_index = 0;

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
        meth_iter->ambiguous.pf =
            reinterpret_cast<uintptr_t>(meth_iter->info->ambiguous);
        meth_iter->not_implemented.pf =
            reinterpret_cast<uintptr_t>(meth_iter->info->not_implemented);

        meth_iter->specs.resize(meth_info.specs.size());
        auto spec_iter = meth_iter->specs.begin();

        for (auto& definition_info : meth_info.specs) {
            spec_iter->method_index = meth_iter - methods.begin();
            spec_iter->spec_index = spec_iter - meth_iter->specs.begin();

            ++trace << type_name(definition_info.type) << " ("
                    << definition_info.pf << ")\n";
            spec_iter->info = &definition_info;
            spec_iter->vp.reserve(meth_info.arity());
            size_t param_index = 0;

            for (auto type :
                 range{definition_info.vp_begin, definition_info.vp_end}) {
                indent YOMM2_GENSYM(trace);
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
        size_t param_index = 0;

        for (auto vp : method.vp) {
            vp->used_by_vp.push_back({&method, param_index++});
        }
    }
}

template<class Policy>
std::vector<generic_compiler::class_*> compiler<Policy>::layer_classes() {
    ++trace << "Layering classes...\n";

    std::vector<class_*> input;
    input.reserve(classes.size());
    std::transform(
        classes.begin(), classes.end(), std::back_inserter(input),
        [](class_& cls) { return &cls; });

    std::vector<class_*> layered;
    layered.reserve(classes.size());

    for (int layer = 1; !input.empty(); ++layer) {
        indent YOMM2_GENSYM(trace, 1);
        ++trace;

        for (auto class_iter = input.begin(); class_iter != input.end();) {
            auto seen_all_bases = true;
            auto in_this_layer = (*class_iter)->direct_bases.empty();

            for (auto base : (*class_iter)->direct_bases) {
                if (!base->layer) {
                    seen_all_bases = false;
                    break;
                } else if (base->layer == layer) {
                    in_this_layer = false;
                    break;
                }
                if (base->layer == layer - 1) {
                    in_this_layer = true;
                }
            }

            if (seen_all_bases && in_this_layer) {
                layered.push_back(*class_iter);
                (*class_iter)->layer = layer;

                if constexpr (trace_enabled) {
                    trace << " " << **class_iter;
                }

                class_iter = input.erase(class_iter);
            } else {
                ++class_iter;
            }
        }
        trace << "\n";
    }

    return std::move(layered);
}

template<class Policy>
void compiler<Policy>::allocate_slots() {
    auto layered = layer_classes();

    ++trace << "Allocating slots...\n";
    indent YOMM2_GENSYM(trace);

    for (auto cls : layered) {
        for (const auto& mp : cls->used_by_vp) {
            size_t slot = cls->next_slot++;

            ++trace << mp.method->info->name << "#" << mp.param << ": slot "
                    << slot << "\n";
            indent YOMM2_GENSYM(trace);
            ++trace << *cls;

            if (mp.method->slots.size() <= mp.param) {
                mp.method->slots.resize(mp.param + 1);
            }

            mp.method->slots[mp.param] = slot;

            if (cls->first_used_slot == -1) {
                cls->first_used_slot = slot;
            }

            cls->mark = ++class_visit;

            for (auto derived : cls->direct_derived) {
                allocate_slot_down(derived, slot);
            }

            ++trace << "\n";
        }
    }

    for (auto& c : classes) {
        c.vtbl.resize(c.next_slot);
    }
}

template<class Policy>
void compiler<Policy>::allocate_slot_down(class_* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << *cls;

    assert(slot >= cls->next_slot);

    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto b : cls->direct_bases) {
        allocate_slot_up(b, slot);
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_down(d, slot);
    }
}

template<class Policy>
void compiler<Policy>::allocate_slot_up(class_* cls, size_t slot) {

    if (cls->mark == class_visit)
        return;

    cls->mark = class_visit;

    trace << " " << *cls;

    assert(slot >= cls->next_slot);
    cls->next_slot = slot + 1;

    if (cls->first_used_slot == -1) {
        cls->first_used_slot = slot;
    }

    for (auto b : cls->direct_bases) {
        allocate_slot_up(b, slot);
    }

    for (auto d : cls->direct_derived) {
        allocate_slot_down(d, slot);
    }
}

template<class Policy>
void compiler<Policy>::build_dispatch_tables() {
    using namespace detail;

    for (auto& m : methods) {
        ++trace << "Building dispatch table for " << m.info->name << "\n";
        indent YOMM2_GENSYM(trace);

        auto dims = m.arity();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            size_t dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];
                ++trace << "make groups for param #" << dim << ", class " << *vp
                        << "\n";
                indent YOMM2_GENSYM(trace);

                for (auto compatible_class : vp->compatible_classes) {
                    ++trace << "specs applicable to " << *compatible_class
                            << "\n";
                    bitvec mask;
                    mask.resize(m.specs.size());

                    size_t group_index = 0;
                    indent YOMM2_GENSYM(trace);

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->compatible_classes.find(
                                compatible_class) !=
                            spec.vp[dim]->compatible_classes.end()) {
                            ++trace << type_name(spec.info->type) << "\n";
                            mask[group_index] = 1;
                        }
                        ++group_index;
                    }

                    auto& group = dim_group[mask];
                    group.classes.push_back(compatible_class);
                    group.has_concrete_classes = group.has_concrete_classes ||
                        !compatible_class->is_abstract;

                    ++trace << "-> mask: " << mask << "\n";
                }

                ++dim;
            }
        }

        {
            size_t stride = 1;
            m.strides.reserve(dims - 1);

            for (size_t dim = 1; dim < m.arity(); ++dim) {
                stride *= groups[dim - 1].size();
                ++trace << "    stride for dim " << dim << " = " << stride
                        << "\n";
                m.strides.push_back(stride);
            }
        }

        for (size_t dim = 0; dim < m.arity(); ++dim) {
            ++trace << "groups for dim " << dim << ":\n";
            indent YOMM2_GENSYM(trace);
            size_t group_num = 0;
            for (auto& [mask, group] : groups[dim]) {
                for (auto cls : group.classes) {
                    auto& entry = cls->vtbl[m.slots[dim]];
                    entry.method_index = &m - &methods[0];
                    entry.vp_index = dim;
                    entry.group_index = group_num;
                }
                if constexpr (trace_enabled) {
                    ++trace << group_num << " mask " << mask << "\n";
                    indent YOMM2_GENSYM(trace);
                    for (auto cls :
                         range{group.classes.begin(), group.classes.end()}) {
                        ++trace << type_name(cls->type_ids[0]) << "\n";
                    }
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
                indent YOMM2_GENSYM(trace);
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
                indent YOMM2_GENSYM(trace);
                ++trace << type_name(spec.info->type) << ":\n";
                std::vector<const definition*> candidates;
                std::copy_if(
                    specs.begin(), specs.end(), std::back_inserter(candidates),
                    [spec](const definition* other) {
                        return is_base(other, &spec);
                    });

                if constexpr (trace_enabled) {
                    indent YOMM2_GENSYM(trace);
                    ++trace << "for next, select best:\n";

                    for (auto& candidate : candidates) {
                        indent YOMM2_GENSYM(trace);
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
    method& m, size_t dim, std::vector<group_map>::const_iterator group_iter,
    const bitvec& candidates, bool concrete) {
    using namespace detail;

    indent YOMM2_GENSYM(trace);
    size_t group_index = 0;

    for (const auto& [group_mask, group] : *group_iter) {
        auto mask = candidates & group_mask;

        if constexpr (trace_enabled) {
            ++trace << "group " << dim << "/" << group_index << " mask " << mask
                    << "\n";
            indent YOMM2_GENSYM(trace);
            for (auto cls : range{group.classes.begin(), group.classes.end()}) {
                ++trace << type_name(cls->type_ids[0]) << "\n";
            }
        }

        if (dim == 0) {
            std::vector<const definition*> applicable;
            size_t i = 0;

            for (const auto& spec : m.specs) {
                if (mask[i]) {
                    applicable.push_back(&spec);
                }
                ++i;
            }

            if constexpr (trace_enabled) {
                ++trace << "select best of:\n";
                indent YOMM2_GENSYM(trace);

                for (auto& app : applicable) {
                    ++trace << "#" << app->spec_index << " "
                            << type_name(app->info->type) << "\n";
                }
            }

            auto specs = best(applicable);

            if (specs.size() > 1) {
                indent YOMM2_GENSYM(trace);
                ++trace << "ambiguous\n";
                m.dispatch_table.push_back(&m.ambiguous);
                ++m.report.ambiguous;
                if (concrete) {
                    ++m.report.concrete_ambiguous;
                }
            } else if (specs.empty()) {
                indent YOMM2_GENSYM(trace);
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

    for (size_t pass = 0; pass != 2; ++pass) {
        Policy::dispatch_data.resize(0);

        if constexpr (trace_enabled) {
            if (pass) {
                ++trace << "Initializing multi-method dispatch tables at "
                        << Policy::dispatch_data.data() << "\n";
            }
        }

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
                if (pass) {
                    ++trace << rflush(4, Policy::dispatch_data.size()) << " "
                            << Policy::dispatch_data.data() +
                            Policy::dispatch_data.size()
                            << " dispatch table for " << m.info->name << "\n";
                    indent YOMM2_GENSYM(trace);
                    ++trace
                        << range(
                               m.dispatch_table.begin(), m.dispatch_table.end())
                        << "\n";
                }
            }

            m.gv_dispatch_table =
                Policy::dispatch_data.data() + Policy::dispatch_data.size();
            std::transform(
                m.dispatch_table.begin(), m.dispatch_table.end(),
                std::back_inserter(Policy::dispatch_data),
                [](auto spec) { return spec->pf; });
        }

        if constexpr (trace_enabled) {
            if (pass) {
                ++trace << "Initializing v-tables at "
                        << (Policy::dispatch_data.data() +
                            Policy::dispatch_data.size())
                        << "\n";
            }
        }

        for (auto& cls : classes) {
            if (cls.first_used_slot == -1) {
                // corner case: no methods for this class
                *cls.static_vptr =
                    Policy::dispatch_data.data() + Policy::dispatch_data.size();
            } else {
                *cls.static_vptr = Policy::dispatch_data.data() +
                    Policy::dispatch_data.size() - cls.first_used_slot;
            }

            if constexpr (trace_enabled) {
                if (pass) {
                    ++trace << rflush(4, Policy::dispatch_data.size()) << " "
                            << *cls.static_vptr << " vtbl for " << cls
                            << " slots " << cls.first_used_slot << "-"
                            << cls.vtbl.size() << "\n";
                    indent YOMM2_GENSYM(trace);

                    for (auto entry : cls.vtbl) {
                        ++trace << "method " << entry.method_index << " vp "
                                << entry.vp_index << " group "
                                << entry.group_index << " ";
                        auto method = methods[entry.method_index];
                        Policy::type_name(method.info->method_type, trace);

                        if (method.arity() == 1) {
                            auto spec =
                                method.dispatch_table[entry.group_index];
                            trace << " ";

                            if (spec->pf ==
                                methods[entry.method_index].ambiguous.pf) {
                                trace << "ambiguous";
                            } else if (
                                spec->pf ==
                                methods[entry.method_index]
                                    .not_implemented.pf) {
                                trace << "not implemented";
                            } else {
                                Policy::type_name(spec->info->type, trace);
                            }
                        }

                        trace << "\n";
                    }
                }
            }

            if (cls.first_used_slot != -1) {
                std::transform(
                    cls.vtbl.begin() + cls.first_used_slot, cls.vtbl.end(),
                    std::back_inserter(Policy::dispatch_data),
                    [](auto entry) { return entry.group_index; });
            }
        }
    }

    ++trace << rflush(4, Policy::dispatch_data.size()) << " "
            << Policy::dispatch_data.data() + Policy::dispatch_data.size()
            << " end\n";

    if constexpr (has_facet<Policy, external_vptr>) {
        Policy::publish_vptrs(classes.begin(), classes.end());
    }
}

template<class Policy>
void compiler<Policy>::optimize() {
    ++trace << "Optimizing\n";

    for (auto& m : methods) {
        ++trace << "  " << m.info->name << "\n";
        indent YOMM2_GENSYM(trace);
        auto slot = m.slots[0];

        if (m.arity() == 1) {
            for (auto cls : m.vp[0]->compatible_classes) {
                auto spec = m.dispatch_table[(*cls->static_vptr)[slot]];
                if constexpr (trace_enabled) {
                    ++trace << *cls << " vtbl[" << slot
                            << "] = " << spec->method_index << "/"
                            << spec->spec_index << " function "
                            << (void*)spec->pf << "\n";
                }
                (*cls->static_vptr)[slot] = spec->pf;
            }
        } else {
            for (auto cls : m.vp[0]->compatible_classes) {
                auto pw = m.gv_dispatch_table + (*cls->static_vptr)[slot];

                if constexpr (trace_enabled) {
                    ++trace << *cls << " vtbl[" << slot << "] = gv+"
                            << (pw - Policy::dispatch_data.data()) << "\n";
                }

                (*cls->static_vptr)[slot] =
                    reinterpret_cast<std::uintptr_t>(pw);
            }
        }
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
            if ((*b_iter)->compatible_classes.find(*a_iter) !=
                (*b_iter)->compatible_classes.end()) {
                result = true;
            } else if (
                (*a_iter)->compatible_classes.find(*b_iter) !=
                (*a_iter)->compatible_classes.end()) {
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
            if ((*a_iter)->compatible_classes.find(*b_iter) ==
                (*a_iter)->compatible_classes.end()) {
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
