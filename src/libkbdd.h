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
void kbdd_init();
/**
 * Deinitialize subsystem
 */
void Kbdd_clean();

/**
 * Update window layout
 */
int  Kbdd_set_window_layout(Display *,Window); 

/**
 * Update group info to the current one
 */
void Kbdd_update_window_layout(Display *, Window, unsigned char group);

/**
 *  Remove window info from storage
 *  Display * pointer to window
 */
void Kbdd_remove_window(Window);


void Kbdd_setupUpdateCallback(UpdateCallback, void *);

int  Kbdd_get_layout_name( uint32_t id, char ** layout);
void Kbdd_set_current_window_layout ( uint32_t ); 
void Kbdd_set_previous_layout();
void Kbdd_set_next_layout();
/**
 * default main loop that need to make xkbd working
 */
void * Kbdd_default_loop();

Display * Kbdd_initialize_display( );

void Kbdd_initialize_listeners( Display * );

void Kbdd_setDisplay(Display *);
int Kbdd_default_iter(void *);

#endif
//vim:ts=4:expandtab
