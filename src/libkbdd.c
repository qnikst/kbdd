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

#include "perwindow.h"
#include "libkbdd.h"
#include "common-defs.h"

#define CLEANMASK(mask) (mask & ~(LockMask))
#define LENGTH(X)       (sizeof X / sizeof X[0])


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
    Display * display;
    Window root_window;
} KbddStructure;

/**
 * method prototypes
 **/
static void _kbdd_assign_window(Display *display,Window window);
static void _kbdd_group_names_initialize( );
static int  _kbdd_add_window(Display * display, Window window);
static void _kbdd_remove_window(Window window);
static void _kbdd_proceed_event();
static void _kbdd_update_window_layout(Window window, unsigned char grp);
static Display * _kbdd_initialize_display();
inline void _kbdd_init_windows(Display * display);
inline void _kbdd_focus_window(Window w);
inline void _kbdd_initialize_listeners();
inline void _kbdd_group_names_initialize();
inline void _kbdd_inner_iter(Display * display);
inline void _kbdd_clean_groups_info();
//xorg events
static void _on_enterEvent(XEvent *e);
static void _on_createEvent(XEvent *e);
static void _on_destroyEvent(XEvent *e);
static void _on_propertyEvent(XEvent *e);
static void _on_focusEvent(XEvent *e);
static void _on_mappingEvent(XEvent *e);
static void _on_keypressEvent(XEvent *e);
__inline__ void _on_xkbEvent(XkbEvent ev);
//xorg errors:
int _xerrordummy(Display *dpy, XErrorEvent *ee);

static volatile UpdateCallback    _updateCallback = NULL;
static volatile void *            _updateUserdata = NULL;
//static volatile Display *  _display        = NULL;

static KbddStructure       _kbdd;
//static Window root  = 0;
static int    _group_count;
static char * * _group_names;

const static long w_events = EnterWindowMask
                           | FocusChangeMask
                           | PropertyChangeMask
                           | StructureNotifyMask
                           ;

const static long root_events = StructureNotifyMask
                              | SubstructureNotifyMask
                              | PropertyChangeMask
                              | LeaveWindowMask
                              | EnterWindowMask
                              | FocusChangeMask
                              | KeymapStateMask
                              ;
/**
 * Event handlers (dwm style)
 */
static void (*handler[LASTEvent]) (XEvent *) = {
    [EnterNotify]    = _on_enterEvent,
    [FocusIn]        = _on_focusEvent,
    [FocusOut]       = _on_focusEvent,
    [PropertyNotify] = _on_propertyEvent,
    [DestroyNotify]  = _on_destroyEvent,
    [CreateNotify]   = _on_createEvent,
    [MappingNotify]  = _on_mappingEvent,
//    [KeyPress]       = _on_keypressEvent
};

static void _set_current_window_layout(const Arg *arg);

//#include "keys.h"

/******************************************************************************
 * Interface part
 *      specify fuctions to deal with the outter world
 *
 * Kbdd_init  - initialize structure and plugins
 * Kbdd_clean - free memory allocated by kbdd
 * Kbdd_initialize_display - initialize display and set xkbEventType
 *
 *****************************************************************************/


/**
 * Initialize kbdd structure
 */
void 
kbdd_init( void )
{
    _kbdd_perwindow_init(); //initialize per-window storage
    _kbdd.display = _kbdd_initialize_display();//initialize Xorg display
    _kbdd_initialize_listeners(); // initialize listeners for the root window
    _kbdd_init_windows(_kbdd.display); //add listeners for all presented windows
    _kbdd_group_names_initialize();//initialize all layout groups exists
}

void 
kbdd_free( void ) 
{
    _kbdd_perwindow_free();//clean per-window storage
    _kbdd_clean_groups_info();//clean groups info
}

Display *
kbdd_get_display( void ) 
{
    return _kbdd.display;
}

void 
kbdd_setupUpdateCallback(UpdateCallback callback,void * userData ) 
{
    _updateCallback = callback;
    _updateUserdata = userData;
}

int 
kbdd_default_iter(void * data)
{
    assert( _kbdd.display != NULL );
    while ( XPending( _kbdd.display ) ) 
        _kbdd_inner_iter(_kbdd.display);
    return 1;
}

void * 
kbdd_default_loop(Display * display) 
{
    dbg( "default loop started\n");
    if (display == NULL)
        display = _kbdd.display;
    assert(display!=NULL);

    while ( 1 ) 
        _kbdd_inner_iter((Display *)display);
}

