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
#ifndef _M_KBDD_SERVICE_H_
#define _M_KBDD_SERVICE_H_

#include <stdint.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>


#define SIGNAL_LAYOUT_CHANGED       "layout_changed"
#define SIGNAL_LAYOUT_NAME_CHANGED  "layout_name_changed"

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
    E_SIGNAL_LAYOUT_NAME_CHANGED,
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
/** DBUS METHODS **/
//set layout to the current one
int m_kbdd_service_set_layout(MKbddService *obj, uint32_t value, GError ** error);
//change layout to the next one
int m_kbdd_service_next_layout(MKbddService *obj, GError ** error);
//change layout to the previous one
int m_kbdd_service_prev_layout(MKbddService *obj, GError ** error);
//set policy (not yet implemented)
int m_kbdd_service_set_policy(MKbddService *obj, unsigned int value, GError**error);
//get layout name dbus method
int m_kbdd_service_get_layout_name(MKbddService *, uint32_t, char **, GError **);
//get current layout id
int m_kbdd_service_get_current_layout(MKbddService *,uint32_t *,GError **);

/** INNER KBDD METHODS **/
void m_kbdd_service_update_layout(MKbddService *, uint32_t, const char * );
/** SERVICE METHODS **/
MKbddService *m_kbdd_service_new (void);
GType m_kbdd_service_get_type (void);


G_END_DECLS

#endif

