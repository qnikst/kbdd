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
#include "m-kbdd-service.h"
#include "../common-defs.h"
#include "../libkbdd.h"

#include "assert.h"

G_DEFINE_TYPE(MKbddService, m_kbdd_service, G_TYPE_OBJECT)

static void
m_kbdd_service_finalize (GObject *object)
{
    G_OBJECT_CLASS (m_kbdd_service_parent_class)->finalize (object);
}


static void
m_kbdd_service_class_init (MKbddServiceClass *klass)
{
    const char * signalNames[E_SIGNAL_COUNT] = {
        SIGNAL_LAYOUT_CHANGED ,
        SIGNAL_LAYOUT_NAME_CHANGED
    };
    int i;

    //Debug
    assert( klass != NULL);

    GObjectClass *object_class;
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = m_kbdd_service_finalize;

    /* SIGNAL_LAYOUT_CHANGED */ {
        guint signalId;
        signalId = g_signal_new(signalNames[0],
            G_OBJECT_CLASS_TYPE(klass), //Gtype to which signal is bound to
            G_SIGNAL_RUN_LAST,
            0,
            NULL,//Accumulator
            NULL,//Userdata Accumulator
            g_cclosure_marshal_VOID__UINT,
            G_TYPE_NONE,
            1,
            G_TYPE_UINT);
        klass->signals[0] = signalId;
    }

    /* SIGNAL_LAYOUT_NAME_CHANGED */ {
        guint signalId;
        signalId = g_signal_new(signalNames[1],
                G_OBJECT_CLASS_TYPE(klass), 
                G_SIGNAL_RUN_LAST,
                0,
                NULL,
                NULL,
                g_cclosure_marshal_VOID__STRING,
                G_TYPE_NONE,
                1,
                G_TYPE_STRING);
        klass->signals[1] = signalId;
    }

}

static void
m_kbdd_service_init (MKbddService *object)
{
    assert( object != NULL );
}


MKbddService *
m_kbdd_service_new (void)
{
    return g_object_new (M_TYPE_KBDD_SERVICE, NULL);
}

/*
static void
kbdd_service_emitSignal(MKbddService * obj,
                        ValueSignalNumber num,
                        const char * message) 
{
    MKbddServiceClass * klass = M_KBDD_SERVICE_GET_CLASS(obj);
    assert(obj!=NULL);
    assert(klass!=NULL);
    assert( (num < E_SIGNAL_COUNT) && (num>0) );

    g_signal_emit(obj, klass->signals[num], 0, message);
}

int
m_kbdd_service_setLayout(MKbddService * obj, unsigned int valueIn,
        GError ** error) 
{
    assert(obj != NULL);
    if (obj->layout != valueIn) 
    {
        obj->layout = valueIn;
        // Kbdd_set_layot(id);
        kbdd_service_emitSignal(obj, E_SIGNAL_LAYOUT_CHANGED, "layout_changed");
    }
    return 1;
}*/

/**
 * Get current layout
 * @param uint32_t valueOut - current layout
 *
 * @TODO: set normal types 
 */
int 
m_kbdd_service_get_layout(MKbddService * obj, unsigned int * valueOut, GError ** error)
{
    assert( obj != NULL );
    assert( valueOut != NULL );
    *valueOut = obj->layout;
    return 1;
}

/**
 * Switch current window to the next layout
 *
 * @TODO implement
 * @TODO improve avalability to set previous layout
 */
int
m_kbdd_service_next_layout(MKbddService *obj, GError ** error) 
{
    kbdd_set_next_layout();
    return 1;
}

int 
m_kbdd_service_prev_layout(MKbddService *obj, GError ** error)
{
    kbdd_set_previous_layout();
    return 1;
}

int 
m_kbdd_service_set_policy(MKbddService *obj, unsigned int value, GError**error)
{
    //Not yet implements (and I think will never be)
    //*error = "feature not implemented yeti\0";
    return 0;
}

/**
 *  Get symbolic layout name 
 *  @param uint32_t id   - layout id
 *  @param char * result - result 
 */
int 
m_kbdd_service_get_layout_name(MKbddService *obj, unsigned int id, char ** value, GError **error)
{
    char * tmp; 
    if ( kbdd_get_layout_name(id, &tmp) ) 
    {
        dbg("returned: %s", tmp);
//      dbg("value addr %p", *value);
        *value = tmp;
        return 1;
    }
    return 0;
}

/**
 * set layout
 * @param uint32_t new layout
 */
void
m_kbdd_service_update_layout(MKbddService *obj, uint32_t value, const char * layout_name)
{
    assert(obj != NULL);
    if (obj->layout != value) 
    {
        obj->layout = value;
        MKbddServiceClass * klass = M_KBDD_SERVICE_GET_CLASS(obj);
        assert(klass != NULL);
        dbg(" set layout event (emmitting signal)");
        g_signal_emit(obj, klass->signals[E_SIGNAL_LAYOUT_CHANGED], 0, value);
        if (layout_name != NULL)
        {
//          dbg(" set layout event (emmitting signal2)");
          g_signal_emit(obj, klass->signals[E_SIGNAL_LAYOUT_NAME_CHANGED], 0, layout_name);
        }
    }
}

int
m_kbdd_service_set_layout(MKbddService *obj, uint32_t value, GError **error) 
{
    //TODO check min max value
    dbg("dbus-set layout %u",value);
    kbdd_set_current_window_layout(value);
    return 1;
}

int 
m_kbdd_service_get_current_layout(MKbddService *obj,uint32_t *value,GError **error)
{
    *value = kbdd_get_current_layout();
    return 1;
}
