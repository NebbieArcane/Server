AC_PREREQ([2.69])
AC_INIT([myst], [3.1.7])
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_CC([g++])
AC_PROG_MAKE_SET
AC_CONFIG_SRCDIR([src/comm.h])
AC_CHECK_FILE([src/configuration.hpp], ,[
	AC_MSG_ERROR([please cp src/configuration_default.hpp src/configuration.hpp
and edit it])
])
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypt':
AC_CHECK_LIB([crypt], [main])
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lgdbm':
AC_CHECK_LIB([gdbm], [main])
# FIXME: Replace `main' with a function in `-lmysqlclient':
AC_CHECK_LIB([mysqlclient], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lresolv':
AC_CHECK_LIB([resolv], [main])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])
# FIXME: Replace `main' with a function in `-lz':
AC_CHECK_LIB([z], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero gethostbyaddr gethostbyname gethostname getpagesize gettimeofday isascii memset modf pow select socket strcasecmp strchr strdup strerror strncasecmp strstr])

AC_CONFIG_HEADERS([src/config.hpp])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	utilities/Makefile	
])
AC_OUTPUT