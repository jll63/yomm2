cmake_minimum_required(VERSION 3.0)
project(YOMM2_ReleaseDebug VERSION 1.0)

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
else()
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Only Debug build allowed! Forcing Debug build.")
  endif()
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++17")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")
include(find_or_download_package)

find_or_download_package(YOMM2)

include(CTest)
enable_testing()

add_subdirectory(../../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)

add_subdirectory(../../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
