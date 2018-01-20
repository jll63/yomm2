# Copyright (c) 2018 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(whitebox whitebox.cpp)
target_link_libraries (whitebox yomm2)

add_executable(blackbox blackbox.cpp)
target_link_libraries (blackbox yomm2)

if (${YOMM2_ENABLE_BENCHMARKS})
  add_executable(benchmarks benchmarks.cpp benchmarks_vfuncs.cpp )
  target_link_libraries (benchmarks yomm2 benchmark)
endif()
