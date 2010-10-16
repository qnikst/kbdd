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
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "libkbdd.h"
#include "common-defs.h"

//>>prototypes
__inline__ void _inner_iter(Display * display);
__inline__ void _on_createEvent(XCreateWindowEvent ev);
__inline__ void _on_destroyEvent(XDestroyWindowEvent ev);
__inline__ void _on_propertyEvent(XPropertyEvent ev);
__inline__ void _on_focusEvent(XFocusChangeEvent ev);
//<<prototypes

typedef struct _KbddStructure {
    long w_events;
    long root_events;
} KbddStructure;

volatile int _xkbEventType;
volatile UpdateCallback    _updateCallback = NULL;
volatile void *            _updateUserdata = NULL;
volatile static Display *  _display        = NULL;

static KbddStructure       _kbdd;

void Kbdd_init()
{
    _kbdd_storage_init(); 
    _kbdd.w_events = EnterWindowMask 
                   | FocusChangeMask
                   | PropertyChangeMask
                   | StructureNotifyMask;
    _kbdd.root_events = StructureNotifyMask
                      | SubstructureNotifyMask
                      | PropertyChangeMask
                      | EnterWindowMask
                      | FocusChangeMask;
}

void Kbdd_clean()
{
    _kbdd_storage_free();
}

Display * 
Kbdd_initialize_display( )
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

void 
Kbdd_setupUpdateCallback(UpdateCallback callback,void * userData ) 
{
    _updateCallback = callback;
    _updateUserdata = userData;
}

void Kbdd_initialize_listeners( Display * display )
{
    dbg("Kbdd_initialize_listeners\n");
    assert(display!=NULL);
    int scr = DefaultScreen( display );
    Window root_win = RootWindow( display, scr );
    dbg("attating to window %u\n",root_win);
    XkbSelectEventDetails( display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput( display, root_win, _kbdd.root_events);
}

void Kbdd_setDisplay(Display * display)
{
    assert(display != NULL);
    _display = display;
}


/**
 * Default iteration of catching Xorg events
 */
int 
Kbdd_default_iter(void * data)
{
    assert( _display != NULL );
    while ( XPending( (Display *)_display ) ) 
        _inner_iter((Display *)_display);
    return 1;
}

/**
 * Default loop for catching Xorg events
 */
void * 
Kbdd_default_loop(Display * display) 
{
    dbg( "default loop started\n");
    if (display == NULL)
        display = (Display *)_display;
    assert(display!=NULL);

    while ( 1 ) 
        _inner_iter((Display *)display);
}

__inline__
void _inner_iter(Display * display)
{
    assert(display != NULL);
    Window focused_win;
    XkbEvent ev;
    int revert;
    uint32_t grp;
    XNextEvent( display, &ev.core);
    if ( ev.type == _xkbEventType )
    {
        switch (ev.any.xkb_type)
        {
            case XkbStateNotify:
                dbg( "LIBKBDD state notify event\n");
                grp = ev.state.locked_group;
                XGetInputFocus( display, &focused_win, &revert);
                Kbdd_update_window_layout( display, focused_win,grp);
                break;
            default:
                dbg("kbdnotify %u\n",ev.any.xkb_type);
                break;
        }
    }
    else 
    {
        switch (ev.type)
        {
            case DestroyNotify:
                _on_destroyEvent(ev.core.xdestroywindow);
                break;
            case CreateNotify:
                _on_createEvent(ev.core.xcreatewindow);
                break;
            case FocusIn:
            case FocusOut:
                _on_focusEvent(ev.core.xfocus);
                break;
            case EnterNotify:
                dbg( "LIBKBDD Enter event\n");
                break;
            case PropertyNotify:
                _on_propertyEvent(ev.core.xproperty);
                dbg("Property change notify\n");
                break;
            default:
                dbg( "LIBKBDD default %u\n", ev.type);
                XGetInputFocus(display, &focused_win, &revert);
                break;
        }
    }
}

/**
 *  X11 Events actions
 *  
 *  here we add an additional actions for X11 events
 *
 *
 *
 */
__inline__ void 
_on_createEvent(XCreateWindowEvent ev )
{
    dbg("start\n");
    assert(ev.display != NULL);
    XSelectInput( ev.display, ev.window, _kbdd.w_events);
    //we can set parent window layout or default layout
    //if we use ev.parent 
    Kbdd_add_window(ev.display, ev.window);
}

__inline__ void 
_on_destroyEvent(XDestroyWindowEvent ev)
{
    dbg( "LIBKBDD destroy notify event\n");
    Kbdd_remove_window(ev.window);
}

__inline__ void
_on_propertyEvent(XPropertyEvent ev) 
{
    dbg("start");
    dbg("window id %i\n",ev.window);
    Kbdd_set_window_layout(ev.display, ev.window);
}

__inline__ void
_on_focusEvent(XFocusChangeEvent ev)
{
    Window focused_win;
    int revert;
    dbg( "LIBKBDD Focus out");
    dbg("window id %i", ev.window);
    XGetInputFocus(ev.display, &focused_win, &revert);
    dbg("window real %i", focused_win);
    dbg("===========================");
    Kbdd_set_window_layout(ev.display, focused_win);

}
/**
 * Kbbdd inner actions
 */
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
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_storage_put(win, g);
    if ( _updateCallback != NULL ) 
        _updateCallback(g, (void *)_updateUserdata);
}
//vim:ts=4:expandtab

