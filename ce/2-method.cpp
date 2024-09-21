#include <iostream>
#include <vector>
#include <yorel/yomm2.hpp>
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

declare_method(
    meet, (virtual_<Animal&>, virtual_<Animal&>, std::ostream&), void);

define_method(meet, (Cat & a1, Cat& a2, std::ostream& os), void) {
    os << a1.name << " ignores " << a2.name << "\n";
}

define_method(meet, (Dog & a1, Cat& a2, std::ostream& os), void) {
    os << a1.name << " chases " << a2.name << "\n";
}

define_method(meet, (Cat & a1, Dog& a2, std::ostream& os), void) {
    os << a1.name << " runs away from " << a2.name << "\n";
}

define_method(meet, (Dog & a1, Dog& a2, std::ostream& os), void) {
    os << a1.name << " wags tail at " << a2.name << "\n";
}

void meet_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        for (auto other : animals) {
            if (&animal != &other) {
                meet(*animal, *other, os);
            }
        }
    }
}

int main() {
    yorel::yomm2::initialize();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};

    meet_animals(animals, std::cout);
}
