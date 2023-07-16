# Copyright (c) 2018-2022 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

file(GLOB cpps "*.cpp")

set(code_deps "${CMAKE_SOURCE_DIR}/dev/code2md;${CMAKE_SOURCE_DIR}/dev/md2md;${CMAKE_SOURCE_DIR}/dev/mdgen.py")
set(entries "")

add_compile_options(-save-temps)

foreach(cpp ${cpps})
    cmake_path(REMOVE_EXTENSION cpp LAST_ONLY OUTPUT_VARIABLE md)
    set(md "${md}.md")
    string(REPLACE "reference.in" "reference" md_out "${md}")
    add_custom_command(
        OUTPUT "${md_out}"
        COMMAND ${CMAKE_SOURCE_DIR}/dev/code2md "${cpp}" "${md_out}"
        DEPENDS "${cpp};${code_deps}")
    list(APPEND entries "${md_out}")
    cmake_path(GET cpp STEM exe)
    set(exe "ref_${exe}")
    add_executable("${exe}" "${cpp}")
    add_compile_definitions(YOMM2_CODE)
    target_link_libraries("${exe}" YOMM2::yomm2)
    add_test(NAME "${exe}" COMMAND "${exe}")
endforeach()

file(GLOB mds "*.md")

foreach(md ${mds})
    string(REPLACE "reference.in" "reference" md_out "${md}")
    add_custom_command(
        OUTPUT "${md_out}"
        COMMAND ${CMAKE_SOURCE_DIR}/dev/md2md "${md}" "${md_out}"
        DEPENDS "${md};${code_deps}")
    list(APPEND entries "${md_out}")
endforeach()

add_custom_target(reference ALL DEPENDS "${entries}")
