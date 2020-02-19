# Copyright (c) 2018-2020 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(synopsis synopsis.cpp)
target_link_libraries (synopsis yomm2)
add_test (synopsis synopsis)

add_executable(matrix matrix.cpp)
target_link_libraries (matrix yomm2)
add_test (matrix matrix)

add_executable(accept_no_visitors accept_no_visitors.cpp)
target_link_libraries (accept_no_visitors yomm2)
add_test (accept_no_visitors accept_no_visitors)

add_executable(adventure adventure.cpp)
target_link_libraries (adventure yomm2)
add_test (adventure adventure)

add_executable(next next.cpp)
target_link_libraries (next yomm2)
add_test (next next)

add_executable(asteroids asteroids.cpp)
target_link_libraries (asteroids yomm2)
add_test (asteroids asteroids)

add_executable(friendship friendship.cpp)
target_link_libraries (friendship yomm2)
add_test (friendship friendship)

if(NOT MSVC)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
  add_executable(dl_main dl_main.cpp)
  add_library(dl_shared SHARED dl_shared.cpp)
  add_dependencies(dl_main dl_shared)
  if(APPLE)
    set_target_properties(dl_main PROPERTIES LINK_FLAGS "-Wl,-export_dynamic")
    set_target_properties(dl_shared PROPERTIES LINK_FLAGS "-Wl,-undefined,dynamic_lookup")
  endif()
  target_link_libraries (dl_main yomm2 dl)
endif()
