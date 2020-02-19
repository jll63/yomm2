// dl.hpp
// Copyright (c) 2018-2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DL_DEFINED
#define DL_DEFINED

#include <string>

#include <yorel/yomm2.hpp>

struct Animal {
  virtual ~Animal() {
  }
};

YOMM2_CLASS(Animal);

struct Herbivore : Animal {
};

YOMM2_CLASS(Herbivore, Animal);

struct Carnivore : Animal {
};

YOMM2_CLASS(Carnivore, Animal);

struct Cow : Herbivore {
};

YOMM2_CLASS(Cow, Herbivore);

struct Wolf : Carnivore {
};

YOMM2_CLASS(Wolf, Carnivore);

YOMM2_DECLARE(
    std::string,
    encounter,
    (yorel::yomm2::virtual_<const Animal&>, yorel::yomm2::virtual_<const Animal&>));

#endif
