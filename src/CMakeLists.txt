#src cmake
cmake_minimum_required (VERSION 3.5)
include(FindPkgConfig)
# The version number.
set(CMAKE_CXX_FLAGS "-g -Wall")
find_library(MYSQL mysqlclient)
if ( NOT MYSQL )
	message(STATUS "Missing mysqlclient")
endif( NOT MYSQL )
#find_library(BOOST boost)
#if ( NOT BOOST )
#	message(FATAL_ERROR "Missing boost")
#endif(NOT BOOST)
pkg_check_modules(MYSQL mysqlclient)
pkg_check_modules(SQLITE sqlite3)
find_library(CRYPT crypt)
message(STATUS "MAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
file(GLOB file_hpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.hpp)
message(STATUS "file_hpp=${file_hpp}")
file(GLOB file_cpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.cpp)
message(STATUS "file_cpp=${file_cpp}")

add_definitions(
	-DENABLE_AUCTION
	-DACCESSI
	-DLOG_DEBUG
	-DLOG
	-DLOG_DEBUG1
	-DALAR
	-DNETBSD
	-DLINUX
	-DIMPL_SECURITY
	-DNO_REGISTER
)
#Switches
add_compile_options(
	-ggdb3 
	-fpermissive
	-finline-functions
	-fno-strength-reduce
	-pipe
	-pg
	-std=c++11
)
configure_file(config.hpp.in config.hpp NEWLINE_STYLE UNIX)
add_custom_command(OUTPUT release.hpp COMMAND ../genrelease.sh release.hpp )
add_executable(myst ${file_cpp} ${file_hpp} release.hpp)
#add_executable(info version.cpp version.hpp release.hpp)
#target_compile_definitions(info PUBLIC -DSTANDALONE)
add_compile_options(${MYSQL_STATIC_CFLAGS} )
add_compile_options(${SQLITE_STATIC_CFLAGS} )
target_link_libraries(myst  ${MYSQL_STATIC_LDFLAGS})
target_link_libraries(myst  ${SQLITE_STATIC_LDFLAGS})
target_link_libraries(myst ${CRYPT})
#Warning managements
target_compile_options(myst PUBLIC
#	-Werror 
	-Wall 
	-Wextra 
#	-pedantic 
	-Wno-write-strings
	-Wcast-align 
	-Wcast-qual 
	-Wctor-dtor-privacy 
	-Wdisabled-optimization 
	-Wformat=2 
	-Winit-self 
	-Wlogical-op 
	-Wmissing-include-dirs 
	-Wnoexcept 
#	-Wold-style-cast 
	-Woverloaded-virtual 
	-Wredundant-decls 
	-Wshadow 
	-Wsign-promo 
	-Wstrict-null-sentinel 
	-Wstrict-overflow=5 
	-Wundef 
	-Wno-unused 
	-Wno-variadic-macros 
	-Wno-parentheses 
	-fdiagnostics-show-option
)
get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#add_custom_command(TARGET info PRE_BUILD COMMAND ../astyle --options=../astylerc *.?pp)
#add_custom_command(TARGET myst PRE_LINK COMMAND echo "Before linking")
#add_custom_command(TARGET myst POST_BUILD COMMAND rm -f release.hpp)
#add_custom_command(TARGET myst POST_BUILD COMMAND ./info)
#add_custom_command(TARGET myst POST_BUILD COMMAND mv -f myst ../mudroot/myst)
set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
