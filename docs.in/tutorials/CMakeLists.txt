# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(api api.cpp)
target_link_libraries(api YOMM2::yomm2)
add_test(NAME api COMMAND api)

add_executable(templates_tutorial_vector_1 templates_tutorial_vector_1.cpp)
target_link_libraries(templates_tutorial_vector_1 YOMM2::yomm2)
add_test(NAME templates_tutorial_vector_1 COMMAND templates_tutorial_vector_1)

add_executable(templates_tutorial_vector_2 templates_tutorial_vector_2.cpp)
target_link_libraries(templates_tutorial_vector_2 YOMM2::yomm2)
add_test(NAME templates_tutorial_vector_2 COMMAND templates_tutorial_vector_2)

add_executable(templates_tutorial_matrix_1 templates_tutorial_matrix_1.cpp)
target_link_libraries(templates_tutorial_matrix_1 YOMM2::yomm2)
add_test(NAME templates_tutorial_matrix_1 COMMAND templates_tutorial_matrix_1)

add_executable(templates_tutorial_matrix_2 templates_tutorial_matrix_2.cpp)
target_link_libraries(templates_tutorial_matrix_2 YOMM2::yomm2)
add_test(NAME templates_tutorial_matrix_2 COMMAND templates_tutorial_matrix_2)

add_executable(custom_rtti_tutorial custom_rtti_tutorial.cpp)
if(NOT YOMM2_SHARED)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set_target_properties(custom_rtti_tutorial PROPERTIES COMPILE_FLAGS "-fno-rtti")
    endif()
endif()
target_link_libraries(custom_rtti_tutorial YOMM2::yomm2)
add_test(NAME custom_rtti_tutorial COMMAND custom_rtti_tutorial)

file(GLOB templates_tutorial_cpps "templates_tutorial_*_*.cpp")
string(REGEX MATCHALL "templates_tutorial_[^_]+_." templates_tutorial_execs "${templates_tutorial_cpps}")

if(${YOMM2_ENABLE_DOC})
    SET(tutorials "")
    SET(cpps "api.cpp;templates_tutorial.cpp;custom_rtti_tutorial.cpp")
    foreach(cpp ${cpps})
        cmake_path(GET cpp STEM LAST_ONLY stem)
        set(md "${CMAKE_SOURCE_DIR}/docs/tutorials/${stem}.md")
        add_custom_command(
            OUTPUT "${md}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/docs.in/tutorials"
            COMMAND ${CMAKE_SOURCE_DIR}/dev/code2md "${cpp}" "${md}"
            DEPENDS "${CMAKE_SOURCE_DIR}/docs.in/tutorials/${cpp}")
        list(APPEND tutorials "${md}")
    endforeach()

    add_custom_target(make_tutorials ALL DEPENDS "${tutorials}")
    add_custom_target(build_and_make_tutorials ALL DEPENDS "${tutorials};${templates_tutorial_execs}")
endif()
