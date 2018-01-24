# Copyright (c) 2018 Jean-Louis Leroy
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
