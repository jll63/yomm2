
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_STD_RTTI_HPP
#define YOREL_YOMM2_POLICY_STD_RTTI_HPP

#include <yorel/yomm2/policies/core.hpp>

#ifndef BOOST_NO_RTTI
#include <typeindex>
#include <typeinfo>
#include <boost/core/demangle.hpp>
#endif

namespace yorel {
namespace yomm2 {
namespace policies {

struct std_rtti : virtual rtti {
#ifndef BOOST_NO_RTTI
    template<class Class>
    static auto static_type() -> type_id {
        auto tip = &typeid(Class);
        return reinterpret_cast<type_id>(tip);
    }

    template<class Class>
    static auto dynamic_type(const Class& obj) -> type_id {
        auto tip = &typeid(obj);
        return reinterpret_cast<type_id>(tip);
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << boost::core::demangle(
            reinterpret_cast<const std::type_info*>(type)->name());
    }

    static auto type_index(type_id type) -> std::type_index {
        return std::type_index(*reinterpret_cast<const std::type_info*>(type));
    }

    template<typename D, typename B>
    static auto dynamic_cast_ref(B&& obj) -> D {
        return dynamic_cast<D>(obj);
    }
#endif
};

} // namespace policies
} // namespace yomm2
} // namespace yorel

#endif
