// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

namespace interfaces {
class Animal {
  public:
    virtual ~Animal() {
    }
};
} // namespace interfaces

namespace canis {
class Dog : public interfaces::Animal {};
class Bulldog : public Dog {};
} // namespace canis

namespace felis {
class Cat : public interfaces::Animal {};
} // namespace felis

namespace delphinus {
class Dolphin : public interfaces::Animal {};
} // namespace delphinus

#include <string>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

using yorel::yomm2::virtual_;

register_classes(
    interfaces::Animal, canis::Dog, canis::Bulldog, felis::Cat,
    delphinus::Dolphin);

// open method with single virtual argument <=> virtual function "from outside"
declare_method(poke, (virtual_<interfaces::Animal&>), std::string);

namespace canis {
// implement 'poke' for dogs
define_method(poke, (Dog & dog), std::string) {
    return "bark";
}

// implement 'poke' for bulldogs
define_method(poke, (Bulldog & dog), std::string) {
    return next(dog) + " and bite";
}
} // namespace canis

// multi-method
declare_method(
    meet, (virtual_<interfaces::Animal&>, virtual_<interfaces::Animal&>),
    std::string);

// 'meet' catch-all implementation
define_method(meet, (interfaces::Animal&, interfaces::Animal&), std::string) {
    return "ignore";
}

define_method(meet, (canis::Dog & dog1, canis::Dog& dog2), std::string) {
    return "wag tail";
}

define_method(meet, (canis::Dog & dog, felis::Cat& cat), std::string) {
    return "chase";
}

define_method(meet, (felis::Cat & cat, canis::Dog& dog), std::string) {
    return "run";
}

// -----------------------------------------------------------------------------
// main

#include <iostream>
#include <memory>

int main() {
    yorel::yomm2::initialize();

    std::unique_ptr<interfaces::Animal> hector =
                                            std::make_unique<canis::Bulldog>(),
                                        snoopy = std::make_unique<canis::Dog>();

    std::cout << "poke snoopy: " << poke(*snoopy) << "\n"; // bark
    std::cout << "poke hector: " << poke(*hector) << "\n"; // bark and bite

    std::unique_ptr<interfaces::Animal>
        sylvester = std::make_unique<felis::Cat>(),
        flipper = std::make_unique<delphinus::Dolphin>();

    std::cout << "hector meets sylvester: " << meet(*hector, *sylvester)
              << "\n"; // chase
    std::cout << "sylvester meets hector: " << meet(*sylvester, *hector)
              << "\n"; // run
    std::cout << "hector meets snoopy: " << meet(*hector, *snoopy)
              << "\n"; // wag tail
    std::cout << "hector meets flipper: " << meet(*hector, *flipper)
              << "\n"; // ignore
}
