GLIB_LIBS = `pkg-config glib-2.0 --libs`
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags`
GTK_LIBS = `pkg-config gtk+-2.0 --libs`
GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`


all: test
	echo "all"

storage:
	$(CC) storage.c -g -c -o storage.o ${GLIB_LIBS} ${GLIB_CFLAGS} -lX11 -DSTORAGE_GHASH 

libkbdd: storage
	$(CC) libkbdd.c storage.o -c -o libkbdd.o ${GLIB_LIBS} ${GLIB_CFLAGS} -lX11

test: libkbdd
	$(CC) test.c libkbdd.o storage.o ${GLIB_LIBS} ${GLIB_CFAGS} -lX11

