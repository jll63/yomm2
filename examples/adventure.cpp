// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Character {
    virtual ~Character() {
    }
};

struct Warrior : Character {};

struct Device {
    virtual ~Device() {
    }
};

struct Hands : Device {};
struct Axe : Device {};
struct Banana : Device {};

struct Creature {
    virtual ~Creature() {
    }
};

struct Dragon : Creature {};
struct Bear : Creature {};

register_classes(
    Character, Warrior, Device, Hands, Axe, Banana, Creature, Dragon, Bear);

declare_method(
    fight, (virtual_<Character&>, virtual_<Creature&>, virtual_<Device&>),
    std::string);

define_method(fight, (Character & x, Creature& y, Banana& z), std::string) {
    return "are you insane?";
}

define_method(fight, (Character & x, Creature& y, Axe& z), std::string) {
    return "not agile enough to wield";
}

define_method(fight, (Warrior & x, Creature& y, Axe& z), std::string) {
    return "and cuts it into pieces";
}

define_method(fight, (Warrior & x, Dragon& y, Axe& z), std::string) {
    return "and dies a honorable death";
}

define_method(fight, (Character & x, Dragon& y, Hands& z), std::string) {
    return "Congratulations! You have just vainquished a dragon with your bare "
           "hands"
           " (unbelievable, isn't it?)";
}

int main() {
    yorel::yomm2::initialize();

    std::unique_ptr<Character> bob = std::make_unique<Character>(),
                               rambo = std::make_unique<Warrior>();

    std::unique_ptr<Creature> elliott = std::make_unique<Dragon>(),
                              paddington = std::make_unique<Bear>();

    std::unique_ptr<Device> hands = std::make_unique<Hands>(),
                            axe = std::make_unique<Axe>(),
                            chiquita = std::make_unique<Banana>();

    std::cout << "bob fights elliot with axe:\n"
              << fight(*bob, *elliott, *axe) << "\n";
    // bob fights elliot with axe:
    // not agile enough to wield

    std::cout << "rambo fights paddington with axe:\n"
              << fight(*rambo, *paddington, *axe) << "\n";
    // rambo fights paddington with axe:
    // and cuts it into pieces

    std::cout << "rambo fights paddington with banana:\n"
              << fight(*rambo, *paddington, *chiquita) << "\n";
    // rambo fights paddington with banana:
    // are you insane?

    std::cout << "rambo fights elliott with axe:\n"
              << fight(*rambo, *elliott, *axe) << "\n";
    // rambo fights elliott with axe:
    // and dies a honorable death

    std::cout << "bob fights elliot with hands:\n"
              << fight(*bob, *elliott, *hands) << "\n";
    // bob fights elliot with hands: Congratulations! You have just vainquished
    // a dragon with your bare hands (unbelievable, isn't it?)

    std::cout << "rambo fights elliot with hands:\n"
              << fight(*rambo, *elliott, *hands) << "\n";
    // rambo fights elliot with hands:
    // you just killed a dragon with your bare hands. Incredible isn't it?

    return 0;
}

auto call_fight(Character& character, Creature& creature, Device& device) {
    return fight(character, creature, device);
}
