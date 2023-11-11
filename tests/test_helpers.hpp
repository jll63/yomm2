#ifndef YOMM2_TEST_HELPERS_INCLUDED
#define YOMM2_TEST_HELPERS_INCLUDED

#include <yorel/yomm2/core.hpp>

template<int Key>
struct test_policy_ :
#ifdef NDEBUG
yorel::yomm2::policy::static_release<test_policy_<Key>>
#else
yorel::yomm2::policy::static_debug<test_policy_<Key>>
#endif
{};

struct yomm2_update {
    yomm2_update() {
        yorel::yomm2::update();
    }
};

#endif