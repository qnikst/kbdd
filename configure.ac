#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([kbdd], [0.5], [qnikst@gentoo.ru])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([kbdd], [0.5])
AC_SUBST(PACKAGE_URL, [http://github.com/qnikst/kbdd] )

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

#check for pkg-config
PKG_PROG_PKG_CONFIG

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])
AC_PATH_X

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
PKG_CHECK_MODULES(X11, x11, [], [AC_MSG_ERROR([libX11 not found])])
AC_SUBST(X11_LIBS)

AC_ARG_ENABLE([dbus], 
        AS_HELP_STRING([--enable-dbus], [Enable providing D-BUS object [default=enabled]]),
        [],
        enableval=yes)
if test "x$enableval" == 'xyes'; then
    AC_DEFINE([ENABLE_DBUS], [], [Enable providing d-bus object])

    #Check for dbus-glib
    GLIB_GENMARSHAL=`$PKG_CONFIG glib-2.0 --variable=glib_genmarshal`
    AC_SUBST(GLIB_GENMARSHAL)
    
    PKG_CHECK_MODULES([DBUSGLIB], [dbus-glib-1 >= 0.60], [], [AC_MSG_ERROR([Dbus-glib not found])])
    AC_SUBST(DBUSGLIB_LIBS)
    AC_SUBST(DBUSGLIB_CFLAGS)
fi
AM_CONDITIONAL(BUILD_DBUS, test "x$enableval" == 'xyes')


AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [Enable verbose output [default=disabled]]),
        [],
        enableval=no)
if test "x$enableval" == 'xyes'; then
    AC_DEFINE([DEBUG], [], [Enable verbose mode]),
fi

#check for Glib
PKG_CHECK_MODULES([GLIB], [glib-2.0], [], [AC_MSG_ERROR([Glib not found])])
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

AC_OUTPUT(Makefile src/Makefile src/dbus/Makefile)
