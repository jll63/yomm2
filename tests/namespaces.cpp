// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

namespace interfaces {
class Animal {
  public:
    virtual ~Animal() {}
};
}

namespace canis {
class Dog     : public interfaces::Animal {};
class Bulldog : public Dog {};
}

namespace felis {
class Cat     : public interfaces::Animal {};
}

namespace delphinus {
class Dolphin : public interfaces::Animal {};
}

#include <yorel/yomm2/cute.hpp>
#include <string>

using yorel::yomm2::virtual_;

register_class(interfaces::Animal);
register_class(canis::Dog, interfaces::Animal);
register_class(canis::Bulldog, canis::Dog);
register_class(felis::Cat, interfaces::Animal);
register_class(delphinus::Dolphin, interfaces::Animal);

// open method with single virtual argument <=> virtual function "from outside"
declare_method(std::string, kick, (virtual_<interfaces::Animal&>));

namespace canis {
// implement 'kick' for dogs
define_method(std::string, kick, (Dog& dog)) {
  return "bark";
}

// implement 'kick' for bulldogs
define_method(std::string, kick, (Bulldog& dog)) {
    return next(dog) + " and bite";
}
}

// multi-method
declare_method(
    std::string, meet,
    (virtual_<interfaces::Animal&>, virtual_<interfaces::Animal&>));

// 'meet' catch-all implementation
define_method(std::string, meet, (interfaces::Animal&, interfaces::Animal&)) {
  return "ignore";
}

define_method(std::string, meet, (canis::Dog& dog1, canis::Dog& dog2)) {
  return "wag tail";
}

define_method(std::string, meet, (canis::Dog& dog, felis::Cat& cat)) {
  return "chase";
}

define_method(std::string, meet, (felis::Cat& cat, canis::Dog& dog)) {
  return "run";
}

// -----------------------------------------------------------------------------
// main

#include <iostream>
#include <memory>

int main()
{
    yorel::yomm2::update_methods();

    std::unique_ptr<interfaces::Animal>
        hector = std::make_unique<canis::Bulldog>(),
        snoopy = std::make_unique<canis::Dog>();

    std::cout << "kick snoopy: " << kick(*snoopy) << "\n"; // bark
    std::cout << "kick hector: " << kick(*hector) << "\n"; // bark and bite

    std::unique_ptr<interfaces::Animal>
        sylvester = std::make_unique<felis::Cat>(),
        flipper = std::make_unique<delphinus::Dolphin>();

  std::cout << "hector meets sylvester: " << meet(*hector, *sylvester) << "\n"; // chase
  std::cout << "sylvester meets hector: " << meet(*sylvester, *hector) << "\n"; // run
  std::cout << "hector meets snoopy: " << meet(*hector, *snoopy) << "\n"; // wag tail
  std::cout << "hector meets flipper: " << meet(*hector, *flipper) << "\n"; // ignore
}
