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
file(GLOB file_hpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.h*)
message(STATUS "file_hpp=${file_hpp}")
file(GLOB file_cpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.cpp)
message(STATUS "file_cpp=${file_cpp}")
set(CMAKE_VERBOSE_MAKEFILE on)
add_definitions(
	-DENABLE_AUCTION
	-DACCESSI
	-DLOG_DEBUG
	-DLOG
	-DLOG_DEBUG1
	-DALAR
	-DNETBSD
	-DLINUX
)
add_compile_options(
	-ggdb3 
	-fpermissive
	-finline-functions
	-fno-strength-reduce
	-pipe
	-pg
)
add_compile_options(
	-Wno-write-strings
	-Wno-unused
	-Wno-trigraphs
	-Wno-format
	-Wno-int-to-pointer-cast
	-Wno-endif-labels
)
configure_file(config.h.in config.h NEWLINE_STYLE UNIX)
add_executable(myst ${file_cpp} ${file_hpp})
target_compile_options(myst PUBLIC ${MYSQL_STATIC_CFLAGS} )
target_compile_options(myst PUBLIC ${SQLITE_STATIC_CFLAGS} )
target_link_libraries(myst  ${MYSQL_STATIC_LDFLAGS})
target_link_libraries(myst  ${SQLITE_STATIC_LDFLAGS})
target_link_libraries(myst ${CRYPT})
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
add_custom_command(TARGET myst PRE_BUILD COMMAND echo "Before building could update revision here")
add_custom_command(TARGET myst PRE_LINK COMMAND echo "Before linking")
add_custom_command(TARGET myst POST_BUILD COMMAND echo "End, maybe we can move files around here: ${CMAKE_SOURCE_DIR}")
