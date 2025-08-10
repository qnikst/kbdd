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

/**
 * Initialize subsystem
 */
void kbdd_init(void);
/**
 * Deinitialize subsystem
 */
void kbdd_free(void);

/**
 * Update window layout
 */
int  kbdd_set_window_layout(Window); 

/**
 *  Remove window info from storage
 *  Display * pointer to window
 */
void Kbdd_remove_window(Window);


void kbdd_setupUpdateCallback(UpdateCallback, void *);

int  kbdd_get_layout_name( uint32_t id, char ** layout);
void kbdd_set_current_window_layout ( uint32_t ); 
void kbdd_set_previous_layout(void);
void kbdd_set_next_layout(void);
uint32_t kbdd_get_current_layout(void);
/**
 * default main loop that need to make xkbd working
 */
void * kbdd_default_loop(Display * display);

int kbdd_default_iter(void *);

Display * kbdd_get_display(void);

#endif
//vim:ts=4:expandtab
