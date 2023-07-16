// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <yorel/yomm2/keywords.hpp>

struct Animal {
    virtual ~Animal() {
    }

    Animal() = default;
    Animal(const Animal&) = delete;
};

using std::cout;
using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;

struct Dog : Animal {
    Dog() = default;
    Dog(const Dog&) = delete;
};

register_classes(Animal, Dog);

template<class T>
T& ref(T& value) {
    return value;
}

BOOST_AUTO_TEST_CASE(test_virtual_ptr_basic) {
    yorel::yomm2::update_methods();

    {
        Dog dog;

        {
            auto p = virtual_ptr<Dog>(dog);
            virtual_ptr<Dog> copy(p);
            virtual_ptr<Animal> upcast_copy(p);
            virtual_ptr<const Dog> const_copy(p);
            virtual_ptr<const Animal> const_upcast_copy(p);
        }

        {
            const auto p = virtual_ptr<Dog>(dog);
            virtual_ptr<Dog> const_copy(p);
            virtual_ptr<Animal> const_upcast_copy(p);
        }

        { auto p = virtual_ptr<const Animal>(dog); }

// should not compile
#ifdef YOMM2_SHOULD_NOT_COMPILE
        { auto vptr = virtual_ptr<Dog>(Dog()); }
#endif
    }

    {
        auto dog = std::make_shared<Dog>();

        {
            virtual_shared_ptr<Dog> p(dog);
            virtual_shared_ptr<Dog> copy(p);
            virtual_shared_ptr<Animal> upcast_copy(p);
        }

        { auto p = virtual_shared_ptr<Dog>(std::make_shared<Dog>()); }

        { auto p = virtual_shared_ptr<Animal>(std::make_shared<Dog>()); }
    }

    {
        auto dog = std::make_shared<const Dog>();

        {
            virtual_shared_ptr<const Dog> p(dog);
            virtual_shared_ptr<const Dog> copy(p);
            virtual_shared_ptr<const Animal> upcast_copy(p);
        }

        { auto p = virtual_shared_ptr<const Dog>(std::make_shared<const Dog>()); }

        { auto p = virtual_shared_ptr<const Animal>(std::make_shared<const Dog>()); }
    }
}
