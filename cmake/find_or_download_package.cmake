macro(find_or_download_package PACKAGE)
  find_package(${PACKAGE} QUIET)
  if(NOT ${${PACKAGE}_FOUND})
    message(STATUS "Package \"${PACKAGE}\" not found in system.")
    message(STATUS
      "Downloading dependency \"${PACKAGE}\" and building from source."
    )

    # Prepare download instructions for dependency
    configure_file(
      ${CMAKE_SOURCE_DIR}/cmake/${PACKAGE}_download.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}-download/CMakeLists.txt
    )

    # Download dependency
    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}-download
      OUTPUT_QUIET
    )
    if(result)
      message(FATAL_ERROR "Download of dependency ${PACKAGE} failed: ${result}")
    endif()

    # Build dependency
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}-download
    )
    if(result)
      message(FATAL_ERROR "Build of  dependency ${PACKAGE} failed: ${result}")
    endif()

    # Update search path and use regular find_package to add dependency
    # TODO Use same directory here as for configure_file up there and inside
    # download instructions!
    list(
      APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern/lib/cmake/${PACKAGE}"
    )
    find_package(${PACKAGE} REQUIRED)
  endif()
endmacro()