int 
kbdd_set_window_layout ( Display * display, Window win ) 
{
    if ( win == _kbdd.focus_win ) return 1;
    GROUP_TYPE group = _kbdd_perwindow_get( (WINDOW_TYPE)win );
    dbg(">>>>>>>>>>>>>>>> SET LAYOUT (%u->%u) <<<<<<<<<<<<<<",(uint32_t)win,group);
    int result = XkbLockGroup(display, XkbUseCoreKbd, group);
    return result;
}

void 
_kbdd_update_window_layout ( Window window, unsigned char grp ) 
{
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_perwindow_put(win, g);
    if ( _updateCallback != NULL ) 
        _updateCallback(g, (void *)_updateUserdata);
}

void 
kbdd_set_current_window_layout ( uint32_t layout) 
{
    dbg(">>>>>>>>> set window layout %u <<<<<<<<<<<",layout);
    if ( layout < 0 || layout > _group_count ) 
    {
        //TODO throw error
        return;
    }
    Window focused_win;
    int revert;
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) ) //TODO remove if available
    {
        _kbdd_perwindow_put(focused_win, layout);
    }
    //int result = XkbLockGroup( _kbdd.display, XkbUseCoreKbd, layout);
}

void 
kbdd_set_previous_layout( void )
{
    dbg("set previous layout");
    Window focused_win;
    int revert;
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) ) //TODO remove if available
    {
        unsigned char group = _kbdd_perwindow_get_prev(focused_win);
        dbg("group %u",group);
        kbdd_set_current_window_layout( group );
    }
}

void 
kbdd_set_next_layout( void )
{
    Window focused_win;
    int revert;
    dbg("set next layout");
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) )//TODO remove if available
    {
        uint32_t group = _kbdd_perwindow_get(focused_win) + 1;
        if ( group >= _group_count ) 
            group = 0;
        kbdd_set_current_window_layout( group );
    }
}

int  
kbdd_get_layout_name( uint32_t id, char ** layout)
{
  if ( id < 0 || id>=_group_count ) return 0;
  dbg( "layout: %s",_group_names[id] );
  *layout = strdup( (const char *)_group_names[id] );
  return 1;
}

/**
 *  X11 Events actions
 *  
 */
static void 
_on_createEvent(XEvent *e )
{
    XCreateWindowEvent * ev = &e->xcreatewindow;
    dbg("creating window %u",(uint32_t)ev->window);
    _kbdd_add_window(ev->display, ev->window);
}

static void 
_on_destroyEvent(XEvent *e)
{
    dbg("destroy event");
    XSetErrorHandler(_xerrordummy);
    XDestroyWindowEvent * ev = &e->xdestroywindow;
    XSync(ev->display, 0);
    dbg("destroying window %u",(uint32_t)ev->window);
    _kbdd_remove_window(ev->window);
}

static void
_on_propertyEvent(XEvent *e) 
{
    XSetErrorHandler(_xerrordummy);
    XPropertyEvent * ev = &e->xproperty;
    if (ev->state==0) return;
    if (ev->window == _kbdd.focus_win)
        return;
    _kbdd_focus_window(ev->window); //TODO change method name
    dbg("property event");
    int revert;
    Window focused_win;
    XGetInputFocus(ev->display, &focused_win, &revert);//TODO remove if avaliable
    kbdd_set_window_layout(ev->display, focused_win);
    XSync(ev->display, 0);
    dbg("property send_event %i\nwindow %i\nstate %i\n",ev->send_event,(uint32_t)ev->window, ev->state);
}

static void
_on_focusEvent(XEvent *e)
{
    XSetErrorHandler(_xerrordummy);
    XFocusChangeEvent *ev = &e->xfocus;
    if (ev->window == _kbdd.focus_win) 
        return;
    _kbdd_focus_window(ev->window);    
    dbg("focus event %u", (uint32_t)ev->window);
    {
      Window focused_win;
      int revert;
      XGetInputFocus(ev->display, &focused_win, &revert);
      (void) kbdd_set_window_layout(ev->display, /*ev->window);*/ focused_win);
      _kbdd_focus_window(focused_win);//real focus
    }
    XSync(ev->display, 0);
}


static void
_on_enterEvent(XEvent *e)
{
    /*
    XSetErrorHandler(_xerrordummy);
    XCrossingEvent *ev = &e->xcrossing;
    if ( (ev->mode != NotifyNormal || ev->detail == NotifyInferior) 
            && ev->window != _kbdd.root_window ) 
        return;
    _kbdd_focus_window(ev->window);
    XSync(ev->display, 0);
    dbg("enter event");*/
    return;
}

static void
_on_mappingEvent(XEvent *e) 
{
    dbg("in map request");
    XMappingEvent *ev = &e->xmapping;
    _kbdd_perwindow_clean();
    _kbdd_group_names_initialize();
    XRefreshKeyboardMapping(ev);
}

