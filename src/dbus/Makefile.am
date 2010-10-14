DBUS_PREFIX=m_kbdd_service

interfacesdir = $(datadir)/dbus-1/interfaces
interfaces_DATA = $(INTERFACE_XML)

kbdd-service-glue.h: $(INTERFACE_XML)
	dbus-binding-tool --mode=glib-server --prefix=${DBUS_PREFIX} $< > $@

m-kbdd-service-marshal.o: kbdd-marshal.list
	$(GLIB_GENMARSHAL) --prefix=kbdd_marshal $< --header > xgen-gmh \
	&& (cmp -s xgen-gmh $@ || cp xgen-gmh $@ ) \
	&& rm -f xgen-gmh xgen-gmh~

m-kbdd-service-marshal.c: kbdd-marshal.list
	$(GLIB_GENMARSHAL) --prefix=kbdd_marshal $< --body > xgen-gmc \
	&& cp xgen-gmc $@ \
	&& rm -f xgen-gmc xgen-gmc~

noinst_HEADERS = $(BUILT_SOURCES)

noinst_LIBRARIES = libkbdddbus.a
libkbdddbus_a_SOURCES = m-kbdd-service.c m-kbdd-service-marshal.c

BUILT_SOURCES = m-kbdd-service-marshal.o m-kbdd-service-marshal.c  kbdd-service-glue.h 

AM_CFLAGS = $(GLIB_CFLAGS) $(DBUSGLIB_CFLAGS)

CLEANFILES = $(BUILT_SOURCES) 
EXTRA_DIST = kbdd-service-interface.xml kbdd-marshal.list
INTERFACE_XML = kbdd-service-interface.xml

