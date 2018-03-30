#src cmake
set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
#set(CMAKE_VERBOSE_MAKEFILE ON) #Uncomment to see actual c++ invocations
MACRO(AUTOENUMS)
	set(files ${CMAKE_SOURCE_DIR}/src/autoenums.hpp ${CMAKE_SOURCE_DIR}/src/autoenums.cpp)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${files}
		COMMAND ./codegen.sh
		DEPENDS ${CMAKE_SOURCE_DIR}/shutils/enums.json
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shutils/
        COMMENT "Generate autoenums")
    ADD_CUSTOM_TARGET(autoenums 
    	DEPENDS ${files}
        COMMENT "Checking if re-generation is required for autoenums")
        
    ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} autoenums)
    
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${files})
    
		
ENDMACRO()
MACRO(ODB_GENERATE dbtype header)
    SET(cxxFile "${CMAKE_SOURCE_DIR}/src/odb/${header}-odb.cxx")
    SET(hxxFile "${CMAKE_SOURCE_DIR}/src/odb/${header}-odb.hxx")
    SET(ixxFile "${CMAKE_SOURCE_DIR}/src/odb/${header}-odb.ixx")
    SET(sqlFile "${CMAKE_SOURCE_DIR}/src/odb/${header}.sql")
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${cxxFile})

    SET(ODB_GENERATED_FILES ${cxxFile} ${hxxFile} ${ixxFile} ${sqlFile})

    ADD_CUSTOM_COMMAND(
        OUTPUT ${ODB_GENERATED_FILES}
        COMMAND odb 
        		--profile boost/smart-ptr
        		--std c++11 
        		-d ${dbtype} 
        		--generate-query 
        		--generate-prepared
                --show-sloc
         		--generate-session       
                -o ${CMAKE_SOURCE_DIR}/src/odb
                ${CMAKE_SOURCE_DIR}/src/odb/${header}.hpp
        DEPENDS ${CMAKE_SOURCE_DIR}/src/odb/${header}.hpp
        COMMENT "Generate database support code for ${header}.hpp")

    ADD_CUSTOM_TARGET(${header}.hpp DEPENDS ${ODB_GENERATED_FILES}
                      COMMENT "Checking if classes regeneration is required for ${header}.hpp")

    ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} ${header}.hpp)
ENDMACRO()
MACRO(ODB_GENERATE_DB dbtype dbschema)
#	set(headers "${ARGN}")
#	foreach(header IN LISTS headers)
#	    message(STATUS "Generating ${header}")
#	    ODB_GENERATE(${dbtype} ${header})
#	endforeach()
	ODB_GENERATE(${dbtype} ${dbschema})
	set(schemaFile "${CMAKE_SOURCE_DIR}/src/odb/${CMAKE_PROJECT_NAME}-schema-${dbtype}.cxx")
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${schemaFile})
    ADD_CUSTOM_COMMAND(
        OUTPUT ${schemaFile}
        COMMAND odb  
        		--profile boost/smart-ptr
        		--std c++11
        		-m dynamic
        		-d common
        		-d ${dbtype} 
                --generate-schema-only
                --schema-format separate
                --at-once
                --schema-name ${dbschema}
                --input-name ${CMAKE_PROJECT_NAME}
                ${dbschema}.hpp
        DEPENDS ${CMAKE_SOURCE_DIR}/src/odb/${dbschema}.hpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/odb/
        COMMENT "Generate database schema")

    ADD_CUSTOM_TARGET(${dbschema} DEPENDS ${schemaFile}
                      COMMENT "Checking if schema regeration is required for ${schemaFile}")

    ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} ${dbschema})
	
ENDMACRO()
enable_language(CXX)
set (CMAKE_USE_RELATIVE_PATHS true)
set (MYSQL_USER $ENV{MYSQL_USER})
set (MYSQL_PASSWORD $ENV{MYSQL_PASSWORD})
set (MYSQL_HOST $ENV{MYSQL_HOST})
set (MYSQL_DB $ENV{MYSQL_DB})
message(STATUS "src")
foreach (_variableName PROJECT_SOURCE_DIR PROJECT_NAME CMAKE_SOURCE_DIR CMAKE_CXX_KNOWN_FEATURES CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
include(FindPkgConfig)
#include(${CMAKE_SOURCE_DIR}/cmake/Modules/PrecompiledHeader.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "-std=c++11 -ggdb3 -fpermissive -finline-functions -fno-strength-reduce -pipe -pg")
set(CMAKE_C_FLAGS "-std=c++11 -ggdb3 -fpermissive -finline-functions -fno-strength-reduce -pipe -pg")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -E -P -C -traditional-cpp")
#Retrieves headers
file(GLOB file_hpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.hpp)
list(SORT file_hpp)
#message(STATUS "file_hpp=${file_hpp}")

#Retrieves code
file(GLOB file_cpp LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR}/src/ *.cpp)
list(SORT file_cpp)
#list(REVERSE file_cpp)
#message(STATUS "file_cpp=${file_cpp}")

