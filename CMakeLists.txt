# -*- compile-command: "cd build ; make && ctest" -*-

# Copyright (c) 2018 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required (VERSION 2.6)

project (YOMM2)

set (YOMM2_VERSION_MAJOR 1)
set (YOMM2_VERSION_MINOR 0)

find_package(Boost)

include_directories ("${YOMM2_SOURCE_DIR}/include")
link_directories ("${YOMM2_BINARY_DIR}/src")

message(STATUS "Using Boost libraries from ${Boost_INCLUDE_DIRS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${Boost_INCLUDE_DIRS}")

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "-D_SCL_SECURE_NO_WARNINGS /EHsc")
endif()

add_subdirectory (src)

add_subdirectory (examples)

if (NOT DEFINED(YOMM2_ENABLE_TESTS))
  set(YOMM2_ENABLE_TESTS CACHE BOOL ON)
endif()

if (${YOMM2_ENABLE_TESTS})
  message(STATUS "Tests enabled")
  include(CTest)
  add_subdirectory (tests)
  enable_testing()
  add_test (whitebox tests/whitebox)
  add_test (blackbox tests/blackbox)
endif()

if (NOT DEFINED(YOMM2_ENABLE_BENCHMARKS))
  set(YOMM2_ENABLE_BENCHMARKS CACHE BOOL ON)
endif()

if (${YOMM2_ENABLE_BENCHMARKS})
  message(STATUS "Benchmarks enabled")
  set(BENCHMARK_ENABLE_TESTING CACHE BOOL OFF)
  set(BENCHMARK_ENABLE_GTEST_TESTS CACHE BOOL OFF)
  add_subdirectory(extern/benchmark)
  include_directories(extern/benchmark/include)
endif()

INSTALL (DIRECTORY include/yorel DESTINATION include)
