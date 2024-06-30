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

# Find Boost dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost REQUIRED COMPONENTS mp11 preprocessor dynamic_bitset)

macro(assign_bool VAR)
  if(${ARGN})
    set(${VAR} ON)
  else()
    set(${VAR} OFF)
  endif()
endmacro()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(COMPILER_IS_CLANG ON)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(VARIANT_IS_DEBUG ON)
endif()

if (VARIANT_IS_DEBUG)
  if(COMPILER_IS_CLANG)
    add_compile_options(-fno-limit-debug-info)
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
    add_compile_definitions(_GLIBCXX_DEBUG)
  endif()
else()
  if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
    add_compile_options(-save-temps -fverbose-asm -masm=intel)
  endif()
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
  find_package(Boost REQUIRED COMPONENTS unit_test_framework )
  if(${YOMM2_ENABLE_BENCHMARKS})
    find_package(benchmark REQUIRED)
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

if(YOMM2_ENABLE_TESTS)
  add_custom_target(static-offsets DEPENDS static-offsets-app test_generator)
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
# Copy config files to install directory
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/YOMM2ConfigVersion.cmake"
  DESTINATION lib/cmake/YOMM2
)

install(DIRECTORY include/yorel DESTINATION include)
