# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(generate-static-offsets generate-static-offsets.cpp animals.cpp)
target_include_directories(generate-static-offsets PRIVATE .)
target_link_libraries(generate-static-offsets YOMM2::yomm2)

add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/animals-yomm2-offsets.hpp
   COMMAND generate-static-offsets > ${CMAKE_CURRENT_BINARY_DIR}/animals-yomm2-offsets.hpp
   DEPENDS generate-static-offsets animals.cpp
)
add_custom_target(
    animals_yomm2_offsets_hpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/animals-yomm2-offsets.hpp)

add_library(animals_yomm2_offsets INTERFACE)
add_dependencies(animals_yomm2_offsets animals_yomm2_offsets_hpp)
target_include_directories(animals_yomm2_offsets INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(static-offsets-app app.cpp animals.cpp)
target_include_directories(static-offsets-app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
    static-offsets-app PRIVATE animals_yomm2_offsets YOMM2::yomm2)
add_test(NAME static-offsets-app COMMAND static-offsets-app)
