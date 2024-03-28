cmake_minimum_required(VERSION 3.13)

project(MESHING LANGUAGES C CXX)

set(MAIN_TARGET meshing)
set(TEST_TARGET testmeshing)

add_executable(${TEST_TARGET} "")

add_subdirectory(tests)

add_library(${MAIN_TARGET} "")

add_subdirectory(src)
add_subdirectory(dependencies)

find_package(GTest REQUIRED)

target_link_libraries(${TEST_TARGET} 
    PRIVATE
        ${MAIN_TARGET}
        GTest::gtest_main
)