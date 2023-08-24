#include <iostream>
#include <vector>
#include <yorel/yomm2/keywords.hpp>

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

declare_method(void, kick, (virtual_<Animal&>, std::ostream&));

define_method(void, kick, (Cat& animal, std::ostream& os)) {
    os << animal.name << " hisses.\n";
}

define_method(void, kick, (Dog& animal, std::ostream& os)) {
    os << animal.name << " barks.\n";
}

void kick_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        kick(*animal, os);
    }
}

int main() {
    yorel::yomm2::update();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};

    kick_animals(animals, std::cout);
}