inline void
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
            //XGetInputFocus( ev.any.display, &focused_win, &revert);
            _kbdd_update_window_layout( _kbdd.focus_win,grp);
            break;
        case XkbNewKeyboardNotify:
            dbg("kbdnotify %u\n",ev.any.xkb_type);
            _kbdd_perwindow_clean();
            _kbdd_clean_groups_info();
            _kbdd_group_names_initialize( );
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

/**************************************************************************
 * Inner methods
 *************************************************************************/

inline void 
_kbdd_focus_window(Window w) 
{
    if (w) 
        _kbdd.focus_win = w;
}

inline void 
_kbdd_assign_window(Display * display, Window window)
{
    static XWindowAttributes wa;
    if ( window == 0 ) return;
    assert(display!=NULL);
    XSetErrorHandler(_xerrordummy);
    if ( ! XGetWindowAttributes(display,window,&wa) ) 
        return;
    XSelectInput( display, window, w_events);
    XSync(display, 0);
}

inline void
_kbdd_init_windows(Display * display)
{
    unsigned int i, num;
    Window d1,d2,*wins = NULL;
    XWindowAttributes wa;
    if ( XQueryTree(display, _kbdd.root_window, &d1, &d2, &wins, &num) )
    {
        for ( i=0; i < num; i++ )
        {
            XSetErrorHandler(_xerrordummy);
            if ( ! XGetWindowAttributes(display, wins[i], &wa) )
                continue;
            XSync(display, 0);
            _kbdd_assign_window( display, wins[i] );
        }
        if (wins) XFree(wins);
    }
}

int 
_kbdd_add_window(Display * display, Window window)
{
    _kbdd_assign_window(display, window);
    XkbStateRec state;
    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success ) 
    {
        WINDOW_TYPE win = (WINDOW_TYPE)window;
        _kbdd_perwindow_put(win, state.group);
//        if ( _updateCallback != NULL ) 
//            _updateCallback(state.group, (void *)_updateUserdata);
    }
    return 0;
}

static void 
_kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_perwindow_remove(win);
}



static 
void _set_current_window_layout(const Arg * arg) 
{
    dbg("inner set window layout");
    kbdd_set_current_window_layout( arg->ui );
}

inline void 
_kbdd_clean_groups_info( void )
{
    unsigned char i;
    for (i = 0; i < _group_count; i++ )
    {
        if ( _group_names[i] != NULL) 
            free( _group_names[i] );
    }
    _group_names[i] = NULL;
}

Display * 
_kbdd_initialize_display( void )
{
    Display * display;
    int xkbEventType,xkbError, reason_rtrn;
    char * display_name = NULL;
    int mjr = XkbMajorVersion;
    int mnr = XkbMinorVersion;
    display = XkbOpenDisplay(display_name,&xkbEventType,&xkbError, &mjr,&mnr,&reason_rtrn);
    //TODO return error if possible
    _kbdd._xkbEventType = xkbEventType;
    return display;
}

inline void 
_kbdd_initialize_listeners( void )
{
    assert( _kbdd.display != NULL );
    dbg("Kbdd_initialize_listeners");
    dbg("keyboard initialized");
    int scr = DefaultScreen( _kbdd.display );
    _kbdd.root_window = RootWindow( _kbdd.display, scr );
    dbg("attating to window %u\n",(uint32_t)_kbdd.root_window);
    XkbSelectEventDetails( _kbdd.display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput( _kbdd.display, _kbdd.root_window, root_events);
}

inline void 
_kbdd_group_names_initialize( void )
{
    dbg("initializing keyboard");
    assert(_kbdd.display != NULL );
    Display * display = _kbdd.display; 
    XkbDescRec * desc = XkbAllocKeyboard();
    assert(desc != NULL);
    XkbGetControls(display, XkbAllControlsMask, desc);
    XkbGetNames(display, XkbSymbolsNameMask | XkbGroupNamesMask, desc);
    if ( (desc->names == NULL) 
            ||  (desc->names->groups == NULL) ) {
        dbg("unable to get names");
        return;
    }
    unsigned char i;
    Atom * group_source = desc->names->groups;
    if ( _group_names )
        free( _group_names ); //TODO fix memory leak
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

inline void 
_kbdd_proceed_event(XkbEvent ev)
{
    if ( ev.type  == _kbdd._xkbEventType )
        _on_xkbEvent(ev);
    else
        if ( handler[ev.type] )
            handler[ev.type](&ev.core);
}


inline void 
_kbdd_inner_iter(Display * display)
{
    assert(display != NULL);
    Window focused_win;
    XkbEvent ev;
    XNextEvent( display, &ev.core);
    _kbdd_proceed_event(ev);
}

//vim:ts=4:expandtab

