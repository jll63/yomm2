# Copyright (c) 2018-2021 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

option(YOMM2_SHARED "Build yomm2 as a shared library" OFF)
option(
  YOMM2_CHECK_ABI_COMPATIBILITY
  "Build shared library and examples in different modes" OFF)

if(YOMM2_SHARED)
  message(STATUS "Building a shared library")
  add_library(yomm2 SHARED)
  target_sources(yomm2 PRIVATE yomm2.cpp)
  if(NOT WIN32)
    target_compile_options(yomm2 PRIVATE -fvisibility=hidden)
  endif()
  target_compile_definitions(yomm2 PUBLIC YOMM2_SHARED=1)
  target_include_directories(
    yomm2 PUBLIC
    $<BUILD_INTERFACE:${YOMM2_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PUBLIC $<TARGET_PROPERTY:Boost::headers,INTERFACE_INCLUDE_DIRECTORIES>
  )
  target_link_libraries(yomm2 PUBLIC Boost::headers)
  target_compile_features(yomm2 PUBLIC cxx_std_17)
  if (${YOMM2_CHECK_ABI_COMPATIBILITY})
    message(STATUS "type: ${CMAKE_BUILD_TYPE} FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set_target_properties(yomm2 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}" )
      #set_property(TARGET yomm2 PROPERTY COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
  endif()
else()
  if (${YOMM2_CHECK_ABI_COMPATIBILITY})
    message(FATAL_ERROR "YOMM2_CHECK_ABI_COMPATIBILITY does not make sense for headers only mode." )
  endif()
  message(STATUS "Building a headers only library")
  add_library(yomm2 INTERFACE)
  target_include_directories(
    yomm2 INTERFACE
    $<BUILD_INTERFACE:${YOMM2_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(yomm2 INTERFACE Boost::headers)
  target_compile_features(yomm2 INTERFACE cxx_std_17)
endif()

install(TARGETS yomm2
  EXPORT YOMM2Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

add_library(YOMM2::yomm2 ALIAS yomm2)
