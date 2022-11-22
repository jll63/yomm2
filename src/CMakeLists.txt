# Copyright (c) 2018-2021 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

option(YOMM2_SHARED "Build yomm2 as a shared library" OFF)
if(YOMM2_SHARED)
  add_library(yomm2 SHARED)
  if(NOT WIN32)
    target_compile_options(yomm2 PRIVATE -fvisibility=hidden)
  endif()
  target_compile_definitions(yomm2
    PRIVATE -DBUILDING_YOMM2=1
    PUBLIC -DYOMM2_SHARED=1
  )
else()
    add_library(yomm2 STATIC)
endif()

target_sources(yomm2 PRIVATE yomm2.cpp)

target_include_directories(yomm2 PUBLIC
  $<BUILD_INTERFACE:${YOMM2_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PUBLIC $<TARGET_PROPERTY:Boost::headers,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(yomm2 PUBLIC Boost::headers)

install(TARGETS yomm2
  EXPORT YOMM2Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

add_library(YOMM2::yomm2 ALIAS yomm2)
