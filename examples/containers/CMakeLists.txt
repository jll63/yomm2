# Copyright (c) 2018-2021 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(
  containers
  main.cpp
  shape_painter.cpp concrete_shape_painters.cpp
  line_painter.cpp arc_painter.cpp segment_painter.cpp
  painter.cpp)
target_link_libraries(containers YOMM2::yomm2)
