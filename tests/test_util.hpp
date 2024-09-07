#ifndef YOMM2_TEST_HELPERS_HPP
#define YOMM2_TEST_HELPERS_HPP

#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/compiler.hpp>

template<int Name>
struct test_policy_ :
#ifdef NDEBUG
    yorel::yomm2::policy::release::rebind<test_policy_<Name>>
#else
    yorel::yomm2::policy::debug::rebind<test_policy_<Name>>
#endif
{
};

#endif
