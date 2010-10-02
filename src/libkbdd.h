#ifndef _XKBDLIB_H_
#define _XKBDLIB_H_

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define GROUP_TYPE unsigned char
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
void Kbdd_update_window_layout(Display *, Window);

/**
 *  Remove window info from storage
 *  Display * pointer to window
 */
void Kbdd_remove_window(Window);


/**
 * default main loop that need to make xkbd working
 */
void Kbdd_default_loop();

Display * Kbdd_initialize_display( );

void Kbdd_initialize_listeners( Display * );
#endif
