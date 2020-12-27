macro(find_or_download_package PACKAGE)
  set(options EXACT PKG_CONFIG INSTALL_WITH_YOMM)
  set(oneValueArgs VERSION)
  cmake_parse_arguments(ARGS
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )
  set(DEPENDENCY_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies/${PACKAGE})
  find_package(
    ${PACKAGE} QUIET
    HINTS ${DEPENDENCY_INSTALL_PREFIX} ${CMAKE_INSTALL_PREFIX}
  )
  if(NOT ${${PACKAGE}_FOUND})
    message(STATUS "Package \"${PACKAGE}\" not found in system.")
    message(STATUS
      "Downloading dependency \"${PACKAGE}\" and building from source."
    )
    # Use below settings for git downloads if available
    if(${CMAKE_VERSION} VERSION_GREATER 3.6)
      list(APPEND ADDITIONAL_GIT_SETTINGS "GIT_SHALLOW True")
    endif()
    if(${CMAKE_VERSION} VERSION_GREATER 3.8)
      list(APPEND ADDITIONAL_GIT_SETTINGS
        "GIT_PROGRESS True GIT_CONFIG advice.detachedHead=false"
      )
    endif()
    # Prepare download instructions for dependency
    configure_file(
      ${CMAKE_SOURCE_DIR}/cmake/${PACKAGE}_download.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}-download/CMakeLists.txt
      @ONLY
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

    # Restrict search path and use regular find_package to add dependency
    find_package(${PACKAGE}
      REQUIRED NO_DEFAULT_PATH PATHS "${DEPENDENCY_INSTALL_PREFIX}"
    )

    # Install the built package alongside YOMM if so desired, by copying the
    # install made in the build tree
    if(${ARGS_INSTALL_WITH_YOMM})
      install(
        DIRECTORY ${DEPENDENCY_INSTALL_PREFIX}/
        DESTINATION ${CMAKE_INSTALL_PREFIX}
      )
    endif()
  endif()
endmacro()
