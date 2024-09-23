// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <iostream>

#include "painter.hpp"

register_classes(geometries::Shape, geometries::Geometry);

namespace painter {
namespace paint2d {

define_method(
    paintObject, (Painter & painter, const geometries::Shape& shape),
    void) {
    ++painter.counter;
    static int counter;
    ++counter;
    std::cout << "#" << painter.counter << " #" << counter << " ";
}

} // namespace paint2d
} // namespace painter
