# Copyright (c) 2018-2022 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(chain chain.cpp)
target_link_libraries(chain YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME chain COMMAND chain)

add_executable(core core.cpp)
target_link_libraries(core YOMM2::yomm2)
add_test(NAME core COMMAND core)

add_executable(templates templates.cpp)
target_link_libraries(templates YOMM2::yomm2)
add_test(NAME templates COMMAND templates)

add_executable(runtime runtime.cpp)
target_link_libraries(runtime YOMM2::yomm2)
add_test(NAME runtime COMMAND runtime)

add_executable(blackbox blackbox.cpp)
target_link_libraries(blackbox YOMM2::yomm2)
add_test(NAME blackbox COMMAND blackbox)

add_executable(lab lab.cpp)
target_link_libraries(lab YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
set_source_files_properties(lab.cpp PROPERTIES COMPILE_FLAGS "-save-temps")
add_test(NAME lab COMMAND lab)

add_executable(member_method member_method.cpp)
target_link_libraries(member_method YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME member_method COMMAND member_method)

add_executable(namespaces namespaces.cpp)
target_link_libraries(namespaces YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME namespaces COMMAND namespaces)

add_executable(pointer_to_method pointer_to_method.cpp)
target_link_libraries(pointer_to_method YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME pointer_to_method COMMAND pointer_to_method)

if(${YOMM2_ENABLE_BENCHMARKS})
  set_source_files_properties(benchmarks_vfuncs.cpp PROPERTIES COMPILE_FLAGS "-save-temps")
  add_executable(benchmarks benchmarks.cpp benchmarks_vfuncs.cpp )
  target_link_libraries(benchmarks YOMM2::yomm2 benchmark::benchmark)
endif()
