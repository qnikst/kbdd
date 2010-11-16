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
#ifndef _KBDD_PERWINDOW_H
#define _KBDD_PERWINDOW_H_

#include <stdint.h>

typedef unsigned int  WINDOW_TYPE ;
typedef unsigned char GROUP_TYPE ;

void _kbdd_perwindow_init();

void _kbdd_perwindow_free();

void _kbdd_perwindow_put(WINDOW_TYPE win, GROUP_TYPE group);

GROUP_TYPE _kbdd_perwindow_get(WINDOW_TYPE win);
GROUP_TYPE _kbdd_perwindow_get_prev(WINDOW_TYPE win);

void _kbdd_perwindow_remove(WINDOW_TYPE win);
void _kbdd_perwindow_clean();

#endif
//vim:ts=4:expandtab
