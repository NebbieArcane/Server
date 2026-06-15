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
			COMMENT "Generating  autoenums"
	)
	ADD_CUSTOM_TARGET(autoenums
			DEPENDS ${files}
			COMMENT "Checking if re-generation is required for autoenums")

	ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} autoenums)

	target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${files})
ENDMACRO()
MACRO(RELEASE)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${CMAKE_SOURCE_DIR}/src/release.hpp
			COMMAND ${CMAKE_SOURCE_DIR}/genrelease.sh ${CMAKE_SOURCE_DIR}/src/release.hpp
			DEPENDS ${CMAKE_SOURCE_DIR}/lastcommit
			COMMENT "Updating release.hpp"
	)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${CMAKE_SOURCE_DIR}/lastcommit notexists
			COMMAND ${CMAKE_SOURCE_DIR}/genlastcommit.sh ${CMAKE_SOURCE_DIR}/lastcommit
			COMMENT "Updating lastcommit"
	)
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
			--profile boost/date-time
			--std c++17
			-m dynamic
			-d common
			-d ${dbtype}

			--generate-query
			--generate-prepared
			--show-sloc
			--generate-session
			--schema-format sql
			--at-once
			--schema-name ${dbschema}-schema

			--input-name ${CMAKE_PROJECT_NAME}
			-o ${CMAKE_SOURCE_DIR}/src/odb
			${CMAKE_SOURCE_DIR}/src/odb/${header}.hpp
			DEPENDS ${CMAKE_SOURCE_DIR}/src/odb/${header}.hpp
			COMMENT "Generate database support code for ${header}.hpp"
	)
	ADD_CUSTOM_TARGET(${header}.hpp DEPENDS ${ODB_GENERATED_FILES}
			COMMENT "Checking if classes regeneration is required for ${header}.hpp"
	)
	ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME}
			${header}.hpp
	)
