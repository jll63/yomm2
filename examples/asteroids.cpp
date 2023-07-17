// asteroids.cpp
// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example for Wikipedia

#include <yorel/yomm2/keywords.hpp>

class Thing {
  public:
    virtual ~Thing() {}
};

class Asteroid : public Thing {
};

class Spaceship : public Thing {
};

register_classes(Thing, Spaceship, Asteroid);

declare_method(void, collideWith, (virtual_<Thing&>, virtual_<Thing&>));

define_method(void, collideWith, (Thing& left, Thing& right)) {
    // default collision handling
}

define_method(void, collideWith, (Asteroid& left, Asteroid& right)) {
    // handle Asteroid-Asteroid collision
}

define_method(void, collideWith, (Asteroid& left, Spaceship& right)) {
    // handle Asteroid-Spaceship collision
}

define_method(void, collideWith, (Spaceship& left, Asteroid& right)) {
    // handle Spaceship-Asteroid collision
}

define_method(void, collideWith, (Spaceship& left, Spaceship& right)) {
    // handle Spaceship-Spaceship collision
}

int main() {
    yorel::yomm2::update();

    Asteroid  a1, a2;
    Spaceship s1, s2;

    collideWith(a1, a2);
    collideWith(a1, s1);

    collideWith(s1, s2);
    collideWith(s1, a1);

    return 0;
}
