// dl.hpp
// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DL_DEFINED
#define DL_DEFINED

#include <string>

#include <yorel/yomm2.hpp>
#include <yorel/yomm2/compiler.hpp>

struct Animal {
    virtual ~Animal() {
    }
};

register_classes(Animal);

struct Herbivore : Animal {};

register_classes(Herbivore, Animal);

struct Carnivore : Animal {};

register_classes(Carnivore, Animal);

struct Cow : Herbivore {};

register_classes(Cow, Herbivore);

struct Wolf : Carnivore {};

register_classes(Wolf, Carnivore);

declare_method(
    encounter, (virtual_<const Animal&>, virtual_<const Animal&>), std::string);

#endif
