/**
 * vim ts=4:expandtab
 *
 * kbdd - simple application for handling per window kbd layout
 *
 * © 2010 Alexander V Vershilov and contributors
 *
 * See file LICENSE for license information.
 *
 * src/kbdd.c: simple libkbdd client
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <errno.h>

#ifdef ENABLE_DBUS
#include <pthread.h>
#include <glib.h>
#include <glib/gthread.h>
#include "dbus/m-kbdd-service.h"
#include "dbus/kbdd-service-glue.h"
#endif

#include "libkbdd.h"
#include "common-defs.h"

#define OPEN_MAX_GUESS 256

#ifdef ENABLE_DBUS
MKbddService * service = NULL;
DBusGConnection * bus  = NULL;
DBusGProxy * proxy     = NULL;
#endif

int main_fork()
{
    pid_t pid,sid;
    int i,stdioFD,numFiles;

    pid = fork();

    switch ( pid )
    {
        case 0:
            break;
        case -1:
            fprintf(stderr,"Error: initial fork failed: %s\n",strerror(errno));
            exit ( EXIT_FAILURE );
            break;
        default:
            exit ( EXIT_SUCCESS );
    }

    sid = setsid();
    if ( sid < 0 ) 
    {
        fprintf(stderr,"Error: setsid failed: %s\n", strerror(errno));
        exit( EXIT_FAILURE );
    }
    printf("kbdd pid: %i\n",sid);

    numFiles = sysconf(_SC_OPEN_MAX);
    if (numFiles<0)
        numFiles=OPEN_MAX_GUESS;

    for (i=numFiles-1;i>=0;i--)
        close(i);
    
    umask(0);

    stdioFD = open("/dev/null",O_RDWR);
    dup(stdioFD);
    dup(stdioFD);
    return 0;
}

#ifdef ENABLE_DBUS
int dbus_init( ) {

    char * request_ret = NULL;
    unsigned int result;
    GError * error = NULL;

    /* Initialize the GType/GObject system */
    g_type_init();


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
    printf(":main RequestName returned %s.\n", request_ret);
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

    g_print("Ready to serve requests\n");
    return 1;
}


void onLayoutUpdate(uint32_t layout, void * obj) 
{
    dbg(" EVENT LAYOUT CHANGED %u\n", layout);
    m_kbdd_service_set_layout((MKbddService *)obj,layout);
}
#endif


int main(int argc, char * argv[])
{

#ifndef NO_DAEMON
#ifndef DAEMON
    main_fork();
#else
    if ( daemon(0,0) != 0 ) {
        perror("Failed to daemonize.\n");
    }
#endif
#else
    printf("Not daemonizing (build with NO_DAEMON-build define)\n");
#endif

#ifdef ENABLE_DBUS
    g_type_init();
    GMainLoop * mainloop = NULL;
    mainloop = g_main_loop_new(NULL,FALSE);
    if ( !g_thread_supported () ) {
        dbg("gthread not supported  - initializing");
        g_thread_init ( NULL );
    }
    dbus_g_thread_init ();
    dbus_init();
#endif

    Kbdd_init();
    Display * display;
    display = Kbdd_initialize_display();
    Kbdd_initialize_listeners(display);
#ifndef ENABLE_DBUS
    Kbdd_default_loop(display);
#else
    Kbdd_setDisplay(display);
    Kbdd_setupUpdateCallback(onLayoutUpdate, service);
    g_timeout_add(100, Kbdd_default_iter, mainloop);
    g_main_loop_run(mainloop);
//    Kbdd_default_loop(display);
//    pthread_t thread1;
//    pthread_create(  &thread1, NULL, Kbdd_default_loop, NULL);
//    g_main_loop_run(mainloop);
//    pthread_join(thread1, NULL);
#endif
    Kbdd_clean();
    return (EXIT_SUCCESS);
}


