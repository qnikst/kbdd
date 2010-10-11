include ../common.mk

pkg_packages := dbus-1 dbus-glib-1 glib-2.0

PKG_CFLAGS := $(shell pkg-config --cflags $(pkg_packages))
PKG_LDFLAGS := $(shell pkg-config --libs $(pkg_packages))

ADD_CFLAGS := -g -Wall -DG_DISABLE_DEPRECATED

ADD_CFLAGS += -DNO_DAEMON

CFLAGS  := $(PKG_CFLAGS) $(ADD_CFLAGS) $(CFLAGS)
LDFLAGS := $(PKG_LDFLAGS) $(LDFLAGS)

all: subsystems kbdd

kbdd: kbdd.o storage.o libkbdd.o dbus
	$(CC) $(LDFLAGS) -lX11 -lpthread kbdd.o storage.o libkbdd.o dbus/server.o -o $@

storage.o: storage.c
	$(CC) $(CFLAGS) -DSTORAGE_GHASH -c -o $@ $<

libkbdd.o: libkbdd.c
	$(CC) $(CFLAGS) -lX11 -DDEBUG -c -o $@ $<

kbdd.o: kbdd.c
	$(CC) $(CFLAGS) -DPROGRAM_NAME=${PACKAGE_NAME} -DENABLE_DBUS -DDEBUG -c -o $@ $<

subsystems: 
	cd dbus && $(MAKE)

clean: 
	cd dbus && $(MAKE) clean
	$(RM) *.o kbdd

