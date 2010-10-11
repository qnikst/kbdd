TOPDIR=$(shell pwd)

include $(TOPDIR)/common.mk

all: subsystem

options:
	@echo COMPILINT ${PROGRAM_NAME}
	@echo VERSION   ${VERSION}
	@echo DEBUG     ${DEBUG}

subsystem: options
	export PROGRAM_NAME
	export PROGRAM_VERSION
	export DEBUG
	cd src/ && $(MAKE)

install: all
	echo "INSTALL"
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0755 kbdd $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 libkbdd.so $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 0644 src/libkbdd.h $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -m 0644 kbdd.1 ${DESTDIR}${MANPREFIX}/man1/kbdd.1
	@echo "installing dbus files"
	$(INSTALL) -d -m 0755 $(DESTDIR)$(DBUSDIR)/services
	$(INSTALL) -m 0644 data/ru.gentoo.kbdd $(DESTDIR)$(DBUSDIR)/services


clean: 
	cd src/ && $(MAKE) clean
