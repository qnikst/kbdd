/*
 * vim:ts=4:expandtab
 *
 * © 2010 Alexander Vershilov and contributors
 *
 * See file LICENSE for license information.
 */
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "libkbdd.h"

volatile int _xkbEventType;
volatile UpdateCallback    _updateCallback = NULL;
volatile void *            _updateUserdata = NULL;

void Kbdd_init()
{
    _kbdd_storage_init(); 
}

void Kbdd_clean()
{
    _kbdd_storage_free();
}

int Kbdd_add_window(Display * display, Window window)
{
    XkbStateRec state;
    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success ) 
    {
        WINDOW_TYPE win = (WINDOW_TYPE)window;
        _kbdd_storage_put(win, state.group);
        if ( _updateCallback != NULL ) 
            _updateCallback(state.group, (void *)_updateUserdata);
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_storage_remove(win);
}

int Kbdd_set_window_layout ( Display * display, Window win ) 
{
    GROUP_TYPE group = _kbdd_storage_get( (WINDOW_TYPE)win );
    int result = XkbLockGroup(display, XkbUseCoreKbd, group);
    if (result && _updateCallback != NULL) 
        _updateCallback(group, (void *)_updateUserdata);
    return result;
}

void Kbdd_update_window_layout ( Display * display, Window window, unsigned char grp ) 
{
//    XkbStateRec state;
//    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success )
//    {
//      _kbdd_storage_put(win, state.group);
//    }
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_storage_put(win, g);
}

Display * Kbdd_initialize_display( )
{
    Display * display;
    int xkbEventType,xkbError,  reason_rtrn;
    char * display_name = NULL;
    int mjr = XkbMajorVersion;
    int mnr = XkbMinorVersion;
    display = XkbOpenDisplay(display_name,&xkbEventType,&xkbError, &mjr,&mnr,&reason_rtrn);
    _xkbEventType = xkbEventType;
    return display;
}

void Kbdd_setupUpdateCallback(UpdateCallback callback,void * userData ) 
{
    _updateCallback = callback;
    _updateUserdata = userData;
}

void Kbdd_initialize_listeners( Display * display )
{
    assert(display!=NULL);
    int scr = DefaultScreen( display );
    Window root_win = RootWindow( display, scr );
    XkbSelectEventDetails( display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput( display, root_win, StructureNotifyMask | SubstructureNotifyMask
            | EnterWindowMask | FocusChangeMask | LeaveWindowMask );
}

void Kbdd_default_loop(Display * display) 
{
    assert(display!=NULL);
    Window focused_win;
    int revert;
    unsigned char grp;
    XkbEvent ev; 
    while ( 1 ) 
    {
        XNextEvent(display, &ev.core);
        if ( ev.type == _xkbEventType)
        {
            switch (ev.any.xkb_type)
            {
                case XkbStateNotify:
                    grp = ev.state.locked_group;
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_update_window_layout( display, focused_win,grp);
                    break;
                default:
                    break;
            }
        } 
        else 
        {
            switch (ev.type)
            {
                case DestroyNotify:
                    Kbdd_remove_window(ev.core.xdestroywindow.window);
                    break;
                case CreateNotify:
                    Kbdd_add_window(display, ev.core.xcreatewindow.window);
                    break;
                case FocusIn:
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_set_window_layout(display, focused_win);
                    break;
                case FocusOut:
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_set_window_layout(display, focused_win);
                default:
                    XGetInputFocus(display, &focused_win, &revert);
                    break;
            }
        }

    }
}

