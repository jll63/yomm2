# Copyright (c) 2018-2020 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(whitebox whitebox.cpp)
target_link_libraries(whitebox YOMM2::yomm2)
add_test(NAME whitebox COMMAND whitebox)

add_executable(blackbox blackbox.cpp)
target_link_libraries(blackbox YOMM2::yomm2)
add_test(NAME blackbox COMMAND blackbox)

add_executable(lab lab.cpp)
target_link_libraries(lab YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME lab COMMAND lab)

add_executable(namespaces namespaces.cpp)
target_link_libraries(namespaces YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME namespaces COMMAND namespaces)

add_executable(static_methods static_methods.cpp)
target_link_libraries(static_methods YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME static_methods COMMAND static_methods)

if(${YOMM2_ENABLE_BENCHMARKS})
  set_source_files_properties(benchmarks_vfuncs.cpp PROPERTIES COMPILE_FLAGS "-save-temps")
  add_executable(benchmarks benchmarks.cpp benchmarks_vfuncs.cpp )
  target_link_libraries(benchmarks YOMM2::yomm2 benchmark::benchmark)
endif()
