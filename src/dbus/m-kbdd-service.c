#include "m-kbdd-service.h"
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
        SIGNAL_LAYOUT_CHANGED
    };
    int i;

    //Debug
    assert( klass != NULL);

    GObjectClass *object_class;
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = m_kbdd_service_finalize;

    for ( i = 0; i <E_SIGNAL_COUNT; i++ ) {
        guint signalId;
        signalId = g_signal_new(signalNames[i],
            G_OBJECT_CLASS_TYPE(klass), //Gtype to which signal is bound to
            G_SIGNAL_RUN_LAST,
            0,
            NULL,//Accumulator
            NULL,//Userdata Accumulator
            g_cclosure_marshal_VOID__UINT,
            G_TYPE_NONE,
            1,
            G_TYPE_UINT);
        klass->signals[i] = signalId;
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

static void
kbdd_service_emitSignal(MKbddService * obj,
                        ValueSignalNumber num,
                        const char * message) 
{
    MKbddServiceClass * klass = M_KBDD_SERVICE_GET_CLASS(obj);

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
        kbdd_service_emitSignal(obj, E_SIGNAL_LAYOUT_CHANGED, "layout_changed");
    }
    return 1;
}

int 
m_kbdd_service_getLayout(MKbddService * obj, unsigned int * valueOut, GError ** error)
{
    assert( obj != NULL );
    assert( valueOut != NULL );
    *valueOut = obj->layout;
    return 1;
}
/*
int
kbdd_layout_changed(GObject self, unsigned int new_state) 
{
    return 1;
}
*/
int
m_kbdd_service_next_layout(MKbddService *obj, GError ** error) 
{
    return 1;
}

int 
m_kbdd_service_set_policy(MKbddService *obj, unsigned int value, GError**error)
{
    return 1;
}

void
m_kbdd_service_set_layout(MKbddService *obj, unsigned int value)
{
    assert(obj != NULL);
    if (obj->layout != value) 
    {
        obj->layout = value;
        kbdd_service_emitSignal(obj, E_SIGNAL_LAYOUT_CHANGED, "layout_changed");
    }
}
