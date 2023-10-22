# Copyright (c) 2018-2022 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
  yomm2
  GIT_REPOSITORY https://github.com/jll63/yomm2.git
)

# After the following call, the CMake targets defined by YOMM2 will be available
# to the rest of the build
FetchContent_MakeAvailable(yomm2)

find_package(YOMM2)

add_executable(adventure adventure.cpp)
target_link_libraries(adventure YOMM2::yomm2)
