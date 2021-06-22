// dl_shared.cpp
// Copyright (c) 2018-2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <yorel/yomm2.hpp>

#include "dl.hpp"

using namespace std;
using yorel::yomm2::virtual_;

YOMM2_DEFINE(string, encounter, (const Herbivore&, const Carnivore&)) {
  return "run";
}

struct Tiger : Carnivore {
};

YOMM2_CLASS(Tiger, Carnivore);

extern "C" Tiger* make_tiger() {
  return new Tiger;
}

YOMM2_DEFINE(string, encounter, (const Carnivore&, const Herbivore&)) {
  return "hunt";
}
