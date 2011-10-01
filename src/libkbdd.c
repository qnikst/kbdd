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

/**
 * method prototypes
 **/
inline void _kbdd_assign_window(Display *display,Window window);
inline void _kbdd_group_names_initialize();
inline void _kbdd_inner_iter(Display * display);
inline void _kbdd_clean_groups_info();
__inline__ void _init_windows(Display * display);
static void _kbdd_update_window_layout(Window window, unsigned char grp);
static int  _kbdd_add_window(const Window window, const int accept_layout);
static Display *  _kbdd_initialize_display();
static void _kbdd_initialize_listeners();
inline void _kbdd_focus_window(Window w);
inline void _kbdd_proceed_event(XkbEvent ev);
static void _on_enterEvent(XEvent *e);
static void _on_createEvent(XEvent *e);
static void _on_destroyEvent(XEvent *e);
static void _on_propertyEvent(XEvent *e);
static void _on_focusEvent(XEvent *e);
static void _on_mappingEvent(XEvent *e);
static void _on_keypressEvent(XEvent *e);
inline void _kbdd_wait_accure();
inline void _kbdd_release();
int _xerrordummy(Display *dpy, XErrorEvent *ee);
inline void _on_xkbEvent(XkbEvent ev);
inline int kbdd_real_lock(int);
//<<prototypes

typedef struct _KbddStructure {
    int haveNames;
    int _xkbEventType;
    int prevGroup;
    int kbddLock;
    Window focus_win;
    Display * display;
    Window root_window;
} KbddStructure;

static volatile UpdateCallback    _updateCallback = NULL;
static volatile void *            _updateUserdata = NULL;

static KbddStructure       _kbdd;
static unsigned char    _group_count;
static char * * _group_names;

static void (*handler[LASTEvent]) (XEvent *) = {
    [EnterNotify]    = _on_enterEvent,
    [FocusIn]        = _on_focusEvent,
    [FocusOut]       = _on_focusEvent,
    [PropertyNotify] = _on_propertyEvent,
    [DestroyNotify]  = _on_destroyEvent,
    [CreateNotify]   = _on_createEvent,
    [MappingNotify]  = _on_mappingEvent,
//    [KeymapNotify]   = _on_mappingEvent
};

const static long w_events = EnterWindowMask
                           | FocusChangeMask
                           | PropertyChangeMask
                           | StructureNotifyMask
                           ;
const static long root_events = StructureNotifyMask
                              | SubstructureNotifyMask
                              | LeaveWindowMask
                              | EnterWindowMask
                              | FocusChangeMask
                              | KeymapStateMask;
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
    _kbdd.kbddLock = 0;
}

void 
kbdd_free( void )
{
    _kbdd_accure();
    _kbdd_perwindow_free();
    _kbdd_clean_groups_info();
    _kbdd_release();
}

Display * 
_kbdd_initialize_display( )
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
    int scr = DefaultScreen( _kbdd.display );
    _kbdd.root_window = RootWindow( _kbdd.display, scr );
    dbg("attating to window %u\n",(uint32_t)_kbdd.root_window);
    XkbSelectEventDetails( _kbdd.display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput(_kbdd.display, _kbdd.root_window, root_events);
    _init_windows(_kbdd.display);
}

/**
 * @global _kbdd
 */
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
        _kbdd_inner_iter(display);
}


/******************************************************************************
 *  Inner iterations
 *****************************************************************************/
inline void
_kbdd_proceed_event(XkbEvent ev)
{
    if (ev.type == _kbdd._xkbEventType )
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
    _kbdd_add_window(ev->window, 0);
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
    _kbdd_focus_window(ev->window);
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
    Window focused_win;
    if (ev->window == _kbdd.focus_win) 
        return;

/*  This function do not work as needed
 *  TODO: remember why did I add this
 *        or totally delete
 *
 *  if ( (ev->mode == NotifyGrab) ) {
      focused_win = ev->window;
    } else {
      _kbdd_focus_window(ev->window);    
      dbg("focus event %u", (uint32_t)ev->window);
    }*/
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
    if ( (ev->mode == NotifyGrab) || (ev->mode == NotifyUngrab)) {
        dbg("mode: %i",ev->mode);
    }
    if ( (ev->mode != NotifyNormal || ev->detail == NotifyInferior) 
            && ev->window != _kbdd.root_window ) 
        return;
    _kbdd_focus_window(ev->window);
    XSync(ev->display, 0);
    dbg("enter event");
    return;
}

