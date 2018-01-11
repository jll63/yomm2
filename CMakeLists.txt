# -*- compile-command: "cd build && make && make test" -*-

# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

# for clang: cmake -DCMAKE_CXX_COMPILER=clang++ CMakeLists.txt

cmake_minimum_required (VERSION 2.6)

project (YOMM2)

set (YOMM2_VERSION_MAJOR 0)
set (YOMM2_VERSION_MINOR 0)

include_directories ("${YOMM2_SOURCE_DIR}/include")
link_directories ("${YOMM2_BINARY_DIR}/src")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DYOMM2_DEBUG=1")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "-D_SCL_SECURE_NO_WARNINGS /EHsc")
endif()

include(CTest)

add_subdirectory (src)

set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(extern/benchmark)
include_directories(extern/benchmark/include)
set_target_properties(benchmark PROPERTIES FOLDER Extern)

add_subdirectory (tests)
#add_subdirectory (examples)

enable_testing()

add_test (whitebox tests/whitebox)
add_test (blackbox tests/blackbox)

INSTALL (DIRECTORY include/yorel DESTINATION include)
