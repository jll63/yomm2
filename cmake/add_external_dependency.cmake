macro(add_external_dependency DEPENDENCY)
  message(STATUS "Downloading dependency \"${DEPENDENCY}\" and building from
  source.")

  # Prepare download instructions for dependency
  configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/${DEPENDENCY}_download.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DEPENDENCY}-download/CMakeLists.txt
  )

  # Download dependency
  execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DEPENDENCY}-download
    OUTPUT_QUIET
  )
  if(result)
    message(FATAL_ERROR "Download of dependency ${DEPENDENCY} failed: ${result}")
  endif()

  # Build dependency
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DEPENDENCY}-download
  )
  if(result)
    message(FATAL_ERROR "Build of  dependency ${DEPENDENCY} failed: ${result}")
  endif()

  # Update search path and use regular find_package to add dependency
  # TODO Use same directory here as for configure_file up there and inside
  # download instructions!
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern/lib/cmake/${DEPENDENCY}")
  find_package(${DEPENDENCY} REQUIRED)
endmacro()
