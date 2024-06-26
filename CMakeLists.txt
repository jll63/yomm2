# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0057 NEW)
project(YOMM2 LANGUAGES CXX VERSION 1.5.2)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(YOMM2_DOWNLOAD_DEPENDENCIES "Set to ON to build missing dependencies from source" OFF)
include(cmake/download_package.cmake)

# Find Boost dependency
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost 1.74 QUIET)
if(NOT TARGET Boost::headers)
  if (YOMM2_DOWNLOAD_DEPENDENCIES)
    download_package(Boost INSTALL_WITH_YOMM)
  else()
    message(FATAL_ERROR  "Boost was not found on your system. Set YOMM2_DOWNLOAD_DEPENDENCIES to ON to download and build from the sources.")
  endif()
endif()
message(STATUS "Using Boost libraries from ${Boost_INCLUDE_DIRS}")

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") AND (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
  add_compile_options(-save-temps -fverbose-asm -masm=intel)
endif()

if(MSVC)
  add_compile_definitions(_SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
  add_compile_options(/EHsc /FAs)
endif()

option(YOMM2_DEBUG_MACROS "Set to ON to debug macros" OFF)
if(${YOMM2_DEBUG_MACROS})
  message(STATUS "Macro debugging enabled")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_SOURCE_DIR}/dev/ppfc)
endif()

add_subdirectory(src)

option(YOMM2_ENABLE_TESTS "Set to ON to build tests" ON)
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(YOMM2_ENABLE_BENCHMARKS
  "Set to ON to build benchmarks" OFF
  "YOMM2_ENABLE_TESTS" OFF
)
if(${YOMM2_ENABLE_TESTS})
  message(STATUS "Tests enabled")
  if(${YOMM2_ENABLE_BENCHMARKS})
    find_package(benchmark QUIET)
    if(NOT ${benchmark_FOUND})
      if (YOMM2_DOWNLOAD_DEPENDENCIES)
        download_package(benchmark)
      else()
        message(FATAL_ERROR  "benchmarks was not found on your system. Set YOMM2_DOWNLOAD_DEPENDENCIES to ON to download and build from the sources.")
      endif()
    endif()
    message(STATUS "Benchmarks enabled")
  endif()
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(ce)
endif()

option(YOMM2_ENABLE_EXAMPLES "Set to ON to build examples" ON)
if(${YOMM2_ENABLE_EXAMPLES})
  message(STATUS "Examples enabled")
  add_subdirectory(examples)
endif()

option(YOMM2_ENABLE_DOC "Set to ON to generate tutorials and reference" OFF)
option(YOMM2_ENABLE_BENCHMARKS "Set to ON to enable benchmarks" OFF)

set(readme_md "${CMAKE_SOURCE_DIR}/README.md")
set(readme_cpp "${CMAKE_SOURCE_DIR}/examples/README.cpp")

if(${YOMM2_ENABLE_DOC})
  message(STATUS "Documentation generation enabled")
  set(YOMM2_PYTHON ${CMAKE_SOURCE_DIR}/.venv/bin/python)
  add_custom_target(README_md ALL DEPENDS "${readme_md}")
  add_custom_command(
    OUTPUT "${readme_md}"
    COMMAND "${YOMM2_PYTHON}" ${CMAKE_SOURCE_DIR}/dev/code2md "${readme_cpp}" "${readme_md}"
    DEPENDS "${readme_cpp}")
endif()

if(YOMM2_ENABLE_TESTS OR YOMM2_ENABLE_DOC)
  add_subdirectory(docs.in)
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
  DESTINATION lib/cmake/YOMM2
)
# Configure package config (tells using code about dependencies)
configure_package_config_file(
  cmake/YOMM2Config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/YOMM2Config.cmake
  INSTALL_DESTINATION lib/cmake/YOMM2
)
# Copy config files to install directory
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2ConfigVersion.cmake"
  DESTINATION lib/cmake/YOMM2
)

install(DIRECTORY include/yorel DESTINATION include)
