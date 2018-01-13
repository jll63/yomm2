add_executable(synopsis synopsis.cpp)
target_link_libraries (synopsis yomm2)
add_test (synopsis synopsis)

add_executable(matrix matrix.cpp)
target_link_libraries (matrix yomm2)
add_test (matrix matrix)
