include(CMakeFindDependencyMacro)

find_dependency(Boost 1.74... REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/YOMM2Targets.cmake")

check_required_components(YOMM2)
