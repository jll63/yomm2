# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(chain chain.cpp)
target_link_libraries(chain YOMM2::yomm2)
add_test(NAME chain COMMAND chain)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core YOMM2::yomm2)
add_test(NAME test_core COMMAND test_core)

add_executable(templates templates.cpp)
target_link_libraries(templates YOMM2::yomm2)
add_test(NAME templates COMMAND templates)

add_executable(compiler compiler.cpp)
target_link_libraries(compiler YOMM2::yomm2)
add_test(NAME compiler COMMAND compiler)

add_executable(blackbox blackbox.cpp)
target_link_libraries(blackbox YOMM2::yomm2)
add_test(NAME blackbox COMMAND blackbox)

if (NOT WIN32)
add_executable(manual_call manual_call.cpp)
target_link_libraries(manual_call YOMM2::yomm2)
add_test(NAME manual_call COMMAND manual_call)
endif()

add_executable(lab lab.cpp)
target_link_libraries(lab YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
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
  add_executable(benchmarks benchmarks.cpp)
  target_link_libraries(benchmarks YOMM2::yomm2 benchmark::benchmark)
endif()

# add_executable(intrusive intrusive.cpp)
# target_link_libraries(intrusive YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
# add_test(NAME intrusive COMMAND intrusive)

add_executable(test_virtual_ptr_basic test_virtual_ptr_basic.cpp)
target_link_libraries(test_virtual_ptr_basic YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_virtual_ptr_basic COMMAND test_virtual_ptr_basic)

add_executable(test_virtual_ptr test_virtual_ptr.cpp)
target_link_libraries(test_virtual_ptr YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_virtual_ptr COMMAND test_virtual_ptr)

add_executable(test_virtual_ptr_all test_virtual_ptr_all.cpp)
target_link_libraries(test_virtual_ptr_all YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_virtual_ptr_all COMMAND test_virtual_ptr_all)

add_executable(test_move test_move.cpp)
target_link_libraries(test_move YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_move COMMAND test_move)

add_executable(test_custom_rtti test_custom_rtti.cpp)
target_link_libraries(test_custom_rtti YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(test_custom_rtti PROPERTIES COMPILE_FLAGS "-fno-rtti")
endif()
add_test(NAME test_custom_rtti COMMAND test_custom_rtti)
