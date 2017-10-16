# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

add_library(yomm2 yomm2.cpp)

INSTALL(TARGETS yomm2
  DESTINATION lib
)
