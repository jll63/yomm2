#include <iostream>
#include <vector>

struct Cat;
struct Dog;

struct Animal {
    const char* name;
    Animal(const char* name) : name(name) {
    }
    virtual ~Animal() {
    }
    virtual void meet(Animal& other, std::ostream& os) = 0;
    virtual void meet_cat(Cat& other, std::ostream& os) = 0;
    virtual void meet_dog(Dog& other, std::ostream& os) = 0;
};

struct Cat : Animal {
    using Animal::Animal;
    void meet(Animal& other, std::ostream& os) override;
    void meet_cat(Cat& other, std::ostream& os) override;
    void meet_dog(Dog& other, std::ostream& os) override;
};

struct Dog : Animal {
    using Animal::Animal;
    void meet(Animal& other, std::ostream& os) override;
    void meet_cat(Cat& other, std::ostream& os) override;
    void meet_dog(Dog& other, std::ostream& os) override;
};

void meet_animals(const std::vector<Animal*>& animals, std::ostream& os) {
    for (auto animal : animals) {
        for (auto other : animals) {
            if (animal != other) {
                animal->meet(*other, os);
            }
        }
    }
}

int main() {
    Dog hector{"Hector"}, snoopy{"Snoopy"};
    Cat felix{"Felix"}, sylvester{"Sylvester"};
    std::vector<Animal*> animals = {&hector, &felix, &sylvester, &snoopy};
    meet_animals(animals, std::cout);
}

void Cat::meet(Animal& other, std::ostream& os) {
    other.meet_cat(*this, os);
}

void Cat::meet_cat(Cat& other, std::ostream& os) {
    os << name << " ignores " << other.name << "\n";
}

void Cat::meet_dog(Dog& other, std::ostream& os) {
    os << name << " runs away from " << other.name << "\n";
}

void Dog::meet(Animal& other, std::ostream& os) {
    other.meet_dog(*this, os);
}
void Dog::meet_cat(Cat& other, std::ostream& os) {
    os << name << " chases " << other.name << "\n";
}
void Dog::meet_dog(Dog& other, std::ostream& os) {
    os << name << " wags tail at " << other.name << "\n";
}
