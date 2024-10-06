# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(slides slides.cpp)
target_link_libraries(slides Boost::openmethod)
add_test(NAME slides COMMAND slides)

add_executable(synopsis synopsis.cpp)
target_link_libraries(synopsis Boost::openmethod)
add_test(NAME synopsis COMMAND synopsis)

add_executable(matrix matrix.cpp)
target_link_libraries(matrix Boost::openmethod)
add_test(NAME matrix COMMAND matrix)

add_executable(accept_no_visitors accept_no_visitors.cpp)
target_link_libraries(accept_no_visitors Boost::openmethod)
add_test(NAME accept_no_visitors COMMAND accept_no_visitors)

add_executable(adventure adventure.cpp)
target_link_libraries(adventure Boost::openmethod)
add_test(NAME adventure COMMAND adventure)

add_executable(next next.cpp)
target_link_libraries(next Boost::openmethod)
add_test(NAME next COMMAND next)

add_executable(asteroids asteroids.cpp)
target_link_libraries(asteroids Boost::openmethod)
add_test(NAME asteroids COMMAND asteroids)
