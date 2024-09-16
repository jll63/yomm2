// Copyright (c) 2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#ifndef PAINTER_DEFINED
#define PAINTER_DEFINED

#include "geometries.hpp"

#include <yorel/yomm2.hpp>

namespace painter {

namespace paint1d {
method_container(painters);
}
namespace paint2d {
method_container(painters);
}

class Painter {
  public:
    void paint(const geometries::Geometry& geometry);
    int painted() const;

  private:
    int counter = 0;
    friend_method(paint1d::painters);
    friend_method(
        paint2d::painters, void, (Painter&, const geometries::Shape&));
};

// Implements paint
declare_method(
    void, paintObject,
    (Painter&, yorel::yomm2::virtual_<const geometries::Geometry&>));

inline void Painter::paint(const geometries::Geometry& geometry) {
    paintObject(*this, geometry);
}

inline int Painter::painted() const {
    return counter;
}

} // namespace painter

#endif
