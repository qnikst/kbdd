#ifndef _STORAGE_H_
#define _STORAGE_H_

#ifdef STORAGE_GHASH
#include <glib.h>
#endif

#include <stdint.h>
#include <X11/Xlib.h>

#define WINDOW_TYPE    uint32_t
#define GROUP_TYPE     unsigned char

void _kbdd_storage_init();

void _kbdd_storage_free();

void _kbdd_storage_put(Window win, GROUP_TYPE group);

GROUP_TYPE _kbdd_storage_get(Window win);

void _kbdd_storage_remove(Window win);

#endif
