AC_INIT([myst], [3.1.7])
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_CC([g++])
AC_CONFIG_SRCDIR([src/comm.h])
AC_CHECK_FILE([src/configuration.hpp], ,[
	AC_MSG_ERROR([please copy src/configuration_default.hpp over src/configuration.hpp and edit it])
])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT