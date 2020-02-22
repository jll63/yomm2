# Method Containers, Namespaces, Inline and Friendship

This example shows how to use method containers to grant friendship to method
definitions, and access method defintions across translation units and
namespaces.

Header [`geometries.hpp`](geometries.hpp) defines a class hierarchy in
namespace `geometries`, representing 1D and 2D geometrical entities. Its
structure is as follows:

```
header geometries.hpp
  namespace geometries
    class Geometry
      class Line
        class Arc
        class Segment
      class Shape
        class Square
        class Circle
```

Header [`painter.hpp`](painter.hpp) defines a mechanism for geometry
objects. It consists of a `Painter` class, a `paintObject` open method, two
container declarations inside two nested namespaces, respectively for
1-dimensional and 2-dimensional entities.

Its structure is as follows:

```
painter.hpp
  namespace painter
    namespace paint1d
      method container painters
    namespace paint2d
      method container painters
    class Painter
      friend declaration for all methods defined in paint1d::painters
      friend declaration for the method defined in paint2d::painters taking a Shape
    declaration of open method paintObject
```

Note that we are using *two* distinct containers, both called `painters`, but
one is in namespace `paint1d` and the other in namespace `paint2d`.

[`line_painter.hpp`](line_painter.hpp) defines an *inline* implementation of
`paintObject` for `Line`:

```
header line_painter.hpp
  namespace painter
    namespace paint1d
      define paintObject for `Line`, making it inline
```

[`segment_painter.hpp`](segment_painter.hpp) implements `paintObject` for
`Segment`, calling the base method for `Line` in the process:

```
translation unit segment_painter.cpp
  namespace painter
    namespace paint1d
      define paintObject for `Segment`
        call inline `paintObject` for `Line` from container `paint1d::painters`
```

[`arc_painter.cpp`](arc_painter.cpp) does the same for `Arc`.

Note that all three method definitions (for `Line`, `Segment` and `Arc`) can
access the private parts of `Painter`, because they are defined inside a
container that was granted friendship as a whole.

Also note that the `next` mechanism is not used, instead the method definition for
`Line` is always used.

[`shape_painter.hpp`](shape_painter.hpp) declares (but does not define) that
container `paint2d::painters` contains a `paintObject` method for `Shape`:

```
header shape_painter.hpp
  namespace painter
    namespace paint2d
      declare method container `painters` and inside it, `paintObject(Painter, Shape)`
```

[`shape_painter.cpp`](shape_painter.cpp) defines the said method:

```
translation unit shape_painter.cpp
  namespace painter
    namespace paint2d
      define `paintObject(Painter, Shape)` inside method container `painters`
```

Note that this method definition is a friend of `Painter`, and thus can access
its private parts.

Finally, [`concrete_shape_painters.cpp`](concrete_shape_painters.cpp) defines
`paintObject` for `Square` and `Circle`:

```
translation unit shape_painter.cpp
  namespace painter
    namespace paint2d
      define `paintObject(Painter, Square)` inside method container `painters`
      define `paintObject(Painter, Circle)` inside method container `painters`
```

Both call base method `paintObject(Painter, Shape)` declared in
[`shape_painter.hpp`](shape_painter.hpp) and defined in
[`shape_painter.cpp`](shape_painter.cpp).
