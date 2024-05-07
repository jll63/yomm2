set(readme_in "${CMAKE_SOURCE_DIR}/docs.in/README.md")
set(readme_out "${CMAKE_SOURCE_DIR}/docs/README.md")

if(${YOMM2_ENABLE_DOC})
  add_custom_target(PAGES_README ALL DEPENDS "${readme_out}")
  add_custom_command(
    OUTPUT "${readme_out}"
    COMMAND "${YOMM2_PYTHON}" "${CMAKE_SOURCE_DIR}/dev/md2md" "${readme_in}" "${readme_out}"
    DEPENDS "${readme_in}")
endif()

add_subdirectory(reference)
add_subdirectory(tutorials)
