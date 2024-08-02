# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

set(code_deps "${CMAKE_SOURCE_DIR}/dev/code2md;${CMAKE_SOURCE_DIR}/dev/md2md;${CMAKE_SOURCE_DIR}/dev/mdgen.py")
file(GLOB cpps "*.cpp")
set(entries "")

foreach(cpp ${cpps})
    cmake_path(REMOVE_EXTENSION cpp LAST_ONLY OUTPUT_VARIABLE md)

    set(md "${md}.md")
    string(REPLACE "docs.in" "docs" md_out "${md}")
    add_custom_command(
        OUTPUT "${md_out}"
        COMMAND "${YOMM2_PYTHON}" ${CMAKE_SOURCE_DIR}/dev/code2md "${cpp}" "${md_out}"
        DEPENDS "${cpp};${code_deps}")
    list(APPEND entries "${md_out}")

    cmake_path(GET cpp STEM exe)
    set(exe "ref_${exe}")
    add_executable("${exe}" "${cpp}")
    add_compile_definitions(YOMM2_CODE)
    target_link_libraries("${exe}" YOMM2::yomm2)
    add_test(NAME "${exe}" COMMAND "${exe}")
endforeach()

file(GLOB_RECURSE MDs "*.md")

foreach(MD ${MDs})
    string(REPLACE "docs.in" "docs" MD_out "${MD}")
    add_custom_command(
        OUTPUT "${MD_out}"
        COMMAND "${YOMM2_PYTHON}" ${CMAKE_SOURCE_DIR}/dev/md2md "${MD}" "${MD_out}"
        DEPENDS "${MD};${code_deps}")
    list(APPEND entries "${MD_out}")
    endforeach()

add_custom_target(REFERENCE DEPENDS "${entries}")
add_dependencies(DOCS REFERENCE)
