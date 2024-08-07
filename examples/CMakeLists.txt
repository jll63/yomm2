# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(README README.cpp)
target_link_libraries(README YOMM2::yomm2)
add_test(NAME README COMMAND README)

add_executable(slides slides.cpp)
target_link_libraries(slides YOMM2::yomm2)
add_test(NAME slides COMMAND slides)

add_executable(synopsis synopsis.cpp)
target_link_libraries(synopsis YOMM2::yomm2)
add_test(NAME synopsis COMMAND synopsis)

add_executable(matrix matrix.cpp)
target_link_libraries(matrix YOMM2::yomm2)
add_test(NAME matrix COMMAND matrix)

add_executable(accept_no_visitors accept_no_visitors.cpp)
target_link_libraries(accept_no_visitors YOMM2::yomm2)
add_test(NAME accept_no_visitors COMMAND accept_no_visitors)

add_executable(adventure adventure.cpp)
target_link_libraries(adventure YOMM2::yomm2)
add_test(NAME adventure COMMAND adventure)

add_executable(next next.cpp)
target_link_libraries(next YOMM2::yomm2)
add_test(NAME next COMMAND next)

add_executable(asteroids asteroids.cpp)
target_link_libraries(asteroids YOMM2::yomm2)
add_test(NAME asteroids COMMAND asteroids)

add_subdirectory(containers)
add_test(NAME containers COMMAND containers)

if (NOT (WIN32 OR APPLE))
  message(STATUS "Building dlopen example.")
  add_executable(dl_main dl_main.cpp)
  add_library(dl_shared SHARED dl_shared.cpp)
  get_target_property(YOMM2_INCLUDE_DIRS YOMM2::yomm2 INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(dl_shared PUBLIC ${YOMM2_INCLUDE_DIRS})
  add_dependencies(dl_main dl_shared)
  set_target_properties(dl_main PROPERTIES LINK_FLAGS "-Wl,-export-dynamic")
  target_link_libraries(dl_main YOMM2::yomm2 dl)
  target_link_libraries(dl_shared YOMM2::yomm2)
  add_test(NAME dlopen COMMAND dl_main)
endif()

if (NOT (MSVC AND YOMM2_SHARED))
  # Running this example with a Windows DLL is too much of a hassle, because we
  # would need to add the path to the directory containing yomm2.dll to PATH.
  # Anyway, if it works with static linking, it is very unlikely that it fails
  # with the runtime in a DLL.
  add_subdirectory(generator)
endif()
