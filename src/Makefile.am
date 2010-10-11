SUBDIRS = dbus
bin_PROGRAMS = kbdd
kbdd_SOURCES = kbdd.c storage.c libkbdd.c
kbdd_LDADD = $(GLIB_LIBS) $(X11_LIBS)
AM_CFLAGS = $(GLIB_CFLAGS) -DSTORAGE_GHASH