add_executable(${CMAKE_PROJECT_NAME}  ${file_cpp} ${file_hpp} release.hpp)
#add_precompiled_header(${CMAKE_PROJECT_NAME} pchheader.h FORCEINCLUDE)

#Boost
find_package( Boost COMPONENTS system filesystem program_options date_time REQUIRED )
include_directories(
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${Boost_LIBRARIES}
)
#libodb
pkg_check_modules( ODB libodb libodb-boost libodb-sqlite libodb-mysql REQUIRED )
include_directories(
	${ODB_INCLUDE_DIRS}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${ODB_LIBRARIES}
)

pkg_check_modules(LIBEVENT libevent_pthreads )
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${LIBEVENT_STATIC_CFLAGS} )
target_link_libraries(${CMAKE_PROJECT_NAME}  ${LIBEVENT_STATIC_LDFLAGS})

#Mysql CPPConnector
#find_package( MySQLConnectorCPP REQUIRED)
#include_directories( ${MySQLConnectorCPP_INCLUDE_DIRS})
#target_link_libraries(${CMAKE_PROJECT_NAME}  ${MySQLConnectorCPP_LIBRARIES})
#Mysql generic client lib
pkg_check_modules(MYSQL mysqlclient)
#target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${MYSQL_STATIC_CFLAGS} )
#target_link_libraries(${CMAKE_PROJECT_NAME}  ${MYSQL_STATIC_LDFLAGS})

#Log for apache
pkg_check_modules(LOG4 liblog4cxx)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${LOG4_STATIC_CFLAGS} )
target_link_libraries(${CMAKE_PROJECT_NAME}  ${LOG4_STATIC_LDFLAGS})

#Sqlite
pkg_check_modules(SQLITE sqlite3)
#target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${SQLITE_STATIC_CFLAGS} )
#target_link_libraries(${CMAKE_PROJECT_NAME}  ${SQLITE_STATIC_LDFLAGS})

#Curl
pkg_check_modules(CURL curlpp)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${CURL_STATIC_CFLAGS} )
target_link_libraries(${CMAKE_PROJECT_NAME}  ${CURL_STATIC_LDFLAGS})

#Libconfig
pkg_check_modules(CONF libconfig++)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${CONF_STATIC_CFLAGS} )
target_link_libraries(${CMAKE_PROJECT_NAME}  ${CONF_STATIC_LDFLAGS})

#Crypt
find_library(CRYPT crypt)
target_link_libraries(${CMAKE_PROJECT_NAME} ${CRYPT})

#set( MYSQL_HOST $ENV{MYSQL_HOST} )

# Generate configuration file
configure_file(config.hpp.in ${CMAKE_SOURCE_DIR}/src/config.hpp NEWLINE_STYLE UNIX)
# Generate release version
add_custom_command(OUTPUT release.hpp COMMAND ${CMAKE_SOURCE_DIR}/genrelease.sh ${CMAKE_SOURCE_DIR}/src/release.hpp )

#Warning managements
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
	-Wno-enum-compare
	-Wno-unknown-pragmas
	-Wno-variadic-macros 
	-Wno-unused-parameter
	-Werror
	-Wall 
	-Wextra 
#	-Wpedantic 
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
#	-Wsign-promo 
#	-Wstrict-null-sentinel 
#	-Wstrict-overflow=5 
#	-Wundef 
#   -Wno-unused 
#	-Wno-parentheses 
#	-fdiagnostics-show-option
#	-w
)
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    #message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
add_custom_target(style ${CMAKE_SOURCE_DIR}/astyle -r -Q --options=${CMAKE_SOURCE_DIR}/astylerc "./*.cpp,*.hpp")
#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_LINK COMMAND echo "Before linking")
#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND rm -f release.hpp)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_PROJECT_NAME} -V && cp -f ${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/mudroot/${CMAKE_PROJECT_NAME})
ODB_GENERATE_DB(mysql account)
AUTOENUMS()
