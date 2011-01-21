#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([cpusb], [0.001], [willian@ufpa.br])
AC_CONFIG_SRCDIR([src/cpusb.c])
AC_CONFIG_FILES([doxyfile])

# Checks for programs.
AC_PROG_CC([gcc cc])
AC_CHECK_PROGS([DOXYGEN], [doxygen])

# Checks for libraries.
AC_CHECK_LIB([confuse], [read_option])
AC_CHECK_LIB([readline], [install_conf])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd mkdir strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
