SUBDIRS = dbus
bin_PROGRAMS = kbdd
kbdd_SOURCES = kbdd.c storage.c libkbdd.c
kbdd_LDADD = $(GLIB_LIBS) $(DBUSGLIB_LIBS) $(X11_LIBS) dbus/libkbdddbus.a
AM_CFLAGS = $(GLIB_CFLAGS) $(DBUSGLIB_CFLAGS) -DSTORAGE_GHASH -Idbus/
