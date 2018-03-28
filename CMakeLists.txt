# Root cmake
cmake_minimum_required (VERSION 3.5)
project (myst)
enable_language(CXX)
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
	math(EXPR NN ${N}/2)
	execute_process(
		COMMAND echo "${NN}"
		OUTPUT_FILE makejobs
	)
else()
	execute_process(
		COMMAND echo "1"
		OUTPUT_FILE makejobs
	)
endif()

set (CMAKE_USE_RELATIVE_PATHS true)
add_subdirectory(src)
include(UsePkgConfig)
#include(${CMAKE_SOURCE_DIR}/cmake/Modules/PrecompiledHeader.cmake)

# The version number.
#set (myst_VERSION_MAJOR 3)
#set (myst_VERSION_MINOR 1)
#find_library(MYSQL mysqld)
#find_library(BOOST boost)
#set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
