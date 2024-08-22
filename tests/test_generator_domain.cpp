#include "test_generator_domain.hpp"

register_classes(Animal, Cat, Dog, Property, DomesticCat, DomesticDog);

define_method(void, kick, (Dog&, std::ostream& os)) {
    os << "bark";
}

define_method(void, kick, (Cat&, std::ostream& os)) {
    os << "hiss";
}

// create an ambiguity with the following two definitions
define_method(void, meet, (Cat&, Animal&, std::ostream& os)) {
    os << "ignore";
}

define_method(void, meet, (Animal&, Dog&, std::ostream& os)) {
    os << "wag tail";
}
// ^^^ ambiguous

define_method(void, meet, (Dog&, Cat&, std::ostream& os)) {
    os << "chase";
}


define_method(void, identify, (DomesticCat & animal, std::ostream& os)) {
    os << animal.owner << "'s"
       << " cat";
}

define_method(void, identify, (DomesticDog & animal, std::ostream& os)) {
    os << animal.owner << "'s"
       << " dog";
}
