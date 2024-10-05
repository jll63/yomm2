#include <iostream>
#include <vector>

struct Animal {
    const char* name;
    Animal(const char* name) : name(name) {
    }
    virtual ~Animal() {
    }
    virtual void poke(std::ostream& os) = 0;
};

struct Dog : Animal {
    using Animal::Animal;
    void poke(std::ostream& os) override {
        os << name << " barks.\n";
    }
};

struct Cat : Animal {
    using Animal::Animal;
    void poke(std::ostream& os) override {
        os << name << " hisses.\n";
    }
};

void poke_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        animal->poke(os);
    }
}

int main() {
    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};
    poke_animals(animals, std::cout);
}
