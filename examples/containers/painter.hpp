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

#define YOMM2_FWD_OVERRIDER(RETURN_TYPE, NAME, ARGS)                           \
    template<typename>                                                         \
    struct NAME##_overriders;                                                  \
    template<>                                                                 \
    struct NAME##_overriders<RETURN_TYPE ARGS>

namespace painter {

class Painter;

namespace paint1d {

template<typename>
struct paintObject_overriders;

} // namespace paint1d

namespace paint2d {

YOMM2_FWD_OVERRIDER(void, paintObject, (Painter&, const geometries::Shape&));

}

// Implements paint
declare_method(
    void, paintObject,
    (Painter&, yorel::yomm2::virtual_<const geometries::Geometry&>));

class Painter {
  public:
    void paint(const geometries::Geometry& geometry);
    int painted() const;

  private:
    int counter = 0;
    template<typename>
    friend struct paint1d::paintObject_overriders;
    friend paint2d::paintObject_overriders<void(
        Painter&, const geometries::Shape&)>;
};

inline void Painter::paint(const geometries::Geometry& geometry) {
    paintObject(*this, geometry);
}

inline int Painter::painted() const {
    return counter;
}

} // namespace painter

#endif
