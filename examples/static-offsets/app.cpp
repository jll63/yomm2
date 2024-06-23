#include "animals.hpp"

#include <iostream>

void call_kick(virtual_ptr<Animal> animal) {
  kick(animal);
}

void call_meet(virtual_ptr<Animal> a1, virtual_ptr<Animal> a2) {
  meet(a1, a2);
}

void call_vf(Animal& a) {
    a.kick();
}

int main() {
  yorel::yomm2::update();

  Cat felix;
  virtual_ptr<Animal> cat = felix;
  Dog snoopy;
  virtual_ptr<Animal> dog = snoopy;

  kick(cat);
  kick(dog);
  meet(cat, cat);
  meet(cat, dog);
  meet(dog, cat);
  meet(dog, dog);

  return 0;
}
