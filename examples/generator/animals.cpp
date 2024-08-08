#include "animals.hpp"

#include <iostream>

Animal::~Animal() {
}

register_classes(Animal, Dog, Cat);

define_method(void, kick, (virtual_ptr<Cat>)) {
    std::cout << "hiss\n";
}

define_method(void, kick, (virtual_ptr<Dog>)) {
    std::cout << "bark\n";
}

define_method(void, pet, (virtual_ptr<Cat>)) {
    std::cout << "purr\n";
}

define_method(void, pet, (virtual_ptr<Dog>)) {
    std::cout << "wag tail\n";
}

// 'meet' catch-all implementation.
define_method(void, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>)) {
    std::cout << "ignore\n";
}

// Add definitions for specific pairs of animals.
define_method(void, meet, (virtual_ptr<Dog> dog1, virtual_ptr<Dog> dog2)) {
    std::cout << "wag tail\n";
}

define_method(void, meet, (virtual_ptr<Dog> dog, virtual_ptr<Cat> cat)) {
    std::cout << "chase\n";
}

define_method(void, meet, (virtual_ptr<Cat> cat, virtual_ptr<Dog> dog)) {
    std::cout << "run\n";
}

// 'mate' catch-all implementation.
define_method(void, mate, (virtual_ptr<Animal>, virtual_ptr<Animal>)) {
    std::cout << "error C1001\n";
}

// Add definitions for specific pairs of animals.
define_method(void, mate, (virtual_ptr<Dog> dog1, virtual_ptr<Dog> dog2)) {
    std::cout << "puppy\n";
}

// Add definitions for specific pairs of animals.
define_method(void, mate, (virtual_ptr<Cat> cat1, virtual_ptr<Cat> cat2)) {
    std::cout << "kitten\n";
}
