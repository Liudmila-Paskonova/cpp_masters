cmake_minimum_required(VERSION 3.15)
project(HindleyMilner)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

include_directories(include)

file(GLOB_RECURSE LIB_SOURCES "lib/*.cpp")
file(GLOB_RECURSE MAIN_SOURCES "src/*.cpp") 
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

add_library(hm ${LIB_SOURCES})

add_executable(hindley_milner ${MAIN_SOURCES})
target_link_libraries(hindley_milner hm)

enable_testing()
add_executable(tests ${TEST_SOURCES})

if(TARGET GTest::gtest_main)
    target_link_libraries(tests hm GTest::gtest_main)
elseif(TARGET GTest::Main)
    target_link_libraries(tests hm GTest::Main)
else()
    target_link_libraries(tests hm gtest_main)
endif()

include(GoogleTest)
gtest_discover_tests(tests)