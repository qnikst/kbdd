if ENABLE_DBUS
SUBDIRS = dbus
endif
bin_PROGRAMS = kbdd
kbdd_SOURCES = kbdd.c perwindow.c libkbdd.c

EXTRA_DIST = perwindow.h libkbdd.h common-defs.h keys.h
kbdd_LDADD = $(GLIB_LIBS) $(X11_LIBS) 
AM_CFLAGS = $(GLIB_CFLAGS) $(DBUSGLIB_CFLAGS) -Idbus/

if ENABLE_DBUS
kbdd_LDADD += $(DBUSGLIB_LIBS) dbus/libkbdddbus.a
AM_CFLAGS += $(DBUSGLIB_CFLAGS)
endif
