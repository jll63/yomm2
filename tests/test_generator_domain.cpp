#include "test_generator_domain.hpp"

register_classes(Animal, Cat, Dog, Property, DomesticCat, DomesticDog);

define_method(kick, (Dog&, std::ostream& os), void) {
    os << "bark";
}

define_method(kick, (Cat&, std::ostream& os), void) {
    os << "hiss";
}

// create an ambiguity with the following two definitions
define_method(meet, (Cat&, Animal&, std::ostream& os), void) {
    os << "ignore";
}

define_method(meet, (Animal&, Dog&, std::ostream& os), void) {
    os << "wag tail";
}
// ^^^ ambiguous

define_method(meet, (Dog&, Cat&, std::ostream& os), void) {
    os << "chase";
}

define_method(identify, (DomesticCat & animal, std::ostream& os), void) {
    os << animal.owner << "'s"
       << " cat";
}

define_method(identify, (DomesticDog & animal, std::ostream& os), void) {
    os << animal.owner << "'s"
       << " dog";
}
