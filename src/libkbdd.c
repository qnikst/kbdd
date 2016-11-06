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
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>

#include "libkbdd.h"
#include "common-defs.h"

/**
 * method prototypes
 **/
static inline void _kbdd_assign_window(Display *display,Window window);
static inline void _kbdd_group_names_initialize();
static inline void _kbdd_inner_iter(Display * display);
static inline void _kbdd_clean_groups_info();
static void _kbdd_update_window_layout(Window window, unsigned char grp);
static int  _kbdd_add_window(const Window window, const int accept_layout);
static Display *  _kbdd_initialize_display();
static void _kbdd_initialize_listeners();
static inline void _kbdd_focus_window(Window w);
static void _get_active_window(Window *win);
static void _get_active_window_fallback(Display *, Window *);
static void _on_enterEvent(XEvent *e);
static void _on_createEvent(XEvent *e);
static void _on_destroyEvent(XEvent *e);
static void _on_propertyEvent_ewmh(XEvent *e);
static void _on_propertyEvent_generic(XEvent *e);
static void _on_focusEvent_ewmh(XEvent *e);
static void _on_focusEvent_generic(XEvent *e);
static int is_ehwm_supported(void);
int _xerrordummy(Display *dpy, XErrorEvent *ee);
static inline void _on_xkbEvent(XkbEvent ev);
static inline int kbdd_real_lock(int);
//<<prototypes

typedef struct _KbddStructure {
    int _xkbEventType;
    int prevGroup;
    int supportEWMH;
    Atom atom_netActiveWindow;
    Window focus_win;
    Display * display;
    Window root_window;
    UpdateCallback _updateCallback;
    void * _updateUserdata;
} KbddStructure;

static KbddStructure       _kbdd;
static unsigned char    _group_count;
static char * * _group_names;

static void (*handler_ewmh[LASTEvent]) (XEvent *) = {
    [EnterNotify]    = _on_enterEvent,
    [FocusIn]        = _on_focusEvent_ewmh,
    [FocusOut]       = _on_focusEvent_ewmh,
    [PropertyNotify] = _on_propertyEvent_ewmh, //28
    [DestroyNotify]  = _on_destroyEvent,
    [CreateNotify]   = _on_createEvent,
};

static void (*handler_generic[LASTEvent]) (XEvent *) = {
    [EnterNotify]    = _on_enterEvent,
    [FocusIn]        = _on_focusEvent_generic,
    [FocusOut]       = _on_focusEvent_generic,
    [PropertyNotify] = _on_propertyEvent_generic,
    [DestroyNotify]  = _on_destroyEvent,
    [CreateNotify]   = _on_createEvent,
};

const static long w_events = EnterWindowMask
                           | FocusChangeMask
                           | PropertyChangeMask
                           | StructureNotifyMask
                           ;
const static long root_events = StructureNotifyMask
                              | SubstructureNotifyMask
                              | PropertyChangeMask
                              | FocusChangeMask
                              | KeymapStateMask
                              | LeaveWindowMask
                              | EnterWindowMask
                              ;

/******************************************************************************
 * Interface part
 *      specify fuctions to deal with the outter world
 *
 * Kbdd_init  - initialize structure and plugins
 * Kbdd_free - free memory allocated by kbdd
 * Kbdd_initialize_display - initialize display and set xkbEventType
 *
 *****************************************************************************/

void
kbdd_init( void )
{
    _kbdd_perwindow_init(); //initialize per-window storage
    _kbdd.display = _kbdd_initialize_display();
    _kbdd_initialize_listeners();
    _kbdd_group_names_initialize();
    _kbdd.prevGroup = 0;
}

void
kbdd_free( void )
{
    _kbdd_perwindow_free();
    _kbdd_clean_groups_info();
}

Display *
_kbdd_initialize_display( )
{
    Display * display;
    int xkbEventType, xkbError, reason_rtrn;
    char * display_name = NULL;
    int mjr = XkbMajorVersion;
    int mnr = XkbMinorVersion;
    display = XkbOpenDisplay(display_name,&xkbEventType,&xkbError, &mjr,&mnr,&reason_rtrn);
    _kbdd._xkbEventType = xkbEventType;
    _kbdd.atom_netActiveWindow = XInternAtom(display, "_NET_ACTIVE_WINDOW", 0);
    return display;
}

void
kbdd_setupUpdateCallback(UpdateCallback callback,void * userData )
{
    _kbdd._updateCallback = callback;
    _kbdd._updateUserdata = userData;
}

Display * kbdd_get_display(void) {
    return _kbdd.display;
}
/**
 * @global root_events
 * @global _kbdd
 */
