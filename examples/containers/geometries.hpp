// Copyright (c) 2020 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This exmaple is based on sample code provided by Github user matpen in
// https://github.com/jll63/yomm2/issues/7

#ifndef GEOMETRY_DEFINED
#define GEOMETRY_DEFINED

namespace geometries {

class Geometry
{
public:
    virtual ~Geometry() {}
};

class Line: public Geometry {
};

class Arc : public Line {
};

class Segment : public Line {
};

class Shape : public Geometry {
};

class Square : public Shape {
};

class Circle : public Shape {
};

} // namespace geometries

#endif
