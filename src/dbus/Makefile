interface_xml := kbdd-service-interface.xml

pkg_packages := dbus-1 dbus-glib-1 glib-2.0

PKG_CFLAGS := $(shell pkg-config --cflags $(pkg_packages))
PKG_LDFLAGS := $(shell pkg-config --libs $(pkg_packages))

ADD_CFLAGS := -g -Wall -DG_DISABLE_DEPRECATED
# Only for testing purposes
ADD_CFLAGS += -DNO_DAEMON

CFLAGS  := $(PKG_CFLAGS) $(ADD_CFLAGS) $(CFLAGS)
LDFLAGS := $(PKG_LDFLAGS) $(LDFLAGS)


#Specify all tools used in project
GLIB_GENMARSHAL:= $(shell pkg-config glib-2.0 --variable=glib_genmarshal)
DBUS_BINDING_TOOL:=dbusbindingtool


#Specify constants
DBUS_PREFIX=m_kbdd_service

cleanfiles := kbdd-service-glue.h \
			  m-kbdd-service-marshal.h \
			  m-kbdd-service-marshal.c

targets = server

.PHONY: all clean checkxml
all: $(targets)

# File where all marshal functions should located
kbdd-marshal.list:

server: server.o
	$(CC) $* -o $@ $(LDFLAGS)

server.o: m-kbdd-service.c m-kbdd-service-marshal.o kbdd-service-glue.h
	$(CC) $(CFLAGS) -c -o $@ $<


m-kbdd-service.o: 
	$(CC) $(CFLAGS) -c -o $@ $<

m-kbdd-service-marshal.o: m-kbdd-service-marshal.c m-kbdd-service-marshal.h
	$(CC) $(CFLAGS) -c -o $@ $<

m-kbdd-service-marshal.h: kbdd-marshal.list
	@echo "generating kbdd-marshal header"
	$(GLIB_GENMARSHAL) --prefix=kbdd_marshal $< --header > xgen-gmh \
	           && (cmp -s xgen-gmh $@ || cp xgen-gmh $@ ) \
			   && rm -f xgen-gmh xgen-gmh~

m-kbdd-service-marshal.c: kbdd-marshal.list
	@echo "generating kbdd-marshal source"
	$(GLIB_GENMARSHAL) --prefix=kbdd_marshal $< --body > xgen-gmc \
				   && cp xgen-gmc $@ \
				   && rm -f xgen-gmc xgen-gmc~

# If interface XML changes, glue interfaces will be automatically regenerated
# Normallyy this would also mean that your builds would fail after this since
# you'd be missign implementation code.
kbdd-service-glue.h: $(interface_xml)
	$(DBUS_BINDING_TOOL) --mode=glib-server --output=$@ --prefix=${DBUS_PREFIX} $< > $@

#run DTD validation on the interface XML. 
checkxml: $(interface_xml)
	@xmllint --valid --noout $<
	@echo $< checks out ok

clean:
	$(RM) $(targets) $(cleanfiles) *.o

server.o client.o: Makefile
