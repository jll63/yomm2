// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/runtime.hpp>
#include <yorel/yomm2/intrusive.hpp>
#include <yorel/yomm2/templates.hpp>

#include "test_helpers.hpp"

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>
#include <boost/utility/identity_type.hpp>

using namespace yorel::yomm2;

using direct_policy = test_policy_<__COUNTER__>;

struct indirect_policy : test_policy_<__COUNTER__> {
    static constexpr bool use_indirect_method_pointers = true;
};

template<class Bear>
auto kick_bear(Bear&) {
    return std::string("growl");
}

template<class Warrior, class Axe, class Bear>
auto fight_bear(Warrior&, Axe&, Bear&) {
    return "kill bear";
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    test_intrusive, Policy,
    BOOST_IDENTITY_TYPE((std::tuple<direct_policy, indirect_policy>))) {

    struct Character : root<Character, Policy> {
        virtual ~Character() {
        }
    };

    struct Warrior : Character, derived<Warrior> {};

    struct Device : root<Device, Policy> {
        virtual ~Device() {
        }
    };

    struct Axe : Device, derived<Axe> {};

    struct Creature : root<Creature, Policy> {
        virtual ~Creature() {
        }
    };

    struct Bear : Creature, derived<Bear> {};

    static use_classes<Policy, Character, Warrior, Device, Axe, Creature, Bear>
        YOMM2_GENSYM;

    struct YOMM2_SYMBOL(kick);
    using kick =
        method<YOMM2_SYMBOL(kick), std::string(virtual_<Creature&>), Policy>;
    static typename kick::template add_function<kick_bear<Bear>> YOMM2_GENSYM;

    using fight = method<
        void,
        std::string(
            virtual_<Character&>, virtual_<Device&>, virtual_<Creature&>),
        Policy>;
    static typename fight::template add_function<fight_bear<Warrior, Axe, Bear>>
        YOMM2_GENSYM;

    update<Policy>();

    Bear bear;
    Warrior warrior;
    Axe axe;

    {
        static_assert(detail::has_mptr<Character>);
        static_assert(detail::has_mptr<Character&>);
        static_assert(detail::has_mptr<const Character&>);

        Character character;
        BOOST_TEST(
            (character.yomm2_mptr() ==
             Policy::template method_table<Character>));
        BOOST_TEST((bear.yomm2_mptr() == Policy::template method_table<Bear>));
    }

    // BOOST_TEST(kick::fn(bear) == "growl");
    // BOOST_TEST(fight::fn(warrior, axe, bear) == "kill bear");
}

namespace bad_intrusive_mptr {

using test_policy = test_policy_<__COUNTER__>;

struct Animal : root<Animal, test_policy> {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(test_policy, Animal, Dog);

declare_method(std::string, kick, (virtual_<Animal&>), test_policy);

BOOST_AUTO_TEST_CASE(test_bad_intrusive_mptr) {
    auto prev_handler =
        test_policy::set_error_handler([](const error_type& ev) {
            if (auto error = std::get_if<method_table_error>(&ev)) {
                static_assert(
                    std::is_same_v<decltype(error), const method_table_error*>);
                throw *error;
            }
            throw std::runtime_error("other error");
        });

    update<test_policy>();

    try {
        Dog snoopy;
        kick(snoopy);
    } catch (const method_table_error& error) {
        set_error_handler(prev_handler);
#ifdef NDEBUG
        BOOST_FAIL("should not have thrown a method_table_error");
#else
        BOOST_TEST(error.ti->name() == typeid(Dog).name());
#endif
        return;
    } catch (...) {
        set_error_handler(prev_handler);
#ifdef NDEBUG
        return;
#else
        BOOST_FAIL("wrong exception");
#endif
    }

    set_error_handler(prev_handler);
    BOOST_FAIL("did not throw");
}
} // namespace bad_intrusive_mptr
