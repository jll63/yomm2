# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_library(generator_lib OBJECT animals.cpp)
target_link_libraries(generator_lib YOMM2::yomm2)

add_executable(generator_gen generator_gen.cpp)
target_link_libraries(generator_gen generator_lib YOMM2::yomm2)

set(GENERATED_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/slots.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/tables.hpp"
)
add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND generator_gen
    DEPENDS generator_gen
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(generator_generate DEPENDS ${GENERATED_FILES})
add_library(generator_generated_lib INTERFACE)
add_dependencies(generator_generated_lib generator_generate)
target_include_directories(generator_generated_lib INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(generator_app generator_app.cpp)
target_include_directories(generator_app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(generator_app PRIVATE generator_generated_lib generator_lib YOMM2::yomm2)
add_test(NAME generator_app COMMAND generator_app)
