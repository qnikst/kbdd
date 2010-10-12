/********************************************************************* 
 * Kbdd - simple per-window-keyboard layout library and deamon 
 * Copyright (C) 2010  Alexander V Vershilov and collaborators
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
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
//vim:ts=4:expandtab
