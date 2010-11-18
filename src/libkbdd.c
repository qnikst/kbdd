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
#include <X11/keysym.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

#include "libkbdd.h"
#include "common-defs.h"

#define CLEANMASK(mask) (mask & ~(LockMask))
#define LENGTH(X)       (sizeof X / sizeof X[0])

//>>prototypes
static void  kbdd_group_names_initialize(Display *display);
__inline__ void _inner_iter(Display * display);
__inline__ void _assign_window(Display *display,Window window);
__inline__ void _init_windows(Display * display);
static void _on_enterEvent(XEvent *e);
static void _on_createEvent(XEvent *e);
static void _on_destroyEvent(XEvent *e);
static void _on_propertyEvent(XEvent *e);
static void _on_focusEvent(XEvent *e);
static void _on_mappingEvent(XEvent *e);
static void _on_keypressEvent(XEvent *e);
static void _focus(Window w);
int _xerrordummy(Display *dpy, XErrorEvent *ee);
__inline__ void _on_xkbEvent(XkbEvent ev);
//<<prototypes

typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const Arg *);
    const Arg arg;
} Key;

typedef struct _KbddStructure {
    long w_events;
    long root_events;
    int haveNames;
    int _xkbEventType;
    Window focus_win;
} KbddStructure;


volatile UpdateCallback    _updateCallback = NULL;
volatile void *            _updateUserdata = NULL;
volatile static Display *  _display        = NULL;

static KbddStructure       _kbdd;
static Window root  = 0;
int    _group_count;
char * * _group_names;

static void (*handler[LASTEvent]) (XEvent *) = {
    [EnterNotify]    = _on_enterEvent,
    [FocusIn]        = _on_focusEvent,
    [FocusOut]       = _on_focusEvent,
    [PropertyNotify] = _on_propertyEvent,
    [DestroyNotify]  = _on_destroyEvent,
    [CreateNotify]   = _on_createEvent,
    [MappingNotify]  = _on_mappingEvent,
//    [KeymapNotify]   = _on_mappingEvent
//    [KeyPress]       = _on_keypressEvent
};

static void _set_current_window_layout(const Arg *arg);
#include "keys.h"

/******************************************************************************
 * Interface part
 *      specify fuctions to deal with the outter world
 *
 * Kbdd_init  - initialize structure and plugins
 * Kbdd_clean - free memory allocated by kbdd
 * Kbdd_initialize_display - initialize display and set xkbEventType
 *
 *****************************************************************************/

void 
kbdd_init(void)
{
    _kbdd.w_events = EnterWindowMask 
                   | FocusChangeMask
                   | PropertyChangeMask
                   | StructureNotifyMask
//                   | KeyPressMask
//                   | SubstructureNotifyMask
                   ;
    _kbdd.root_events = StructureNotifyMask
                      | SubstructureNotifyMask
                      | PropertyChangeMask
                      | LeaveWindowMask
                      | EnterWindowMask
                      | FocusChangeMask
                      | KeymapStateMask
//                      | KeyPressMask;
                      ;
    
    _kbdd_perwindow_init(); //initialize per-window storage
}

void 
kbdd_clean()
{
    size_t i;
    for (i = 0; i < _group_count; i++ )
    {
        if ( _group_names[i] != NULL) 
            free( _group_names[i] );
    }
    _group_names[i] = 0;

    _kbdd_perwindow_free();
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
    _kbdd._xkbEventType = xkbEventType;
    return display;
}

void 
kbdd_setupUpdateCallback(UpdateCallback callback,void * userData ) 
{
    _updateCallback = callback;
    _updateUserdata = userData;
}

