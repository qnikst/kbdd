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
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "common-defs.h"
#include <glib.h>

#include "perwindow.h"

GHashTable *gStorage = NULL;

#ifdef DEBUG
void debug();
#else
#define debug(dummy...) 
#endif

GROUP_TYPE _kbdd_perwindow_init_default(WINDOW_TYPE win, int isCurrent);

void 
_kbdd_perwindow_init() {
    if ( gStorage!=NULL ) return; 
    gStorage = g_hash_table_new(g_direct_hash, NULL);
}

void 
_kbdd_perwindow_free() {
    if ( gStorage != NULL ) {
        g_hash_table_destroy(gStorage);
    }
}

void 
_kbdd_perwindow_put(WINDOW_TYPE win, GROUP_TYPE group)
{
    assert( gStorage != NULL );
    gpointer key = NULL;
    gpointer value = NULL;
    const gpointer pWindow = GUINT_TO_POINTER(win);
    if ( g_hash_table_lookup_extended(gStorage, pWindow, &key, &value) )
    {
        GROUP_TYPE oldv = (GROUP_TYPE)( GPOINTER_TO_UINT(value) & 0xFF);
        if ( oldv != group ) {
            dbg("switching from %u |->new %u", oldv, group);
            value = GUINT_TO_POINTER( ( oldv << 8 ) | ( group & 0xFF ) );
        }
    }
    else 
    {
        value = GUINT_TO_POINTER(group);
    }
    dbg("inserting %p -> %u (%u)", pWindow, GPOINTER_TO_UINT(value),group);
    g_hash_table_replace(gStorage, pWindow, value);
    debug();
}

GROUP_TYPE
_kbdd_perwindow_get_prev(WINDOW_TYPE win)
{
    assert( gStorage != NULL );
    dbg("getprev %u", (uint32_t)win);
    GROUP_TYPE group = 0;
    assert(gStorage != NULL);
    gpointer key = NULL;
    gpointer value = NULL;
    gpointer pWindow = GUINT_TO_POINTER(win);
    if ( g_hash_table_lookup_extended(gStorage, pWindow, &key, &value) )
    {
        group = (GROUP_TYPE)((GPOINTER_TO_UINT(value)>>8) & 0xFF );
    }
    else
        group = _kbdd_perwindow_init_default(win, 0);
    return group;
}

GROUP_TYPE 
_kbdd_perwindow_get(WINDOW_TYPE win)
{
    assert( gStorage != NULL );
    GROUP_TYPE group;
    gpointer key = NULL;
    gpointer value = NULL;
    gpointer pWindow = GUINT_TO_POINTER(win);
    if ( g_hash_table_lookup_extended(gStorage, pWindow, &key, &value) )
    {
        group = (GROUP_TYPE)(GPOINTER_TO_UINT(value) & 0xFF);
    }
    else 
    {
        group = _kbdd_perwindow_init_default(win, 1);
    }
    return group;
}

GROUP_TYPE
_kbdd_perwindow_init_default(WINDOW_TYPE win, int isCurrent)
{
    assert( gStorage != NULL );
    const gpointer value = GUINT_TO_POINTER( (DEFAULT_SND_LAYOUT<<8) | (DEFAULT_FST_LAYOUT & 0xFF));
    const gpointer pWindow = GUINT_TO_POINTER(win);
    g_hash_table_replace(gStorage, pWindow, value);
    return (isCurrent)?DEFAULT_FST_LAYOUT:DEFAULT_SND_LAYOUT;
}


void 
_kbdd_perwindow_remove(WINDOW_TYPE win)
{
    assert( gStorage != NULL );
    gpointer key = GUINT_TO_POINTER(win);
    g_hash_table_remove(gStorage, key);
}



void
_kbdd_perwindow_clean() 
{
    assert( gStorage != NULL );
    g_hash_table_remove_all(gStorage);
}

#ifdef DEBUG
void debug() {
    GHashTableIter iter;
    g_hash_table_iter_init (&iter, gStorage);
    int size=g_hash_table_size(gStorage);
    printf("=================\nFirst size: %d\n",size);
    gpointer *val;
    gpointer *key_;
    while (g_hash_table_iter_next (&iter, (gpointer) &key_, (gpointer) &val))
    {
        printf("key %p ---> %u \n",GPOINTER_TO_UINT(key_),GPOINTER_TO_UINT(val));
    }
    printf("=================\n");
}
#endif

//vim:ts=4:expandtab
