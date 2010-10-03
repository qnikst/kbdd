VERSION=0.1
UNAME=$(shell uname)
DEBUG=1
INSTALL=install
PREFIX=/usr
ifeq ($(PREFIX),/usr)
SYSCONFDIR=/etc
else
SYSCONFDIR=$(PREFIX)/etc
endif

CFLAGS += -Wall
CFLAGS += -pipe