void Kbdd_initialize_listeners( Display * display )
{
    dbg("Kbdd_initialize_listeners");
    assert(display!=NULL);
    dbg("keyboard initialized");
    kbdd_group_names_initialize(display);
    int scr = DefaultScreen( display );
    root = RootWindow( display, scr );
    dbg("attating to window %u\n",(uint32_t)root);
    XkbSelectEventDetails( display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput( display, root, _kbdd.root_events);
    _init_windows(display);
}

void Kbdd_setDisplay(Display * display)
{
    assert(display != NULL);
    _display = display;
}

int 
Kbdd_default_iter(void * data)
{
    assert( _display != NULL );
    while ( XPending( (Display *)_display ) ) 
        _inner_iter((Display *)_display);
    return 1;
}

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


/******************************************************************************
 *  Inner iterations
 *****************************************************************************/
__inline__
void _inner_iter(Display * display)
{
    assert(display != NULL);
    Window focused_win;
    XkbEvent ev;
    int revert;
    uint32_t grp;
    XNextEvent( display, &ev.core);
    if ( ev.type == _kbdd._xkbEventType )
    {
        dbg("xkbEvent");
        _on_xkbEvent(ev);
    }
    else 
    {
        if ( handler[ev.type] )
            handler[ev.type](&ev.core);
        else 
            dbg("doesn't handle");
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
static void 
_on_createEvent(XEvent *e )
{
    XCreateWindowEvent * ev = &e->xcreatewindow;
    dbg("creating window %u",(uint32_t)ev->window);
    Kbdd_add_window(ev->display, ev->window);
}

static void 
_on_destroyEvent(XEvent *e)
{
    dbg("destroy event");
    XSetErrorHandler(_xerrordummy);
    XDestroyWindowEvent * ev = &e->xdestroywindow;
    XSync(ev->display, 0);
    dbg("destroying window %u",(uint32_t)ev->window);
    Kbdd_remove_window(ev->window);
}

static void
_on_propertyEvent(XEvent *e) 
{
    XSetErrorHandler(_xerrordummy);
    XPropertyEvent * ev = &e->xproperty;
    if (ev->state==0) return;
    if (ev->window == _kbdd.focus_win)
        return;
    _focus(ev->window);
    dbg("property event");
    int revert;
    Window focused_win;
    XGetInputFocus(ev->display, &focused_win, &revert);
    kbdd_set_window_layout(ev->display, /*ev->window,*/ focused_win);
    XSync(ev->display, 0);
    dbg("property send_event %i\nwindow %i\nstate %i\n",ev->send_event,(uint32_t)ev->window, ev->state);
    //dbg("focused window: %u (%i)",focused_win,revert);
}

static void
_on_focusEvent(XEvent *e)
{
    XSetErrorHandler(_xerrordummy);
    XFocusChangeEvent *ev = &e->xfocus;
    if (ev->window == _kbdd.focus_win) 
        return;
    _focus(ev->window);    
    dbg("focus event %u", (uint32_t)ev->window);
    Window focused_win;
    int revert;
    XGetInputFocus(ev->display, &focused_win, &revert);
    kbdd_set_window_layout(ev->display, /*ev->window);*/ focused_win);
    XSync(ev->display, 0);
}


static void
_on_enterEvent(XEvent *e)
{
    XSetErrorHandler(_xerrordummy);
    XCrossingEvent *ev = &e->xcrossing;
    if ( (ev->mode != NotifyNormal || ev->detail == NotifyInferior) 
            && ev->window != root ) 
        return;
    _focus(ev->window);
    XSync(ev->display, 0);
    dbg("enter event");
    return;
}

static void
_on_mappingEvent(XEvent *e) 
{
    dbg("in map request");
    XMappingEvent *ev = &e->xmapping;
    _kbdd_perwindow_clean();
    kbdd_group_names_initialize(ev->display);
    XRefreshKeyboardMapping(ev);
}

static void 
_on_keypressEvent(XEvent *e)
{
    /*
    unsigned int i;

    KeySym keysym;
    XKeyEvent *ev;
    ev = &e->xkey;
    keysym = XKeycodeToKeysym(ev->display, ev->keycode, 0);
    for ( i = 0; i < LENGTH(keys); i++ )
        if (keysym == keys[i].keysym
                && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
                && keys[i].func)
            keys[i].func(&(keys[i].arg));
            */
}

static void 
_focus(Window w) 
{
    if (w) 
        _kbdd.focus_win = w;
}

__inline__ void
_on_xkbEvent(XkbEvent ev)
{
    Window focused_win;
    int revert;
    uint32_t grp;
    switch (ev.any.xkb_type)
    {
        case XkbStateNotify:
            dbg( "LIBKBDD state notify event\n");
            grp = ev.state.group;
            XGetInputFocus( ev.any.display, &focused_win, &revert);
            kbdd_update_window_layout( ev.any.display, focused_win,grp);
            break;
        case XkbNewKeyboardNotify:
            dbg("kbdnotify %u\n",ev.any.xkb_type);
            _kbdd_perwindow_clean();
            /* kbdd_group_names_clean >> */ { 
                size_t i;
                for (i = 0; i < _group_count; i++ )
                {
                    if ( _group_names[i] != NULL) 
                        free( _group_names[i] );
                }
                _group_names[i] = 0;
            } /* << kbdd_group_names_clean */
            kbdd_group_names_initialize( ev.any.display );
            break;
        default:
            break;
    }
}


int 
_xerrordummy(Display *dpy, XErrorEvent *ee) 
{
    return 0;
}
/**
 * Kbbdd inner actions
 */
__inline__
void _assign_window(Display * display, Window window)
{
    static XWindowAttributes wa;
    if ( window == 0 ) return;
    assert(display!=NULL);
    XSetErrorHandler(_xerrordummy);
    if ( ! XGetWindowAttributes(display,window,&wa) ) 
        return;
    XSelectInput( display, window, _kbdd.w_events);
    XSync(display, 0);
}

__inline__ void
_init_windows(Display * display)
{
    unsigned int i, num;
    Window d1,d2,*wins = NULL;
    XWindowAttributes wa;
    if ( XQueryTree(display, root, &d1, &d2, &wins, &num) )
    {
        for ( i=0; i < num; i++ )
        {
            if ( ! XGetWindowAttributes(display, wins[i], &wa) )
                continue;
            _assign_window( display, wins[i] );
        }
        if (wins) XFree(wins);
    }
}

int Kbdd_add_window(Display * display, Window window)
{
    _assign_window(display, window);
    XkbStateRec state;
    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success ) 
    {
        WINDOW_TYPE win = (WINDOW_TYPE)window;
        _kbdd_perwindow_put(win, state.group);
        if ( _updateCallback != NULL ) 
            _updateCallback(state.group, (void *)_updateUserdata);
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_perwindow_remove(win);
}

int 
kbdd_set_window_layout ( Display * display, Window win ) 
{
    GROUP_TYPE group = _kbdd_perwindow_get( (WINDOW_TYPE)win );
    int result = XkbLockGroup(display, XkbUseCoreKbd, group);
    if (result && _updateCallback != NULL) 
        _updateCallback(group, (void *)_updateUserdata);
    return result;
}

void 
kbdd_update_window_layout ( Display * display, Window window, unsigned char grp ) 
{
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_perwindow_put(win, g);
    if ( _updateCallback != NULL ) 
        _updateCallback(g, (void *)_updateUserdata);
}

void Kbdd_set_current_window_layout ( uint32_t layout) 
{
    dbg("set window layout %u",layout);
    int result = XkbLockGroup( (Display *)_display, XkbUseCoreKbd, layout);
}

void 
Kbdd_set_previous_layout()
{
    Window focused_win;
    int revert;
    dbg("set previous layout");
    if ( XGetInputFocus( (Display *)_display, &focused_win, &revert) )
    {
        uint32_t group = _kbdd_perwindow_get_prev(focused_win);
        dbg("group %u",group);
        Kbdd_set_current_window_layout( group );
    }
    
}

void 
Kbdd_set_next_layout()
{
    Window focused_win;
    int revert;
    dbg("set next layout");
    if ( XGetInputFocus( (Display *)_display, &focused_win, &revert) )
    {
        uint32_t group = _kbdd_perwindow_get(focused_win) + 1;
        if ( group >= _group_count ) 
            group = 0;
        Kbdd_set_current_window_layout( group );
    }
}

static 
void _set_current_window_layout(const Arg * arg) 
{
    dbg("inner set window layout");
    Kbdd_set_current_window_layout( arg->ui );
}
/**
 * Group names functions
 */

static void 
kbdd_group_names_initialize(Display * display)
{
    
    dbg("initializing keyboard");
    XkbDescRec * desc = XkbAllocKeyboard();
    assert(desc != NULL);
    XkbGetControls(display, XkbAllControlsMask, desc);
    XkbGetNames(display, XkbSymbolsNameMask | XkbGroupNamesMask, desc);
    if ( (desc->names == NULL) 
            ||  (desc->names->groups == NULL) ) {
        dbg("unable to get names");
        return;
    }
    uint32_t i;
    Atom * group_source = desc->names->groups;
    if ( _group_names )
        free( _group_names );
    _group_names = malloc( XkbNumKbdGroups * sizeof( char * ) );
    for ( i=0; i < XkbNumKbdGroups; i++ )
    {
        _group_names[i] = NULL;
        if ( group_source[i] != None )
        {
            _group_count = i+1;
            char * p = XGetAtomName(display, group_source[i]);
            _group_names[i] = strdup(p);
            dbg("%u: %s",i,_group_names[i]);
            XFree(p);
        }
    }
    XkbFreeKeyboard(desc, 0, 1);
}



int  
Kbdd_get_layout_name( uint32_t id, char ** layout)
{
  if ( id < 0 || id>=_group_count ) return 0;
  dbg( "layout: %s",_group_names[id] );
  *layout = strdup( (const char *)_group_names[id] );
  return 1;
}

//vim:ts=4:expandtab

