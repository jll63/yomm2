// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <iostream>

#include "geometries.hpp"
#include "shape_painter.hpp"

register_class(geometries::Shape, geometries::Geometry);
register_class(geometries::Square, geometries::Shape);
register_class(geometries::Circle, geometries::Shape);

namespace painter {
namespace paint2d {

define_method(
    painters,
    void, paintObject, (Painter& painter, const geometries::Square& square))
{
    method_definition(painters, void, (Painter&, const geometries::Shape&))(painter, square);
    std::cout << "painting square\n";
}

define_method(
    painters,
    void, paintObject, (Painter& painter, const geometries::Circle& circle))
{
    method_definition(painters, void, (Painter&, const geometries::Shape&))(painter, circle);
    std::cout << "painting Circle\n";
}

}
}
