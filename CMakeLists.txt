# Root cmake
cmake_minimum_required (VERSION 3.5)
project (myst)
enable_language(CXX)

# IMPOSTAZIONE C++ STANDARD (Necessaria per ODB su Bionic)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# LOGICA CONDIZIONALE PER DOCKER: Disabilita -Werror solo nel container
# Verifica se la variabile d'ambiente DOCKER_BUILD è impostata nel Dockerfile.
if (NOT "$ENV{DOCKER_BUILD}" STREQUAL "")
	message(STATUS "Disabling -Werror for Docker Build environment.")
	# Aggiunge i flag per disabilitare i warning come errori.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -Wno-type-limits")
else()
	message(STATUS "Using default compiler flags (Vagrant/Local).")
endif()

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
add_custom_target(checkcpp
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND cppcheck --enable=all --xml src 2>check.xml
		COMMAND cppcheck-htmlreport --report-dir=check  --source-dir=./ --file=check.xml
		COMMAND echo use your browser to view check/index.html
		COMMENT "Executing checkcpp"
)
add_custom_target(style
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
		COMMAND ../astyle -r -Q --options=../astylerc "./*.cpp,*.hpp"
)