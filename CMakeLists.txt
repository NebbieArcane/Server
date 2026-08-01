# Root cmake
cmake_minimum_required (VERSION 3.5)
project (myst)
enable_language(CXX)

# IMPOSTAZIONE C++ STANDARD (Necessaria per ODB su Bionic)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

# helptbl: C++ builder + content stamp (skip rewrite if help_src unchanged)
# Output dir includes CMAKE_SYSTEM_NAME so a host Mac build/ tree shared into
# Docker does not leave a Mach-O helptbl_builder that fails with Exec format error.
add_executable(helptbl_builder tools/helptbl_builder.cpp)
set_target_properties(helptbl_builder PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/${CMAKE_SYSTEM_NAME}"
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
set(HELP_SRC_DIR "${CMAKE_SOURCE_DIR}/pages/help_src")
set(HELP_OUT "${CMAKE_SOURCE_DIR}/pages/helptbl")
set(HELP_STAMP "${CMAKE_SOURCE_DIR}/pages/helptbl.stamp")
add_custom_target(helptbl ALL
	COMMAND $<TARGET_FILE:helptbl_builder>
		"${HELP_SRC_DIR}/MANIFEST"
		"${HELP_SRC_DIR}"
		"${HELP_OUT}"
		"${HELP_STAMP}"
	DEPENDS helptbl_builder
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Refreshing pages/helptbl if help_src content changed"
	VERBATIM
)

# wizhelptbl: same builder, sources under pages/wizhelp_src/
set(WIZHELP_SRC_DIR "${CMAKE_SOURCE_DIR}/pages/wizhelp_src")
set(WIZHELP_OUT "${CMAKE_SOURCE_DIR}/pages/wizhelptbl")
set(WIZHELP_STAMP "${CMAKE_SOURCE_DIR}/pages/wizhelptbl.stamp")
add_custom_target(wizhelptbl ALL
	COMMAND $<TARGET_FILE:helptbl_builder>
		"${WIZHELP_SRC_DIR}/MANIFEST"
		"${WIZHELP_SRC_DIR}"
		"${WIZHELP_OUT}"
		"${WIZHELP_STAMP}"
	DEPENDS helptbl_builder
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Refreshing pages/wizhelptbl if wizhelp_src content changed"
	VERBATIM
)

add_subdirectory(src)
add_dependencies(myst helptbl wizhelptbl)

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