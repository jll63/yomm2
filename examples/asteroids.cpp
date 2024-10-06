// asteroids.cpp
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example for Wikipedia

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

class Thing {
  public:
    virtual ~Thing() {
    }
};

class Asteroid : public Thing {};

class Spaceship : public Thing {};

BOOST_OPENMETHOD_CLASSES(Thing, Spaceship, Asteroid);

BOOST_OPENMETHOD(collideWith, (virtual_<Thing&>, virtual_<Thing&>), void);

BOOST_OPENMETHOD_OVERRIDE(collideWith, (Thing & left, Thing& right), void) {
    // default collision handling
}

BOOST_OPENMETHOD_OVERRIDE(
    collideWith, (Asteroid & left, Asteroid& right), void) {
    // handle Asteroid-Asteroid collision
}

BOOST_OPENMETHOD_OVERRIDE(
    collideWith, (Asteroid & left, Spaceship& right), void) {
    // handle Asteroid-Spaceship collision
}

BOOST_OPENMETHOD_OVERRIDE(
    collideWith, (Spaceship & left, Asteroid& right), void) {
    // handle Spaceship-Asteroid collision
}

BOOST_OPENMETHOD_OVERRIDE(
    collideWith, (Spaceship & left, Spaceship& right), void) {
    // handle Spaceship-Spaceship collision
}

int main() {
    boost::openmethod::initialize();

    Asteroid a1, a2;
    Spaceship s1, s2;

    collideWith(a1, a2);
    collideWith(a1, s1);

    collideWith(s1, s2);
    collideWith(s1, a1);

    return 0;
}
