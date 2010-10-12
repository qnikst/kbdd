SUBDIRS = dbus
bin_PROGRAMS = kbdd
kbdd_SOURCES = kbdd.c storage.c libkbdd.c
kbdd_LDADD = $(GLIB_LIBS) $(DBUSGLIB_LIBS) $(X11_LIBS)
kbdd_CFLAGS = -o ./dbus/libkbdddbus.a
AM_CFLAGS = $(GLIB_CFLAGS) $(DBUSGLIB_CFLAGS) -I./dbus/ -DSTORAGE_GHASH 
