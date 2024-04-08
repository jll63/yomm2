#ifndef YOMM2_TEST_HELPERS_INCLUDED
#define YOMM2_TEST_HELPERS_INCLUDED

#include <yorel/yomm2/core.hpp>

template<int Key>
struct test_policy_ :
#ifdef NDEBUG
    yorel::yomm2::policy::release::rebind<test_policy_<Key>>
#else
    yorel::yomm2::policy::debug::rebind<test_policy_<Key>>
#endif
{
};

#endif
