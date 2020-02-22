// Copyright (c) 2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <iostream>

#include <yorel/yomm2/cute.hpp>

#include "geometries.hpp"
#include "line_painter.hpp"

register_class(geometries::Segment, geometries::Line);

namespace painter {
namespace paint1d {

define_method(
    painters,
    void, paintObject, (Painter& painter, const geometries::Segment& segment))
{
    ++painter.counter;
    method_definition(
        painters, void, (Painter&, const geometries::Line&))(
            painter, segment);
    std::cout << " " << "painting segment\n";
}

}
}
