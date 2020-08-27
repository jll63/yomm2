cmake_minimum_required(VERSION 3.5)
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
add_library(yomm2 ALIAS YOMM2::yomm2)

get_target_property(YOMM2_INCLUDE_DIRS yomm2 INTERFACE_INCLUDE_DIRECTORIES)
message("Test: " ${YOMM2_INCLUDE_DIRS})

add_subdirectory(../../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)

add_subdirectory(../../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)