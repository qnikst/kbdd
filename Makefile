TOPDIR=$(shell pwd)

include $(TOPDIR)/common.mk

FILTER:=src/kbdd.c

FILES:=$(filter-out $(FILTER),$(wildcard src/*.c))
FILES:=$(FILES:.c=.o)
HEADERS:=$(wildcard include/*.h)

SRCDIR=$(TOPDIR)/src

GLIB_LIBS = `pkg-config glib-2.0 --libs`
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags`
GTK_LIBS = `pkg-config gtk+-2.0 --libs`
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`


all: ${FILES} src/kbdd src/libkbdd.so

options:
	@echo libkbdd build options
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC}"

src/storage.o:
	$(CC) $(CFLAGS) ${SRCDIR}/storage.c -c -o storage.o ${GLIB_LIBS} ${GLIB_CFLAGS} -DSTORAGE_GHASH -fPIC

src/libkbdd.o: src/storage.o
	@echo "compile libkbdd"
	$(CC) $(CFLAGS) ${SRCDIR}/libkbdd.c storage.o -c -o libkbdd.o -lX11 -fPIC

src/libkbdd.so: src/libkbdd.o
	$(CC) $(CFLAGS) -shared libkbdd.o storage.o -o libkbdd.so ${GLIB_LIBS} ${GLIB_CFLAGS} -lX11 -fPIC

src/kbdd: src/libkbdd.o
	$(CC) $(CFLAGS) ${SRCDIR}/kbdd.c libkbdd.o storage.o -o kbdd ${GLIB_LIBS} ${GLIB_CFLAGS} -lX11


install: all
	echo "INSTALL"
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0755 kbdd $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 libkbdd.so $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 src/libkbdd.h $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -m 0644 kbdd.1 ${DESTDIR}${MANPREFIX}/man1/kbdd.1

clean: 
	rm -f *.o *.so kbdd
