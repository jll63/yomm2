cmake_minimum_required(VERSION 3.0)
project(YOMM2_ReleaseDebug VERSION 1.0)

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
    set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Only Release build allowed! Forcing Release build.")
    endif()
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")
include(find_or_download_package)

find_or_download_package(YOMM2)

add_subdirectory(../../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)

add_subdirectory(../../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)