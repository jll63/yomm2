#include "animals.hpp"

#include <iostream>

Animal::~Animal() {
}

register_classes(Animal, Dog, Cat);

define_method(poke, (virtual_ptr<Cat>), void) {
    std::cout << "hiss\n";
}

define_method(poke, (virtual_ptr<Dog>), void) {
    std::cout << "bark\n";
}

// 'meet' catch-all implementation.
define_method(meet, (virtual_ptr<Animal>, virtual_ptr<Animal>), void) {
    std::cout << "ignore\n";
}

// Add definitions for specific pairs of animals.
define_method(meet, (virtual_ptr<Dog> dog1, virtual_ptr<Dog> dog2), void) {
    std::cout << "wag tail\n";
}

define_method(meet, (virtual_ptr<Dog> dog, virtual_ptr<Cat> cat), void) {
    std::cout << "chase\n";
}

define_method(meet, (virtual_ptr<Cat> cat, virtual_ptr<Dog> dog), void) {
    std::cout << "run\n";
}
