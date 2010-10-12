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

/**
 * This is a simple dbus testing appication
 *
 *  Thanks to Maemo Diablo Source code for GLib
 *  D-Bus signal example 
 *  Trainig Matherial
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "m-kbdd-service.h"
#include "kbdd-service-glue.h"

int main(int argc, char ** argv) {
    MKbddService * service = NULL;
    DBusGConnection * bus = NULL;
    DBusGProxy * proxy = NULL;
    GMainLoop *mainloop = NULL;

    char * request_ret = NULL;

    unsigned int result;
    GError * error = NULL;
    /* Initialize the GType/GObject system */
    g_type_init();

    mainloop = g_main_loop_new(NULL,FALSE);

    g_print(":main Connecting to the Session D-Bus.\n");
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if ( error != NULL ) 
    {
        fprintf(stderr,"Couldn't connect to session bus: %s\n",error->message);
        exit (EXIT_FAILURE);
    }

    printf(":main Regiresting the well-known name (%s)\n", M_DBUS_KBDD_SERVICE);


    /**
     * In order to register a well-known name, we need to use the 
     * "RequestMethod" of the /org/freedesktop/DBus interface. Each
     * bus provides an object that will implement this interface
     *
     */
    proxy = dbus_g_proxy_new_for_name(bus,
                DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    if ( proxy == NULL) 
    {
        fprintf(stderr,"Failed to get a proxy for D-Bus\n");
    }

    //if (! org_freedesktop_DBus_request_name( proxy, M_DBUS_KBDD_SERVICE,
    //            DBUS_NAME_FLAG_DO_NOT_QUEUE, &request_ret, &error) )
    if (! dbus_g_proxy_call( proxy, 
                "RequestName",
                &error,
                G_TYPE_STRING, M_DBUS_KBDD_SERVICE,
                G_TYPE_UINT, 0,
                G_TYPE_INVALID, 
                G_TYPE_UINT, &result,
                G_TYPE_INVALID))
    {
        fprintf(stderr, "Unable to register service: %s", error->message);
        exit (EXIT_FAILURE);
    }
    printf(":main RequestName returned %d.\n", request_ret);
    if ( result != 1 ) 
    {
        fprintf(stderr,"Failed to get the primary well-known name.\n");
        exit (EXIT_FAILURE);
    }

    printf(":main Creating one MKbddService Object\n");

    service = g_object_new(M_TYPE_KBDD_SERVICE, NULL);
    if (service == NULL) 
    {
        fprintf(stderr,"Failed to create one KbddService instance\n");
        exit (EXIT_FAILURE);
    }

    g_print(":main Registering project to D-Bus.\n");

    dbus_g_object_type_install_info (M_TYPE_KBDD_SERVICE, &dbus_glib_m_kbdd_service_object_info);
    dbus_g_connection_register_g_object(bus, M_DBUS_KBDD_SERVICE_PATH, G_OBJECT(service));
/*
    dbus_register_object( bus, proxy,
            M_TYPE_KBDD_SERVICE,
            &dbus_glib_m_kbdd_service_object_info,
            M_DBUS_KBDD_SERVICE_PATH);*/

    g_print("Ready to serve requests\n");

    g_main_loop_run(mainloop);

}
