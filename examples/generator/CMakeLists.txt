# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(generate generate.cpp animals.cpp)
target_include_directories(generate PRIVATE .)
target_link_libraries(generate YOMM2::yomm2)

set(GENERATED_FILES "${CMAKE_CURRENT_BINARY_DIR}/slots.hpp;${CMAKE_CURRENT_BINARY_DIR}/tables.hpp")

add_custom_command(
   OUTPUT ${GENERATED_FILES}
   COMMAND generate ${GENERATED_FILES}
   cat "${CMAKE_CURRENT_BINARY_DIR}/slots.hpp"
   DEPENDS generate animals.cpp
)
add_custom_target(
    animals_yomm2_slots_hpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/slots.hpp ${CMAKE_CURRENT_BINARY_DIR}/tables.hpp)

add_library(animals_yomm2_slots INTERFACE)
add_dependencies(animals_yomm2_slots animals_yomm2_slots_hpp)
target_include_directories(animals_yomm2_slots INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(generator_example generator_example.cpp animals.cpp)
target_include_directories(generator_example PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
    generator_example PRIVATE animals_yomm2_slots YOMM2::yomm2)
add_test(NAME generator_example COMMAND generator_example)
