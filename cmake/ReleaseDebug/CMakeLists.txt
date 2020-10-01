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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")
include(find_or_download_package)

find_or_download_package(YOMM2)

if (${YOMM2_ENABLE_TESTS})
  message(STATUS "Tests enabled")
  include(CTest)
endif()

add_subdirectory(../../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)

add_subdirectory(../../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
