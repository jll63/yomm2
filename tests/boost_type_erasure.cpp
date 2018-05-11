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
#include <exception>

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

struct Dog {
};

register_class(Any);
register_class(int, Any);
register_class(Dog, Any);

declare_method(string, getType, (virtual_<Any>));

begin_method(string, getType, (int value)) {
    return "int " + std::to_string(value);
} end_method;

begin_method(string, getType, (Dog value)) {
    return "dog";
} end_method;

declare_method(string, add, (virtual_<Any>, virtual_<Any>));

begin_method(string, add, (int a, int b)) {
    return std::to_string(a + b);
} end_method;

begin_method(string, add, (Dog a, Dog b)) {
    return "puppies";
} end_method;

BOOST_AUTO_TEST_CASE(getType_erasure)
{
    using namespace yorel::yomm2;
    update_methods();

    Any x(10), y(32), tramp((Dog())), lady((Dog()));

    BOOST_TEST(getType(x) == "int 10");
    BOOST_TEST(getType(tramp) == "dog");

    BOOST_TEST("42" == add(x, y));
    BOOST_TEST("puppies" == add(tramp, lady));

    set_method_call_error_handler([](const yorel::yomm2::method_call_error& error) {
            throw std::runtime_error("nope"); });

    BOOST_CHECK_THROW(add(tramp, x), std::runtime_error);
}
