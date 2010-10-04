VERSION=0.1
UNAME=$(shell uname)
DEBUG=1
INSTALL=install
PREFIX=/usr
MANPREFIX = ${PREFIX}/share/man

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
