// asteroids.cpp
// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example for Wikipedia

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

class Thing {
  public:
    virtual ~Thing() {
    }
};

class Asteroid : public Thing {};

class Spaceship : public Thing {};

register_classes(Thing, Spaceship, Asteroid);

declare_method(collideWith, (virtual_<Thing&>, virtual_<Thing&>), void);

define_method(collideWith, (Thing & left, Thing& right), void) {
    // default collision handling
}

define_method(collideWith, (Asteroid & left, Asteroid& right), void) {
    // handle Asteroid-Asteroid collision
}

define_method(collideWith, (Asteroid & left, Spaceship& right), void) {
    // handle Asteroid-Spaceship collision
}

define_method(collideWith, (Spaceship & left, Asteroid& right), void) {
    // handle Spaceship-Asteroid collision
}

define_method(collideWith, (Spaceship & left, Spaceship& right), void) {
    // handle Spaceship-Spaceship collision
}

int main() {
    yorel::yomm2::initialize();

    Asteroid a1, a2;
    Spaceship s1, s2;

    collideWith(a1, a2);
    collideWith(a1, s1);

    collideWith(s1, s2);
    collideWith(s1, a1);

    return 0;
}
