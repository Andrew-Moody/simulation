cmake_minimum_required(VERSION 3.13)

target_sources(${MAIN_TARGET}
	PRIVATE
		surfacemeshdata.h
		api.h
		api.cpp
)

target_include_directories(${MAIN_TARGET}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
