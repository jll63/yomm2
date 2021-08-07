// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <yorel/yomm2/cute.hpp>

#include <iostream>

#include "geometries.hpp"
#include "painter.hpp"

using yorel::yomm2::virtual_;
using std::cout;

int main() {
    yorel::yomm2::update_methods();

    const geometries::Geometry& arc = geometries::Arc();
    const geometries::Geometry& segment = geometries::Segment();
    const geometries::Geometry& square = geometries::Square();
    const geometries::Geometry& circle = geometries::Circle();

    painter::Painter painter;
    painter.paint(arc);
    painter.paint(segment);
    painter.paint(square);
    painter.paint(circle);
}
