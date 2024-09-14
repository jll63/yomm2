# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(test_static_list test_static_list.cpp)
target_link_libraries(test_static_list YOMM2::yomm2)
add_test(NAME test_static_list COMMAND test_static_list)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core YOMM2::yomm2)
add_test(NAME test_core COMMAND test_core)

add_executable(test_templates test_templates.cpp)
target_link_libraries(test_templates YOMM2::yomm2)
add_test(NAME test_templates COMMAND test_templates)

add_executable(test_compiler test_compiler.cpp)
target_link_libraries(test_compiler YOMM2::yomm2)
add_test(NAME test_compiler COMMAND test_compiler)

add_executable(test_blackbox test_blackbox.cpp)
target_link_libraries(test_blackbox YOMM2::yomm2)
add_test(NAME test_blackbox COMMAND test_blackbox)

add_executable(test_lab test_lab.cpp)
target_link_libraries(test_lab YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_lab COMMAND test_lab)

add_executable(test_member_method test_member_method.cpp)
target_link_libraries(test_member_method YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_member_method COMMAND test_member_method)

add_executable(test_namespaces test_namespaces.cpp)
target_link_libraries(test_namespaces YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_namespaces COMMAND test_namespaces)

add_executable(test_pointer_to_method test_pointer_to_method.cpp)
target_link_libraries(test_pointer_to_method YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME test_pointer_to_method COMMAND test_pointer_to_method)

if(YOMM2_ENABLE_BENCHMARKS AND NOT (WIN32 OR APPLE))
  add_executable(benchmarks benchmarks.cpp)
  target_link_libraries(
    benchmarks YOMM2::yomm2 benchmark::benchmark ${CMAKE_THREAD_LIBS_INIT})
  add_executable(benchmark_rdtsc benchmark_rdtsc.cpp)
  target_link_libraries(benchmark_rdtsc YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
endif()

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

if (NOT MSVC)
  # boost::demangle's output is not stable across compilers, just test with
  # clang and gcc.
  add_executable(test_generator_forward_decls test_generator_forward_decls.cpp)
  target_link_libraries(test_generator_forward_decls YOMM2::yomm2 ${CMAKE_THREAD_LIBS_INIT})
  add_test(NAME test_generator_forward_decls COMMAND test_generator_forward_decls)

  # Thanks to https://stackoverflow.com/users/3440745/tsyvarev for providing the
  # following cmake incantations.

  if(CMAKE_CONFIGURATION_TYPES)
    # Multi-configuration generator
    #
    # Explicitly use generator expression,
    # so CMake won't automatically append the subdirectory with a config name.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
  else()
    # Single-configuration generator
    #
    # While it is allowable to create config-specific directory,
    # we don't do that: every configuration already has its own build tree.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()

  add_library(test_generator_lib OBJECT test_generator_domain.cpp)
  target_link_libraries(test_generator_lib YOMM2::yomm2)

  add_executable(test_generator_gen test_generator_gen.cpp)
  add_dependencies(test_generator_gen test_generator_lib)
  target_link_libraries(test_generator_gen test_generator_lib YOMM2::yomm2)

  set(GENERATED_FILES
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_generator_slots.hpp"
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_generator_tables.hpp"
  )

  add_custom_command(
      OUTPUT ${GENERATED_FILES}
      COMMAND test_generator_gen
      DEPENDS test_generator_gen
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
  add_custom_target(test_generator_generate DEPENDS ${GENERATED_FILES})
  add_library(test_generator_generated_lib INTERFACE)
  add_dependencies(test_generator_generated_lib test_generator_generate)
  target_include_directories(test_generator_generated_lib INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

  add_executable(test_generator test_generator.cpp)
  target_include_directories(test_generator PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  target_link_libraries(test_generator PRIVATE test_generator_generated_lib test_generator_lib YOMM2::yomm2)
  add_test(NAME test_generator COMMAND test_generator)
endif()
