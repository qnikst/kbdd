#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([kbdd], [0.2], [qnikst@gentoo.ru])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([kbdd], [0.1])
AC_SUBST(PACKAGE_URL, [http://github.com/qnikst/kbdd] )
CFLAGS="$CFLAGS"

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_CC_C_O
DBUS_REQUIRED=0.60

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [XNextEvent], [X11_LIBS=[-lX11]], [AC_MSG_ERROR([libX11 not found])])
AC_SUBST(X11_LIBS)

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

AC_ARG_ENABLE([dbus],
        AS_HELP_STRING([--enable-dbus], [Enable providing D-BUS object]),
        [],
        enableval=yes)
if test x$enableval == xyes; then
    AC_DEFINE([ENABLE_DBUS], [], [Enable providing d-bus object])
fi


# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([strerror])

PKG_CHECK_MODULES([GLIB], [glib-2.0], [], [AC_MSG_ERROR([Glib not found])])
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

GLIB_GENMARSHAL=`$PKG_CONFIG glib-2.0 --variable=glib_genmarshal`
AC_SUBST(GLIB_GENMARSHAL)

PKG_CHECK_MODULES([DBUSGLIB], [dbus-glib-1], [], [AC_MSG_ERROR([Dbus-glib not found])])
AC_SUBST(DBUSGLIB_LIBS)
AC_SUBST(DBUSGLIB_CFLAGS)
AC_OUTPUT(Makefile src/Makefile src/dbus/Makefile)
