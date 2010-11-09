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
#ifndef _XKBDLIB_H_
#define _XKBDLIB_H_


#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "perwindow.h"


typedef void (*UpdateCallback)(unsigned int, void *);

void kbdd_init();
void kbdd_free();
Display * kbdd_get_display();
void kbdd_setupUpdateCallback(UpdateCallback, void *);
/**
 * Actions
 */
void kbdd_set_group ( uint32_t ); 
void kbdd_set_previous_group();
void kbdd_set_next_group();

/**
 * Get info
 */
int  kbdd_get_group_name( uint32_t id, char ** layout);
//int  kbdd_set_window_layout(Display *,Window); 
//void kbdd_update_window_layout(Display *, Window, unsigned char group);

/**
 * default main loop that need to make xkbd working
 */
void * kbdd_default_loop(Display *);
void kbdd_initialize_listeners( Display * );
int kbdd_default_iter(void *);

#endif
//vim:ts=4:expandtab
