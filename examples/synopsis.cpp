#include <iostream>
#include <memory>

#include <yorel/yomm2/cute.hpp>

class Animal {
  public:
    virtual ~Animal() {}
};

class Dog     : public Animal {};
class Bulldog : public Dog {};
class Cat     : public Animal {};
class Dolphin : public Animal {};

using yorel::yomm2::virtual_;

register_class(Animal);
register_class(Dog, Animal);
register_class(Bulldog, Dog);
register_class(Cat, Animal);
register_class(Dolphin, Animal);

// open method with single argument <=> virtual function "from outside"
declare_method(std::string, kick, virtual_<Animal&>);

// implement 'kick' for dogs
begin_method(std::string, kick, Dog& dog) {
  return "bark";
} end_method;

// implement 'kick' for bulldogs
begin_method(std::string, kick, Bulldog& dog) {
    return next(dog) + " and bite";
} end_method;

// multi-method
declare_method(std::string, meet, virtual_<Animal&>, virtual_<Animal&>);

// 'meet' catch-all implementation
begin_method(std::string, meet, Animal&, Animal&) {
  return "ignore";
} end_method;

begin_method(std::string, meet, Dog& dog1, Dog& dog2) {
  return "wag tail";
} end_method;

begin_method(std::string, meet, Dog& dog, Cat& cat) {
  return "chase";
} end_method;

begin_method(std::string, meet, Cat& cat, Dog& dog) {
  return "run";
} end_method;

int main()
{
    yorel::yomm2::update_methods();

    std::unique_ptr<Animal>
        hector = std::make_unique<Bulldog>(),
        snoopy = std::make_unique<Dog>();
    std::cout << "kick snoopy: " << kick(*snoopy) << "\n"; // bark
    std::cout << "kick hector: " << kick(*hector) << "\n"; // bark and bite

    std::unique_ptr<Animal>
        sylvester = std::make_unique<Cat>(),
        flipper = std::make_unique<Dolphin>();
  std::cout << "hector meets sylvester: " << meet(*hector, *sylvester) << "\n"; // chase
  std::cout << "sylvester meets hector: " << meet(*sylvester, *hector) << "\n"; // chase
  std::cout << "hector meets snoopy: " << meet(*hector, *snoopy) << "\n"; // wag tail
  std::cout << "hector meets flipper: " << meet(*hector, *flipper) << "\n"; // ignore
}
