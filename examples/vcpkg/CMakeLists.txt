cmake_minimum_required(VERSION 3.15)

project(adventure CXX)

find_package(YOMM2 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(adventure adventure.cpp)

target_link_libraries(adventure PRIVATE YOMM2::yomm2)
