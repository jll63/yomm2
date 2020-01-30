// Copyright (c) 2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#include <yorel/yomm2/cute.hpp>

#include <iostream>
#include <memory>
#include <string>

using yorel::yomm2::virtual_;
using std::cout;

class Geometry
{
public:
    virtual ~Geometry() {}
};

class Arc : public Geometry
{
public:
    virtual ~Arc() {}
};

class Edge : public Geometry
{
public:
    virtual ~Edge() {}
};

register_class(Geometry);
register_class(Arc, Geometry);
register_class(Edge, Geometry);

class Painter
{
public:
    void paint(const Geometry& geometry);
private:
    int counter = 0;
    friend_named_method(paint_geo);
};

// Implements paint
declare_method(void, paintObject, (Painter&, virtual_<const Geometry&>));

// Catch-all paint(Geometry)
define_named_method(
    paint_geo,
    void, paintObject, (Painter& painter, const Geometry& /*geometry*/)) {
    std::cout << "painting geometry (" << painter.counter << ")\n";
    ++painter.counter;
}

// Specific paint(Arc)
define_method(void, paintObject, (Painter& painter, const Arc& arc)) {
    //next(painter, arc);
    // also provides a way of calling a specific override:
    call_named_method(paint_geo, (painter, arc));
    std::cout << "    painting arc\n";
}

// Specific paint(Edge)
define_method(void, paintObject, (Painter& painter, const Edge& edge)) {
    next(painter, edge);
    std::cout << "    painting edge\n";
}

void Painter::paint(const Geometry& geometry)
{
    paintObject(*this, geometry);
}

int main() {
    yorel::yomm2::update_methods();

    Arc arc;
    Edge edge;
    Painter painter;

    painter.paint(arc);
    painter.paint(edge);
}
