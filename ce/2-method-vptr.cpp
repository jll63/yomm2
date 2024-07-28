#include <iostream>
#include <vector>
#include <yorel/yomm2/keywords.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Animal {
    const char* name;
    Animal(const char* name) : name(name) {
    }
    virtual ~Animal() {
    }
};

struct Dog : Animal {
    using Animal::Animal;
};

struct Cat : Animal {
    using Animal::Animal;
};

register_classes(Animal, Dog, Cat);

using yorel::yomm2::virtual_ptr;

declare_method(
    void, meet, (virtual_ptr<Animal>, virtual_ptr<Animal>, std::ostream&));

define_method(
    void, meet, (virtual_ptr<Cat> a1, virtual_ptr<Cat> a2, std::ostream& os)) {
    os << a1->name << " ignores " << a2->name << "\n";
}

define_method(
    void, meet, (virtual_ptr<Dog> a1, virtual_ptr<Cat> a2, std::ostream& os)) {
    os << a1->name << " chases " << a2->name << "\n";
}

define_method(
    void, meet, (virtual_ptr<Cat> a1, virtual_ptr<Dog> a2, std::ostream& os)) {
    os << a1->name << " runs away from " << a2->name << "\n";
}

define_method(
    void, meet, (virtual_ptr<Dog> a1, virtual_ptr<Dog> a2, std::ostream& os)) {
    os << a1->name << " wags tail at " << a2->name << "\n";
}

void meet_animals(const std::vector<virtual_ptr<Animal>>& animals, std::ostream& os) {
    for (auto animal : animals) {
        for (auto other : animals) {
            if (&animal != &other) {
                meet(animal, other, os);
            }
        }
    }
}

int main() {
    yorel::yomm2::update();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<virtual_ptr<Animal>> animals = {
        hector, felix, sylvester, snoopy};

    meet_animals(animals, std::cout);
}
