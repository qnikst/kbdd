#ifndef _M_KBDD_SERVICE_H_
#define _M_KBDD_SERVICE_H_

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>


#define SIGNAL_LAYOUT_CHANGED       "layout_changed"

#define M_DBUS_KBDD_SERVICE_PATH  "/ru/gentoo/KbddService"
#define M_DBUS_KBDD_SERVICE       "ru.gentoo.KbddService"
        
#define M_TYPE_KBDD_SERVICE            (m_kbdd_service_get_type ())
#define M_KBDD_SERVICE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), M_TYPE_KBDD_SERVICE, MKbddService))
#define M_KBDD_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), M_TYPE_KBDD_SERVICE, MKbddServiceClass))
#define M_IS_KBDD_SERVICE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), M_TYPE_KBDD_SERVICE))
#define M_IS_KBDD_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), M_TYPE_KBDD_SERVICE))
#define M_KBDD_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), M_TYPE_KBDD_SERVICE, MKbddServiceClass))

G_BEGIN_DECLS

typedef enum {
    E_SIGNAL_LAYOUT_CHANGED,
    E_SIGNAL_COUNT
} ValueSignalNumber;

typedef struct _MKbddService MKbddService;
typedef struct _MKbddServiceClass MKbddServiceClass;

struct _MKbddService {
            GObject parent;
            unsigned int layout;
};

struct _MKbddServiceClass {
            GObjectClass parent;
            guint signals[E_SIGNAL_COUNT];
};

//void m_test_service_ping (MTestService *object, DBusGMethodInvocation *context);
//int kbdd_next_layout(MKbddService *object, DBusGMethodInvocation * context);
//int kbdd_set_policy(MKbddService *object, unsigned int policy, DBusGMethodInvocation * context);

int m_kbdd_service_next_layout(MKbddService *obj, GError ** error);
int m_kbdd_service_set_policy(MKbddService *obj, unsigned int value, GError**error);

MKbddService *m_kbdd_service_new (void);
GType m_kbdd_service_get_type (void);


G_END_DECLS

#endif

