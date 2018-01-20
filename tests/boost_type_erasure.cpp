// Copyright (c) 2018 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/any_cast.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/free.hpp>
#include <boost/type_erasure/typeid_of.hpp>
#include <boost/mpl/vector.hpp>
#include <iostream>
#include <vector>

#include <yorel/yomm2/cute.hpp>

namespace yorel {
namespace yomm2 {
namespace detail {

template<typename... CONCEPTS>
using any_type = boost::type_erasure::any< boost::mpl::vector<CONCEPTS...> >;

struct any_cast_ {};

template<typename... CONCEPTS, typename DERIVED>
struct select_cast< boost::type_erasure::any< boost::mpl::vector<CONCEPTS...> >, DERIVED > {
    using type = any_cast_;
};

template<typename... CONCEPTS>
struct virtual_traits< virtual_< boost::type_erasure::any< boost::mpl::vector<CONCEPTS...> > > > {
    using base_type = boost::type_erasure::any< boost::mpl::vector<CONCEPTS...> >;
    using argument_type = base_type;

    template <typename SPEC>
    struct spec_arg_type {
        using type = SPEC;
    };

    static auto key(argument_type arg) {
        return &boost::type_erasure::typeid_of(arg);
    }

    template<class DERIVED>
    static DERIVED cast(argument_type obj, any_cast_) {
        return boost::type_erasure::any_cast< std::remove_pointer_t<DERIVED> >(obj);
    }
};

}
}
}

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

namespace mpl = boost::mpl;
using namespace boost::type_erasure;

using yorel::yomm2::virtual_;

using std::string;

using Any = any<mpl::vector<copy_constructible<>, typeid_<>, relaxed> >;

declare_method(string, type, (virtual_<Any>));

begin_method(string, type, (int value)) {
    return "int " + std::to_string(value);
} end_method;

register_class(Any);
register_class(int, Any);

struct Animal {};

BOOST_AUTO_TEST_CASE(type_erasure)
{
    //yorel::yomm2::detail::log_on(&std::cerr);
    yorel::yomm2::update_methods();
    Any x(10);
    BOOST_TEST(type(x) == "int 10");
}
