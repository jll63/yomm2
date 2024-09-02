
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_VPTR_MAP_HPP
#define YOREL_YOMM2_POLICY_VPTR_MAP_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

template<
    class Policy,
    class Map = std::unordered_map<type_id, const std::uintptr_t*>>
struct yOMM2_API_gcc vptr_map : virtual external_vptr {
    static Map vptrs;

    template<typename ForwardIterator>
    static void publish_vptrs(ForwardIterator first, ForwardIterator last) {
        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                vptrs[*type_iter] = iter->vptr();
            }
        }
    }

    template<class Class>
    static auto dynamic_vptr(const Class& arg) {
        return vptrs.find(Policy::dynamic_type(arg))->second;
    }
};

template<class Policy, class Map>
Map vptr_map<Policy, Map>::vptrs;

}
}
}

#endif
