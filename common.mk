# Program definition
VERSION=0.1
PACKAGE_NAME=KBDD
PACKAGE_VERSION=$(VERSION)

# System options
UNAME=$(shell uname)
INSTALL=install
PREFIX=/usr
MANPREFIX = ${PREFIX}/share/man
DBUSDIR = ${PREFIX}/share/dbus-1
ifeq ($(PREFIX),/usr)
SYSCONFDIR=/etc
else
SYSCONFDIR=$(PREFIX)/etc
endif

#build options
DEBUG=1
WITH_DBUS=1

#options

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
