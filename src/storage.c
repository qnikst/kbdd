#include <X11/Xlib.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef STORAGE_GHASH
#include <glib.h>
#endif

#ifdef STORAGE_GHASH
GHashTable *gStorage = NULL;
#endif

void _kbdd_storage_init() {
    if ( gStorage!=NULL ) return; 
#ifdef STORAGE_GHASH
    gStorage = g_hash_table_new(g_int_hash, g_int_equal);
#endif
}

void _kbdd_storage_free() {
    if ( gStorage != NULL ) {
#ifdef STORAGE_GHASH
        g_hash_table_destroy(gStorage);
#endif
    }
}

void _kbdd_storage_put(Window win, int group)
{
    if ( gStorage != NULL ) {
#ifdef STORAGE_GHASH
        int * stored_group = (int *)malloc( sizeof (int) );
        g_hash_table_replace(gStorage, GINT_TO_POINTER(win), stored_group);
    }
#endif
}

int _kbdd_storage_get(Window win)
{
    int group;
    if (gStorage != NULL) 
    {
#ifdef STORAGE_GHASH
        gpointer pGroup = g_hash_table_lookup(gStorage, &win  );
        group = GPOINTER_TO_INT(pGroup);
#endif
    }
    return group;
}

void _kbdd_storage_remove(Window win)
{
    if (gStorage != NULL) {
#ifdef STORAGE_GHASH
        g_hash_table_remove(gStorage, &win);
#endif
    }
}