static void
_kbdd_initialize_listeners()
{
    dbg("Kbdd_initialize_listeners");
    assert(_kbdd.display!=NULL);
    dbg("keyboard initialized");
    XWindowAttributes wa;
    const int scr = DefaultScreen( _kbdd.display );
    (void)scr; // do not emit warning about unused variable.
    _kbdd.root_window = DefaultRootWindow( _kbdd.display);
    XGetWindowAttributes(_kbdd.display, _kbdd.root_window, &wa);
    XSelectInput(_kbdd.display, _kbdd.root_window, root_events | wa.your_event_mask);
    dbg("attaching to root window %u\n",(uint32_t)_kbdd.root_window);
    XkbSelectEventDetails( _kbdd.display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    // TODO extract methods for ewmh and generic cases
    is_ehwm_supported();
    if (_kbdd.supportEWMH) {
        fprintf(stderr, "Initializing EWHM event listeners\n");
        XkbSelectEventDetails( _kbdd.display, XkbUseCoreKbd, XkbStateNotify,
                    XkbAllStateComponentsMask, XkbGroupStateMask);
        int n;
        const Atom * atoms = XListProperties(_kbdd.display, _kbdd.root_window, &n);
        (void)atoms; // do not emit warning about unused variable.
        int i;
        for (i=0;i<n;i++) {
            dbg("%s",XGetAtomName(_kbdd.display,atoms[i]));
        }
    } else {
        fprintf(stderr, "Initializing generic event listeners\n");
        unsigned int i, num;
        Window d1,d2,*wins = NULL;
        XWindowAttributes wa;
        if ( XQueryTree(_kbdd.display, _kbdd.root_window, &d1, &d2, &wins, &num) )
        {
            for ( i=0; i < num; i++ )
            {
                XSetErrorHandler(_xerrordummy);
                if ( ! XGetWindowAttributes(_kbdd.display, wins[i], &wa) )
                    continue;
                _kbdd_assign_window( _kbdd.display, wins[i] );
            }
            if (wins) XFree(wins);
        }
    }

}

/**
 * @global _kbdd
 */
int
kbdd_default_iter(void * data)
{
    assert( _kbdd.display != NULL );
//    while ( XPending( _kbdd.display ) )
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
        _kbdd_inner_iter(display);
}


/******************************************************************************
 *  Inner iterations
 *****************************************************************************/

inline void
_kbdd_inner_iter(Display * display)
{
    assert(display != NULL);
    while (XPending(display)) {
        XkbEvent ev;
        XNextEvent( display, &ev.core);
        if (ev.type == _kbdd._xkbEventType)
            _on_xkbEvent(ev);
        else {
            //        dbg("%u %u",FocusIn, ev.type);
            if ( _kbdd.supportEWMH ) {
                if ( handler_ewmh[ev.type] )
                    handler_ewmh[ev.type](&ev.core);
            } else
                if ( handler_generic[ev.type] ) {
                    handler_generic[ev.type](&ev.core);
                } else {
                    dbg("no handler for %u", ev.type);
                }
        }
    }
}

/*****************************************************************************
 *  X11 Events actions
 *
 *  here we add an additional actions for X11 events
 ****************************************************************************/

/**
 * Create window event [http://www.xfree86.org/current/XCreateWindowEvent.3.html]
 *  - add new window to storage
 *  - TODO: use ev.parent to inherit layout
 */
static void
_on_createEvent(XEvent *e )
{
    XCreateWindowEvent * ev = &e->xcreatewindow;
    dbg("creating window %u",(uint32_t)ev->window);
    _kbdd_add_window(ev->window, 0);
}

/**
 * Destroy window [http://www.xfree86.org/current/XDestroyWindowEvent.3.html]
 *  - remove window from storage
 */
static void
_on_destroyEvent(XEvent *e)
{
    dbg("destroy event");
    XSetErrorHandler(_xerrordummy);
    XDestroyWindowEvent * ev = &e->xdestroywindow;
    dbg("destroying window %u",(uint32_t)ev->window);
    Kbdd_remove_window(ev->window);
}

/**
 * Property event handler [http://www.xfree86.org/current/XPropertyEvent.3.html]
 *  - check that this is NetActive Property and that this property is set
 *
 * @global _kbdd
 */
static void
_on_propertyEvent_ewmh(XEvent *e)
{
    dbg("property");
    XPropertyEvent * ev = &e->xproperty;
    (void)ev;
    Window w = None;
    _get_active_window(&w);
    kbdd_set_window_layout(w);
    dbg("property send_event %i\nwindow %i\nstate %i\n",ev->send_event,(uint32_t)ev->window, ev->state);
    //dbg("focused window: %u (%i)",focused_win,revert);
}

static void
_on_propertyEvent_generic(XEvent *e)
{
    XSetErrorHandler(_xerrordummy);
    XPropertyEvent * ev = &e->xproperty;
    if (ev->window!=_kbdd.root_window
            || ev->atom!=_kbdd.atom_netActiveWindow)
        return;
    _kbdd_focus_window(ev->window);
    int revert;
    kbdd_set_window_layout(ev->window);
    dbg("property send_event %i\nwindow %i\nstate %i\n",ev->send_event,(uint32_t)ev->window, ev->state);
    //dbg("focused window: %u (%i)",focused_win,revert);
}

/**
 * Focus Event Handler [http://www.xfree86.org/current/XFocusChangeEvent.3.html]
 *  - set currently selected window
 *  - set window layout
 */
static void
_on_focusEvent_ewmh(XEvent * e)
{
    (void)e;
    dbg("focus");
    XSetErrorHandler(_xerrordummy);
    Window focused_win = None;
    _get_active_window(&focused_win);
    kbdd_set_window_layout(focused_win);
}

static void
_on_focusEvent_generic(XEvent *e)
{
    dbg("focus in/out");
    XSetErrorHandler(_xerrordummy);
    XFocusChangeEvent *ev = &e->xfocus;
    Window focused_win;
    if (ev->window == _kbdd.focus_win)
        return;

    int revert;
    XGetInputFocus(ev->display, &focused_win, &revert);
    kbdd_set_window_layout(focused_win);
}


/**
 * Crossing event (enter/leave) [http://www.xfree86.org/current/XCrossingEvent.3.html]
 *
 */
static void
_on_enterEvent(XEvent *e)
{
    XSetErrorHandler(_xerrordummy);
    XCrossingEvent *ev = &e->xcrossing;
    if ( (ev->mode == NotifyGrab) || (ev->mode == NotifyUngrab)) {
        dbg("mode: %i",ev->mode);
    }
    if ( (ev->mode != NotifyNormal || ev->detail == NotifyInferior)
            && ev->window != _kbdd.root_window )
        return;
    _kbdd_focus_window(ev->window);
    dbg("enter event");
    return;
}

inline void
_kbdd_focus_window(Window w)
{
    if (w)
        _kbdd.focus_win = w;
}

inline void
_on_xkbEvent(XkbEvent ev)
{
    switch (ev.any.xkb_type)
    {
        case XkbStateNotify: do {
            dbg( "LIBKBDD state notify event\n");
            int grp = ev.state.group;
            Window focused_win = None;
            _get_active_window_fallback(ev.any.display, &focused_win);
            if (grp == ev.state.locked_group) { //do not save layout with modifier
                _kbdd.prevGroup = grp;
                _kbdd_update_window_layout( focused_win, grp);
            }
            } while ( 0 );
            break;
        case XkbNewKeyboardNotify:
            dbg("kbdnotify %u\n",ev.any.xkb_type);
            _kbdd_perwindow_clean();
            _kbdd_clean_groups_info();
            _kbdd_group_names_initialize();
            break;
        default:
            break;
    }
}


int
_xerrordummy(Display *dpy, XErrorEvent *ee)
{
#ifdef DEBUG
    char codebuff[256];
    XGetErrorText(dpy, ee->error_code, (char *)&codebuff, 256);
    printf("XError code: %c\n text: %s", ee->error_code, codebuff);
#endif
    return 0;
}
/**
 * Kbbdd inner actions
 * global w_events
 */
inline void
_kbdd_assign_window(Display * display, Window window)
{
    if (_kbdd.supportEWMH) return;
    static XWindowAttributes wa;
    if ( window == 0 ) return;
    assert(display!=NULL);
    XSetErrorHandler(_xerrordummy);
    if ( ! XGetWindowAttributes(display,window,&wa) )
        return;
    XSelectInput( display, window, w_events);
}

static int
_kbdd_add_window(const Window window, const int accept_layout)
{
    Display * display = _kbdd.display;
    assert( display != NULL );
    _kbdd_assign_window(display, window);

    if ( accept_layout ) {
      XkbStateRec state;
      if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success )
      {
          WINDOW_TYPE win = (WINDOW_TYPE)window;
          _kbdd_perwindow_put(win, state.group);
      }
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_perwindow_remove(win);
}

int
kbdd_set_window_layout ( Window win )
{
    if (win == None) return 0;
    int result = 0;
    GROUP_TYPE group = _kbdd_perwindow_get( (WINDOW_TYPE)win );
    if ( _kbdd.prevGroup != group ) {
        kbdd_real_lock(group);
    }
    return result;
}

static void
_kbdd_update_window_layout ( Window window, unsigned char grp )
{
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_perwindow_put(win, g);
    if ( _kbdd._updateCallback != NULL )
        _kbdd._updateCallback(g, (void *)_kbdd._updateUserdata);
}

void
kbdd_set_current_window_layout ( uint32_t layout)
{
    Window focused_win = None;
    _get_active_window_fallback(_kbdd.display, &focused_win);
    if ( focused_win != None )
    {
        if (_kbdd.focus_win == focused_win )  //this hack will not save us in case ok KDE+Awesome
            _kbdd_perwindow_put(focused_win, layout);
        //else
        kbdd_real_lock(layout);
    }
    dbg("set window layout %u",layout);
}

void
kbdd_set_previous_layout(void)
{
    Window focused_win = None;
    dbg("set previous layout");
    _get_active_window_fallback(_kbdd.display, &focused_win);
    if ( focused_win != None )
    {
        uint32_t group = _kbdd_perwindow_get_prev(focused_win);//not thread safe
        dbg("group %u",group);
        kbdd_set_current_window_layout( group );
    }
}


/**
 * @global _group_count
 * @global _group_names
 */
inline void
_kbdd_clean_groups_info( void)
{
    unsigned char i;
    for (i = 0; i < _group_count; i++ )
    {
        if ( _group_names[i] != NULL)
            free( _group_names[i] );
        _group_names[i] = NULL;
    }
    _group_count = 0;
}


void
kbdd_set_next_layout()
{
    Window focused_win = None;
    dbg("set next layout");
    _get_active_window_fallback(_kbdd.display, &focused_win);
    if ( focused_win != None )
    {
        uint32_t group = _kbdd_perwindow_get(focused_win) + 1;
        if ( group >= _group_count )
            group = 0;
        kbdd_set_current_window_layout( group );
    }
}

uint32_t
kbdd_get_current_layout(void)
{
    uint32_t result = 0;
    XkbStateRec state;
    if ( XkbGetState(_kbdd.display, XkbUseCoreKbd, &state) == Success )
    {
        result =  state.locked_group;
    }
    return result;
}

/**
 * Group names functions
 */

inline void
_kbdd_group_names_initialize()
{
    Display * display = _kbdd.display;
    dbg("initializing keyboard");
    assert( display != NULL );
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
kbdd_get_layout_name( uint32_t id, char ** layout)
{
  if ( id>=_group_count ) return 0;
  dbg( "layout: %s",_group_names[id] );
  *layout = strdup( (const char *)_group_names[id] );
  return 1;
}

int
kbdd_real_lock(int group) {
    int result = 0;
    if ( _kbdd.prevGroup != group) {
        result = XkbLockGroup(_kbdd.display, XkbUseCoreKbd, group);
        if (result) {
            _kbdd.prevGroup = group;
        }
    }
    return result;
}

void _get_active_window(Window *win) {
    Atom actualType;
    int  actualFormat;
    long unsigned int nItems, bytesAfter;
    unsigned char *propReturn = 0;
    int ret;
    ret =  XGetWindowProperty(_kbdd.display,_kbdd.root_window, _kbdd.atom_netActiveWindow, 0, sizeof(Window), 0, XA_WINDOW,
                        &actualType, &actualFormat, &nItems, &bytesAfter, &propReturn);
    if ( ret == Success && propReturn) {
        memcpy(win, propReturn, sizeof(Window));
        XFree(propReturn);
    } else {
        dbg("XGetWindowProperty error %u %u %u %u",BadAtom, BadValue, BadWindow, ret);
    }
    dbg("active window %u", *win);
}

static void _get_active_window_fallback(Display * d, Window *win) {
    if (_kbdd.supportEWMH) {
        _get_active_window(win);
    } else {
        int revert;
        XGetInputFocus( d, win, &revert);
    }
}

int is_ehwm_supported(void) {
    Atom actualType;
    int  actualFormat;
    unsigned char *propReturn = 0;
    long unsigned int nItems, bytesAfter;
    int ret = XGetWindowProperty(_kbdd.display,_kbdd.root_window, _kbdd.atom_netActiveWindow, 0, sizeof(Window), 0, XA_WINDOW,
                        &actualType, &actualFormat, &nItems, &bytesAfter, &propReturn);
    if ( ret == Success && propReturn) {
        XFree(propReturn);
        _kbdd.supportEWMH = True;
        fprintf(stderr,"EWMH is supported\n");
    } else {
        _kbdd.supportEWMH = False;
        fprintf(stderr,"EWMH is not supported: switching to generic\n");
    }
    return _kbdd.supportEWMH;
}
//vim:ts=4:expandtab
