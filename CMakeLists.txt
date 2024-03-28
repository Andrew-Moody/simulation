cmake_minimum_required(VERSION 3.13)

project(MESHING LANGUAGES CXX)

set(MAIN_TARGET meshing)
set(TEST_TARGET testmeshing)

find_package(GTest REQUIRED)

add_executable(${TEST_TARGET} "")

add_subdirectory(tests)

add_library(${MAIN_TARGET} "")

add_subdirectory(src)
#add_subdirectory(dependencies)

target_link_libraries(${TEST_TARGET} ${MAIN_TARGET} GTest::gtest_main)