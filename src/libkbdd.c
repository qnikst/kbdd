#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "libkbdd.h"
#include "storage.h"

volatile int _xkbEventType;

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
        _kbdd_storage_put(window, state.group);
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    _kbdd_storage_remove(window);
}

int Kbdd_set_window_layout ( Display * display, Window win ) 
{
    int group = _kbdd_storage_get( win );
    return XkbLockGroup(display, XkbUseCoreKbd, group);
}

void Kbdd_update_window_layout ( Display * display, Window win,unsigned int grp ) 
{
//    XkbStateRec state;
//    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success )
//    {
//      _kbdd_storage_put(win, state.group);
//    }
    _kbdd_storage_put(win, grp);
}

Display * Kbdd_initialize_display( )
{
    Display * display;
    XkbEvent ev;
    int xkbEventType,xkbError,  reason_rtrn;
    char * display_name = NULL;
    int mjr = XkbMajorVersion;
    int mnr = XkbMinorVersion;
    display = XkbOpenDisplay(display_name,&xkbEventType,&xkbError, &mjr,&mnr,&reason_rtrn);
    if (display == NULL) 
    {
    }
    _xkbEventType = xkbEventType;
    return display;
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
    int revert,grp;
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

