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

declare_method(poke, (virtual_<Animal&>, std::ostream&), void);

define_method(poke, (Cat & animal, std::ostream& os), void) {
    os << animal.name << " hisses.\n";
}

define_method(poke, (Dog & animal, std::ostream& os), void) {
    os << animal.name << " barks.\n";
}

void poke_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        poke(*animal, os);
    }
}

int main() {
    yorel::yomm2::initialize();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};

    poke_animals(animals, std::cout);
}