ENDMACRO()
MACRO(ODB_GENERATE_DB dbtype dbschema)
	#	ODB_GENERATE(${dbtype} ${dbschema})
	SET(cxxFile 	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb.cxx")
	SET(hxxFile 	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb.hxx")
	SET(ixxFile 	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb.ixx")
	SET(cxxSchema	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb-${dbtype}.cxx")
	SET(hxxSchema	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb-${dbtype}.hxx")
	SET(ixxSchema	"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-odb-${dbtype}.ixx")
	SET(Schema		"${CMAKE_SOURCE_DIR}/src/odb/${dbschema}-schema-${dbtype}.cxx")
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${cxxFile} ${cxxSchema} ${Schema})
	# account-schema-*.cxx: rigenerare a mano (odb --generate-schema) quando cambia la versione modello;
	# l'aggiunta di valori ENUM non è gestita automaticamente dall'ODB compiler.
	SET(ODB_GENERATED_FILES ${cxxFile} ${cxxSchema})

	ADD_CUSTOM_COMMAND(
			OUTPUT ${ODB_GENERATED_FILES}
			COMMAND odb
			--profile boost/smart-ptr
			--profile boost/date-time
			--std c++17
			-m dynamic
			-d common
			-d ${dbtype}
			--generate-query
			--generate-prepared
			--show-sloc
			--generate-session
			--generate-schema
			--omit-drop
			--schema-format separate			
			--at-once
			--schema-name ${dbschema}
			--input-name ${dbschema}
			${dbschema}.hpp
			DEPENDS ${CMAKE_SOURCE_DIR}/src/odb/${dbschema}.hpp
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/odb/
			COMMENT "Generating  ${ODB_GENERATED_FILES}"
	)
	ADD_CUSTOM_TARGET(${dbschema}
			DEPENDS ${ODB_GENERATED_FILES}
			COMMENT "Checking if ODB Files need to be regenerated from ${dbschema}.hpp"
	)
	ADD_DEPENDENCIES(${CMAKE_PROJECT_NAME} ${dbschema}

	)
ENDMACRO()

enable_language(CXX)
set(MYSQL_USER $ENV{MYSQL_USER})
set(MYSQL_PASSWORD $ENV{MYSQL_PASSWORD})
set(MYSQL_HOST $ENV{MYSQL_HOST})
set(MYSQL_DB $ENV{MYSQL_DB})
set(MYSQL_PORT $ENV{MYSQL_PORT})

include(FindPkgConfig)
#include(${CMAKE_SOURCE_DIR}/cmake/Modules/PrecompiledHeader.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Ambiente target: release, master, devel, vagrant (build.sh esporta ENVIRONMENT).
if(DEFINED ENV{ENVIRONMENT} AND NOT "$ENV{ENVIRONMENT}" STREQUAL "")
	set(ENVIRONMENT "$ENV{ENVIRONMENT}")
else()
	set(ENVIRONMENT "vagrant")
endif()
message(STATUS "Myst build environment: ${ENVIRONMENT} (config.hpp: env_${ENVIRONMENT})")

# LOGICA CONDIZIONALE PER L'OTTIMIZZAZIONE:
# env_<target> viene da config.hpp (configure_file), non duplicare con -D qui.
if (NOT "$ENV{DOCKER_BUILD}" STREQUAL "")
	# Docker: Disabilita l'ottimizzazione (-O0),
	# e AGGIUNGE -DNDEBUG per disattivare le istruzioni assert() (handler.cpp crash).
	message(STATUS "DOCKER BUILD: Setting CXX flags to -O0 and disabling assertions (NDEBUG).")
	set(CMAKE_CXX_FLAGS "-std=c++17 -ggdb3 -pipe -pg -O0 -DNDEBUG")
	set(CMAKE_C_FLAGS "-std=gnu11 -ggdb3 -pipe -pg -O0 -DNDEBUG")
else()
	# Vagrant/Locale: Usa i flag di ottimizzazione originali (compatibilità)
	message(STATUS "LOCAL BUILD: Using original optimization flags.")
	set(CMAKE_CXX_FLAGS "-std=c++17 -ggdb3 -finline-functions -fno-strength-reduce -pipe -pg")
	set(CMAKE_C_FLAGS "-std=gnu11 -ggdb3 -finline-functions -fno-strength-reduce -pipe -pg")
endif()
# FINE LOGICA CONDIZIONALE

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

# Object files: build/src/CMakeFiles/myst.dir/odb/ (ODB sources live under src/odb/).
# PRE_BUILD is ignored by Makefile generators; use a dependency target instead.
set(MYST_OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME}.dir")
file(MAKE_DIRECTORY "${MYST_OBJ_DIR}")
file(MAKE_DIRECTORY "${MYST_OBJ_DIR}/odb")
add_custom_target(myst_obj_dirs ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory "${MYST_OBJ_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${MYST_OBJ_DIR}/odb"
		COMMENT "Ensure myst object output directories exist"
)
add_dependencies(${CMAKE_PROJECT_NAME} myst_obj_dirs)

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
pkg_check_modules(MYSQL
		mysqlclient)
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

# Generate configuration file
configure_file(config.hpp.in ${CMAKE_SOURCE_DIR}/src/config.hpp NEWLINE_STYLE UNIX)
# Generate release version


#Warning managements
target_link_libraries(${CMAKE_PROJECT_NAME} -g -gdwarf-3)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
		-Werror
		-Wfatal-errors
		-Wall
		-Wextra
		-Wcast-align
		-Wcast-qual
		-Wno-ctor-dtor-privacy
		-Wdisabled-optimization
		-Wenum-compare
		-Wno-format-nonliteral
		-Wformat=2
		-Wformat-overflow
		-Wimplicit-fallthrough=3
		-Winit-self
		-Wlogical-op
		-Wmissing-include-dirs
		-Wnoexcept
		-Wno-old-style-cast
		-Woverloaded-virtual
		-Wparentheses
		-Wpedantic
		-Wredundant-decls
		-Wreturn-local-addr
		-Wreturn-type
		-Wno-sign-compare
		-Wsign-promo
		#	-Wstack-protector
		-Wstrict-null-sentinel
		-Wstrict-overflow=5
		#	-Wswitch
		-Wundef
		-Wno-unknown-pragmas
		#	-Wno-unused
		-Wno-unused-parameter
		-Wno-variadic-macros
		-fdelete-null-pointer-checks
		-fdiagnostics-show-option
		-fdiagnostics-color=always
		-fstack-protector


)
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# Opt-in only: set RELAX_WARNINGS=1 to disable -Werror (e.g. warning cleanup loops).
if (NOT "$ENV{RELAX_WARNINGS}" STREQUAL "")
	message(STATUS "RELAX_WARNINGS: overriding -Werror and -Wfatal-errors.")
	target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
			-Wno-error
			-Wno-fatal-errors
	)
endif()

#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_LINK COMMAND echo "Before linking")
#add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND rm -f release.hpp)
add_custom_command(
		TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_PROJECT_NAME} -V && cp -f ${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/mudroot/${CMAKE_PROJECT_NAME}
)
RELEASE()
ODB_GENERATE_DB(mysql account)
AUTOENUMS()