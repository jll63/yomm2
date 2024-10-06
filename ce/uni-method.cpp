#include <iostream>
#include <vector>
#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

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

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat);

BOOST_OPENMETHOD(poke, (virtual_<Animal&>, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (Cat & animal, std::ostream& os), void) {
    os << animal.name << " hisses.\n";
}

BOOST_OPENMETHOD_OVERRIDE(poke, (Dog & animal, std::ostream& os), void) {
    os << animal.name << " barks.\n";
}

void poke_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        poke(*animal, os);
    }
}

int main() {
    boost::openmethod::initialize();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};

    poke_animals(animals, std::cout);
}