static void
_on_mappingEvent(XEvent *e) 
{
    dbg("in map request");
    XMappingEvent *ev = &e->xmapping;
    if ( ev->request == MappingKeyboard ) 
    {
      _kbdd_accure();
      _kbdd_perwindow_clean();
      _kbdd_group_names_initialize();
      _kbdd_release();
    }
    XRefreshKeyboardMapping(ev);
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
        case XkbStateNotify:
            dbg( "LIBKBDD state notify event\n");
            uint32_t grp = ev.state.group;
            Window focused_win;
            int revert;
            XGetInputFocus( ev.any.display, &focused_win, &revert);      
            if (grp == ev.state.locked_group) //do not save layout with modifier
                _kbdd_update_window_layout( focused_win, grp);
            break;
        case XkbNewKeyboardNotify:
            dbg("kbdnotify %u\n",ev.any.xkb_type);
            _kbdd_accure();
            _kbdd_perwindow_clean();
            _kbdd_clean_groups_info();
            _kbdd_group_names_initialize();
            _kbdd_release();
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
    static XWindowAttributes wa;
    if ( window == 0 ) return;
    assert(display!=NULL);
    XSetErrorHandler(_xerrordummy);
    if ( ! XGetWindowAttributes(display,window,&wa) ) 
        return;
    XSelectInput( display, window, w_events);
    XSync(display, 0);
}

__inline__ void
_init_windows(Display * display)
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
          _kbdd_accure();
          _kbdd_perwindow_put(win, state.group);
          if ( state.group != _kbdd.prevGroup, _updateCallback != NULL ) 
              _updateCallback(state.group, (void *)_updateUserdata);
          _kbdd_release();
      }
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_accure();
    _kbdd_perwindow_remove(win);
    _kbdd_release();
}

int 
kbdd_set_window_layout ( Display * display, Window win ) 
{
    //if (win==_kbdd.focus_win) return 1; //HACK maybe doesn't need it
    int result = 0;
    _kbdd_accure();
    GROUP_TYPE group = _kbdd_perwindow_get( (WINDOW_TYPE)win );
    if ( _kbdd.prevGroup != group ) {
        if (kbdd_real_lock(group) && _updateCallback != NULL) {
            _updateCallback(group, (void *)_updateUserdata);
        }
    }
    _kbdd_release();
    return result;
}

static void 
_kbdd_update_window_layout ( Window window, unsigned char grp ) 
{
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_accure();
    _kbdd_perwindow_put(win, g);
    if ( _updateCallback != NULL ) 
        _updateCallback(g, (void *)_updateUserdata);
    _kbdd_release();
}

void 
kbdd_set_current_window_layout ( uint32_t layout) 
{
    Window focused_win;
    int revert;
    _kbdd_accure();
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) )
    {
        if (_kbdd.focus_win == focused_win )  //this hack will not save us in case ok KDE+Awesome
            _kbdd_perwindow_put(focused_win, layout);
        //else
        kbdd_real_lock(layout);
    }
    _kbdd_release();
    dbg("set window layout %u",layout);
}

void 
kbdd_set_previous_layout(void)
{
    Window focused_win;
    int revert;
    dbg("set previous layout"); 
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) )
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
    Window focused_win;
    int revert;
    dbg("set next layout");
    if ( XGetInputFocus( _kbdd.display, &focused_win, &revert) )
    {
        uint32_t group = _kbdd_perwindow_get(focused_win) + 1;
        if ( group >= _group_count ) 
            group = 0;
        kbdd_set_current_window_layout( group );
    }
}

uint32_t
kbdd_get_current_layout()
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
  if ( id < 0 || id>=_group_count ) return 0;
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



//vim:ts=4:expandtab
inline void
_kbdd_accure() {
    while(_kbdd.kbddLock);
    _kbdd.kbddLock = 1;
}

inline void
_kbdd_release() {
    _kbdd.kbddLock = 0;
}
