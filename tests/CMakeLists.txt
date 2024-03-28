cmake_minimum_required(VERSION 3.13)

target_sources(${TEST_TARGET}
    PRIVATE
        test.cpp
)

target_include_directories(${TEST_TARGET}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)