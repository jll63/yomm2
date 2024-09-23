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

class Painter;

// Implements paint
declare_method(
    paintObject,
    (Painter&, yorel::yomm2::virtual_<const geometries::Geometry&>), void);

namespace paint1d {
template<typename...>
struct YOMM2_OVERRIDERS(paintObject);
}

namespace paint2d {
template<typename...>
struct YOMM2_OVERRIDERS(paintObject);
}

class Painter {
  public:
    void paint(const geometries::Geometry& geometry);
    int painted() const;

  private:
    int counter = 0;
    template<typename...>
    friend struct paint1d::YOMM2_OVERRIDERS(paintObject);
    friend struct paint2d::YOMM2_OVERRIDERS(
        paintObject)<void(Painter&, const geometries::Shape&)>;
};

inline void Painter::paint(const geometries::Geometry& geometry) {
    paintObject(*this, geometry);
}

inline int Painter::painted() const {
    return counter;
}

} // namespace painter

#endif
