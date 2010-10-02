#ifndef _STORAGE_H_
#define _STORAGE_H_

#ifdef STORAGE_GHASH
#include <glib.h>
#endif

#include <X11/Xlib.h>

void _kbdd_storage_init();

void _kbdd_storage_free();

void _kbdd_storage_put(Window win, int group);

int  _kbdd_storage_get(Window win);

void _kbdd_storage_remove(Window win);

#endif
