#include <iostream>
#include <vector>

struct Animal {
    const char* name;
    Animal(const char* name) : name(name) {
    }
    virtual ~Animal() {
    }
    virtual void kick(std::ostream& os) = 0;
};

struct Dog : Animal {
    using Animal::Animal;
    void kick(std::ostream& os) override {
        os << name << " barks.\n";
    }
};

struct Cat : Animal {
    using Animal::Animal;
    void kick(std::ostream& os) override {
        os << name << " hisses.\n";
    }
};

void kick_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        animal->kick(os);
    }
}

int main() {
    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};
    kick_animals(animals, std::cout);
}
