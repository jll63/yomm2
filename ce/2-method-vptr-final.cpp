#include <iostream>
#include <vector>
#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

struct Animal {
    const char* name;
    Animal(const char* name) : name(name) {
    }
};

struct Dog : Animal {
    using Animal::Animal;
};

struct Cat : Animal {
    using Animal::Animal;
};

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat);

using boost::openmethod::virtual_ptr;

BOOST_OPENMETHOD(
    meet, (virtual_ptr<Animal>, virtual_ptr<Animal>, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(
    meet, (virtual_ptr<Cat> a1, virtual_ptr<Cat> a2, std::ostream& os), void) {
    os << a1->name << " ignores " << a2->name << "\n";
}

BOOST_OPENMETHOD_OVERRIDE(
    meet, (virtual_ptr<Dog> a1, virtual_ptr<Cat> a2, std::ostream& os), void) {
    os << a1->name << " chases " << a2->name << "\n";
}

BOOST_OPENMETHOD_OVERRIDE(
    meet, (virtual_ptr<Cat> a1, virtual_ptr<Dog> a2, std::ostream& os), void) {
    os << a1->name << " runs away from " << a2->name << "\n";
}

BOOST_OPENMETHOD_OVERRIDE(
    meet, (virtual_ptr<Dog> a1, virtual_ptr<Dog> a2, std::ostream& os), void) {
    os << a1->name << " wags tail at " << a2->name << "\n";
}

void meet_animals(
    const std::vector<virtual_ptr<Animal>>& animals, std::ostream& os) {
    for (auto animal : animals) {
        for (auto other : animals) {
            if (&animal != &other) {
                meet(animal, other, os);
            }
        }
    }
}

int main() {
    boost::openmethod::initialize();

    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<virtual_ptr<Animal>> animals = {
        virtual_ptr<Dog>::final(hector), virtual_ptr<Cat>::final(felix),
        virtual_ptr<Cat>::final(sylvester), virtual_ptr<Dog>::final(snoopy)};

    meet_animals(animals, std::cout);
}
