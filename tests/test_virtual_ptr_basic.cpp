// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/compiler.hpp>

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "test_helpers.hpp"

using std::cout;
using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;

auto debug_handler = &default_policy::error;

namespace YOMM2_GENSYM {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

register_classes(Animal, Dog);

namespace YOMM2_GENSYM {

void kick_dog(virtual_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

using kick = method<void, void(virtual_ptr<Animal>, std::ostream&)>;
YOMM2_STATIC(kick::add_function<kick_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_ptr_by_ref) {
    yorel::yomm2::update();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        virtual_ptr<Animal> vptr(dog);
        kick::fn(vptr, os);
    }

    {
        // Using  deduction guide.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        auto vptr = virtual_ptr(animal); // GLOP
        kick::fn(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        // Using conversion ctor.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        kick::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

BOOST_AUTO_TEST_CASE(test_final_error) {
    auto prev_handler = set_error_handler([](const error_type& ev) {
        if (auto error = std::get_if<method_table_error>(&ev)) {
            static_assert(
                std::is_same_v<decltype(error), const method_table_error*>);
            throw *error;
        }
    });

    yorel::yomm2::update();
    bool threw = false;

    try {
        Dog snoopy;
        Animal& animal = snoopy;
        virtual_ptr<Animal>::final(animal);
    } catch (const method_table_error& error) {
        set_error_handler(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(Dog)));
        threw = true;
    } catch (...) {
        set_error_handler(prev_handler);
        BOOST_FAIL("wrong exception");
        return;
    }

    if constexpr (default_policy::has_facet<policy::runtime_checks>) {
        if (!threw) {
            BOOST_FAIL("should have thrown");
        }
    } else {
        if (threw) {
            BOOST_FAIL("should not have thrown");
        }
    }
}
} // namespace YOMM2_GENSYM

namespace YOMM2_GENSYM {

void kick_dog(virtual_shared_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

using kick = method<void, void(virtual_shared_ptr<Animal>, std::ostream&)>;
YOMM2_STATIC(kick::add_function<kick_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_value) {
    yorel::yomm2::update();

    {
        boost::test_tools::output_test_stream os;
        virtual_shared_ptr<Animal> animal = make_virtual_shared<Dog>();
        kick::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}
} // namespace YOMM2_GENSYM

namespace YOMM2_GENSYM {

void kick_dog(const virtual_shared_ptr<Dog>&, std::ostream& os) {
    os << "bark";
}

using kick =
    method<void, void(const virtual_shared_ptr<Animal>&, std::ostream&)>;
YOMM2_STATIC(kick::add_function<kick_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_const_reference) {
    yorel::yomm2::update();

    {
        boost::test_tools::output_test_stream os;
        virtual_shared_ptr<Animal> animal = make_virtual_shared<Dog>();
        kick::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}
} // namespace YOMM2_GENSYM
} // namespace YOMM2_GENSYM

namespace YOMM2_GENSYM {

struct Animal {};

struct Dog : Animal {};

register_classes(Animal, Dog);

void kick_dog(virtual_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

using kick = method<void, void(virtual_ptr<Animal>, std::ostream&)>;
YOMM2_STATIC(kick::add_function<kick_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_ptr_non_polymorphic) {
    yorel::yomm2::update();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        auto vptr = virtual_ptr<Dog>::final(dog);
        kick::fn(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}
} // namespace YOMM2_GENSYM
