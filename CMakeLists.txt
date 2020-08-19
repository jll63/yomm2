# -*- compile-command: "cd build ; make && ctest" -*-

# Copyright (c) 2018-2020 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required (VERSION 3.0)

project (YOMM2 VERSION 1.0)

# Find Boost dependency
find_package(Boost 1.53 REQUIRED)
message(STATUS "Using Boost libraries from ${Boost_INCLUDE_DIRS}")

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps")
  endif ()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "/std:c++17 -D_SCL_SECURE_NO_WARNINGS /EHsc")
endif()

# Remember original build type if doing a hybrid (Debug<->Release) build.
set(YOMM2_BUILD_TYPE ${CMAKE_BUILD_TYPE})

# Hybrid builds: first set mode for the library.

if("${YOMM2_BUILD_TYPE}" STREQUAL "DebugRelease")
  # Compile the library in Debug mode and the tests and examples in Release
  # mode.
  message(STATUS "Testing compatibility of Release code against a Debug version of yomm2")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if("${YOMM2_BUILD_TYPE}" STREQUAL "ReleaseDebug")
  # Compile the library in Release mode and the tests and examples in Debug
  # mode.
  message(STATUS "Testing compatibility of Debug code against a Release version of yomm2")
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory (src)

# Hybrid builds: set the (different) mode for examples and tests.

if("${YOMM2_BUILD_TYPE}" STREQUAL "DebugRelease")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if("${YOMM2_BUILD_TYPE}" STREQUAL "ReleaseDebug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

option(YOMM2_ENABLE_EXAMPLES "Set to ON to build examples" ON)
if (${YOMM2_ENABLE_EXAMPLES})
  message(STATUS "Examples enabled")
  add_subdirectory (examples)
endif()

option(YOMM2_ENABLE_TESTS "Set to ON to build tests" OFF)
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(YOMM2_ENABLE_BENCHMARKS
  "Set to ON to build benchmarks" OFF
  "YOMM2_ENABLE_TESTS" OFF
)
if (${YOMM2_ENABLE_TESTS})
  message(STATUS "Tests enabled")
  if (${YOMM2_ENABLE_BENCHMARKS})
    message(STATUS "Benchmarks enabled")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    include(find_or_download_package)
    find_or_download_package(benchmark)
  endif()
  include(CTest)
  add_subdirectory (tests)
  enable_testing()
  add_test (whitebox tests/whitebox)
  add_test (blackbox tests/blackbox)
  add_test (namespaces tests/namespaces)
  add_test (synopsis examples/synopsis)
  add_test (matrix examples/matrix)
  add_test (accept_no_visitors examples/accept_no_visitors)
  add_test (adventure examples/adventure)
  add_test (next examples/next)
  add_test (asteroids examples/asteroids)
  add_test (containers examples/containers/containers)
endif()

## Install instruction
# Create version file for cmake package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  YOMM2ConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion
)
# Create targets file of yomm2
install(EXPORT YOMM2Targets
  FILE YOMM2Targets.cmake
  NAMESPACE YOMM2::
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/YOMM2
)
# Configure package config (tells using code about dependencies)
configure_package_config_file(
  cmake/Config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/YOMM2Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/YOMM2
)
# Copy config files to install directory
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2ConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/YOMM2
)

INSTALL (DIRECTORY include/yorel DESTINATION include)
