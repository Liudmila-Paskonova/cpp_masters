cmake_minimum_required(VERSION 3.16)
project(matrix-conan-gtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(matrix INTERFACE)
target_include_directories(matrix INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

add_executable(matrix_app src/main.cpp)
target_link_libraries(matrix_app PRIVATE matrix)

find_package(GTest REQUIRED)   

add_executable(test_matrix test/test_square_matrix.cpp)
target_link_libraries(test_matrix PRIVATE matrix GTest::gtest_main)

enable_testing()
add_test(NAME MatrixTests COMMAND test_matrix)
