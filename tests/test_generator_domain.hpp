#ifndef TEST_GENERATOR_DOMAIN_HPP
#define TEST_GENERATOR_DOMAIN_HPP

#include <string>
#include <yorel/yomm2/policy.hpp>

struct throw_policy
    : yorel::yomm2::default_policy::rebind<throw_policy>::replace<
          yorel::yomm2::policies::error_handler,
          yorel::yomm2::policies::throw_error> {};

#define YOMM2_DEFAULT_POLICY throw_policy

#include <yorel/yomm2.hpp>

#ifndef _MSC_VER
// Because MSC is believes that forward declaring with 'struct' or 'class' makes
// a difference.
#if __has_include("test_generator_slots.hpp")
#include "test_generator_slots.hpp"
#endif
#endif

struct Animal {
    virtual ~Animal() {
    }
};

struct Cat : Animal {};
struct Dog : Animal {};

struct Property {
    explicit Property(std::string owner) : owner(owner) {
    }

    virtual ~Property() {
    }

    std::string owner;
};

struct DomesticCat : Cat, Property {
    using Property::Property;
};

struct DomesticDog : Dog, Property {
    using Property::Property;
};

declare_method(void, kick, (virtual_<Animal&>, std::ostream&));
declare_method(
    void, meet, (virtual_<Animal&>, virtual_<Animal&>, std::ostream&));
declare_method(void, identify, (virtual_<Property&>, std::ostream&));

#ifdef _MSC_VER
// Because MSC is believes that forward declaring with 'struct' or 'class' makes
// a difference.
#if __has_include("test_generator_slots.hpp")
#include "test_generator_slots.hpp"
#endif
#endif

#endif // TEST_GENERATOR_DOMAIN_HPP
