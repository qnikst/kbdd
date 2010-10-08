VERSION=0.1
PACKAGE_NAME=kbdd
PACKAGE_VERSION=$(VERSION)
UNAME=$(shell uname)
DEBUG=1
INSTALL=install
PREFIX=/usr
MANPREFIX = ${PREFIX}/share/man
DBUSDIR = ${PREFIX}/share/dbus-1

#options
WITH_DBUS=1

ifeq ($(PREFIX),/usr)
SYSCONFDIR=/etc
else
SYSCONFDIR=$(PREFIX)/etc
endif

CFLAGS += -std=c99
CFLAGS += -Wall
#CFLAGS += -Wextra
#CFLAGS += -fno-strict-overflow
#CFLAGS += -fstrict-aliasing
#CFLAGS += -pedantic-errors
#CFLAGS += -Wformat=2
#CFLAGS += -Winit-self
#CFLAGS += -Wstrict-overflow=5
#CFLAGS += -Wwrite-strings
#CFLAGS += -Wconversion
#CFLAGS += -Waggregate-return
CFLAGS += -pipe
