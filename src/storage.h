/*
 * vim:ts=4:expandtab
 *
 * © 2010 Alexander Vershilov and contributors
 *
 * See file LICENSE for license information.
 */
#ifndef _STORAGE_H_
#define _STORAGE_H_

#ifdef STORAGE_GHASH
#include <glib.h>
#endif

#include <stdint.h>

typedef unsigned int   WINDOW_TYPE ;
typedef unsigned int GROUP_TYPE ;

void _kbdd_storage_init();

void _kbdd_storage_free();

void _kbdd_storage_put(WINDOW_TYPE win, GROUP_TYPE group);

GROUP_TYPE _kbdd_storage_get(WINDOW_TYPE win);

void _kbdd_storage_remove(WINDOW_TYPE win);

#endif
