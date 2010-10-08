/*
 * vim:ts=4:expandtab
 *
 * © 2010 Alexander Vershilov and contributors
 *
 * See file LICENSE for license information.
 */
#ifndef _XKBDLIB_H_
#define _XKBDLIB_H_

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "storage.h"


typedef void (*UpdateCallback)(unsigned int, void *);

/**
 * Initialize subsystem
 */
void Kbdd_init();
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

/**
 * default main loop that need to make xkbd working
 */
void Kbdd_default_loop();

Display * Kbdd_initialize_display( );

void Kbdd_initialize_listeners( Display * );
#endif
