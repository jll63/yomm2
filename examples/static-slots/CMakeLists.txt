# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(generate-static-slots generate-static-slots.cpp animals.cpp)
target_include_directories(generate-static-slots PRIVATE .)
target_link_libraries(generate-static-slots YOMM2::yomm2)

set(GENERATED_FILES "${CMAKE_CURRENT_BINARY_DIR}/slots.hpp;${CMAKE_CURRENT_BINARY_DIR}/tables.hpp")

add_custom_command(
   OUTPUT ${GENERATED_FILES}
   COMMAND generate-static-slots ${GENERATED_FILES}
   DEPENDS generate-static-slots animals.cpp
)
add_custom_target(
    animals_yomm2_slots_hpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/slots.hpp ${CMAKE_CURRENT_BINARY_DIR}/tables.hpp)

add_library(animals_yomm2_slots INTERFACE)
add_dependencies(animals_yomm2_slots animals_yomm2_slots_hpp)
target_include_directories(animals_yomm2_slots INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(static-slots-app app.cpp animals.cpp)
target_include_directories(static-slots-app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
    static-slots-app PRIVATE animals_yomm2_slots YOMM2::yomm2)
add_test(NAME static-slots-app COMMAND static-slots-app)

# For convenience: all targets related to static slots generation.
add_custom_target(static-slots DEPENDS static-slots-app test_generator)
