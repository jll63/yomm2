# Copyright (c) 2018-2021 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

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

install(TARGETS yomm2
  EXPORT YOMM2Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

add_library(YOMM2::yomm2 ALIAS yomm2)
