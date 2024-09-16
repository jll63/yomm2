// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

#include <string>

#define BOOST_TEST_MODULE yomm2
#include <boost/test/included/unit_test.hpp>

using yorel::yomm2::virtual_;

class Animal {
  public:
    virtual ~Animal() {
    }
};

class Dog : public Animal {};

register_classes(Animal, Dog, Animal);

declare_method(std::string, kick, (virtual_<Animal&>));

define_method(std::string, kick, (Dog & dog)) {
    return "bark";
}

BOOST_AUTO_TEST_CASE(noadl) {
    yorel::yomm2::initialize();
    auto stimulus = &kick;
    Dog snoopy;
    Animal& animal = snoopy;
    BOOST_TEST(stimulus(snoopy) == "bark");
}
