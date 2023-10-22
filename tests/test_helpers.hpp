#ifndef YOMM2_TEST_HELPERS_INCLUDED
#define YOMM2_TEST_HELPERS_INCLUDED

#include <yorel/yomm2/core.hpp>

template<int Key, class BasePolicy = yorel::yomm2::default_policy>
struct test_policy_ : BasePolicy {
    static yorel::yomm2::catalog catalog;
    static yorel::yomm2::context context;
};

template<int Key, class BasePolicy>
yorel::yomm2::catalog test_policy_<Key, BasePolicy>::catalog;

template<int Key, class BasePolicy>
yorel::yomm2::context test_policy_<Key, BasePolicy>::context;

struct yomm2_update {
    yomm2_update() {
        yorel::yomm2::update();
    }
};

#endif