#ifndef YOMM2_TEST_POLICY_INCLUDED
#define YOMM2_TEST_POLICY_INCLUDED

#include <yorel/yomm2/core.hpp>

namespace yorel {
namespace yomm2 {

template<int Key, class BasePolicy = yorel::yomm2::default_policy>
struct test_policy : BasePolicy {
    static yorel::yomm2::catalog catalog;
    static yorel::yomm2::context context;
};

template<int Key, class BasePolicy>
yorel::yomm2::catalog test_policy<Key, BasePolicy>::catalog;

template<int Key, class BasePolicy>
yorel::yomm2::context test_policy<Key, BasePolicy>::context;

} // namespace yomm2
} // namespace yorel

#endif