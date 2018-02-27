#src cmake
set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
cmake_minimum_required (VERSION 3.5)
project (Alarmud)
include(FindPkgConfig)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb3 -fpermissive 	-finline-functions 	-fno-strength-reduce -pipe -pg")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -E -x c++ -P -C -traditional-cpp")
#Retrieves headers
file(GLOB file_hpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.hpp)
list(SORT file_hpp)
message(STATUS "file_hpp=${file_hpp}")

#Retrieves code
file(GLOB file_cpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.cpp)
list(SORT file_cpp)
message(STATUS "file_cpp=${file_cpp}")

add_executable(myst ${file_cpp} ${file_hpp} release.hpp)
add_executable(info version.cpp release.hpp)
target_compile_definitions(info PUBLIC STANDALONE)

#Boost
find_package( Boost COMPONENTS system filesystem program_options REQUIRED )
include_directories(
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(myst
    ${Boost_LIBRARIES}
)

#Mysql CPPConnector
find_package( MySQLConnectorCPP REQUIRED)
include_directories( ${MySQLConnectorCPP_INCLUDE_DIRS})
target_link_libraries(myst  ${MySQLConnectorCPP_LIBRARIES})
#Mysql generic client lib
pkg_check_modules(MYSQL mysqlclient)
target_compile_options(myst PUBLIC ${MYSQL_STATIC_CFLAGS} )
target_link_libraries(myst  ${MYSQL_STATIC_LDFLAGS})

#Log for apache
pkg_check_modules(LOG4 liblog4cxx)
target_compile_options(myst PUBLIC ${LOG4_STATIC_CFLAGS} )
target_link_libraries(myst  ${LOG4_STATIC_LDFLAGS})

#Sqlite
pkg_check_modules(SQLITE sqlite3)
target_compile_options(myst PUBLIC ${SQLITE_STATIC_CFLAGS} )
target_link_libraries(myst  ${SQLITE_STATIC_LDFLAGS})

#Curl
pkg_check_modules(CURL curlcpp)
target_compile_options(myst PUBLIC ${CURL_STATIC_CFLAGS} )
target_link_libraries(myst  ${CURL_STATIC_LDFLAGS})

#Libconfig
pkg_check_modules(CONF libconfig++)
target_compile_options(myst PUBLIC ${CONF_STATIC_CFLAGS} )
target_link_libraries(myst  ${CONF_STATIC_LDFLAGS})

#Crypt
find_library(CRYPT crypt)
target_link_libraries(myst ${CRYPT})

#set( MYSQL_HOST $ENV{MYSQL_HOST} )

# Generate configuration file
configure_file(config.hpp.in config.hpp NEWLINE_STYLE UNIX)
# Generate release version
add_custom_command(OUTPUT release.hpp COMMAND ../genrelease.sh release.hpp )

#Warning managements
target_compile_options(myst PUBLIC
	-Werror 
	-w
#	-Wall 
#	-Wextra 
#	-pedantic 
	-Wwrite-strings
#	-Wcast-align 
#	-Wcast-qual 
#	-Wctor-dtor-privacy 
#	-Wdisabled-optimization 
#	-Wformat=2 
#	-Winit-self 
#	-Wlogical-op 
#	-Wmissing-include-dirs 
#	-Wnoexcept 
#	-Wold-style-cast 
#	-Woverloaded-virtual 
#	-Wredundant-decls 
	-Wshadow 
#	-Wsign-promo 
#	-Wstrict-null-sentinel 
#	-Wstrict-overflow=5 
#	-Wundef 
#    -Wno-unused 
#	-Wno-variadic-macros 
#	-Wno-parentheses 
#	-fdiagnostics-show-option
)
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    #message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
add_custom_command(TARGET info PRE_BUILD COMMAND ../astyle --options=../astylerc *.?pp)
#add_custom_command(TARGET myst PRE_LINK COMMAND echo "Before linking")
#add_custom_command(TARGET myst POST_BUILD COMMAND rm -f release.hpp)
add_custom_command(TARGET myst POST_BUILD COMMAND ./info)
add_custom_command(TARGET myst POST_BUILD COMMAND cp -f myst ../mudroot/myst)
set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
