// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <iostream>

#include "geometries.hpp"
#include "painter.hpp"

register_classes(
    geometries::Geometry, geometries::Shape, geometries::Square,
    geometries::Circle);

namespace painter {
namespace paint2d {

define_method_in(
    painters, paintObject,
    (Painter & painter, const geometries::Square& square), void) {
    next(painter, square);
    std::cout << "painting square\n";
}

define_method_in(
    painters, paintObject,
    (Painter & painter, const geometries::Circle& circle), void) {
    next(painter, circle);
    std::cout << "painting Circle\n";
}

} // namespace paint2d
} // namespace painter
