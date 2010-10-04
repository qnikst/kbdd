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


#include <X11/Xlib.h>
#include <errno.h>


#include "libkbdd.h"

#define OPEN_MAX_GUESS 256

int main_proc()
{
    Kbdd_init();
    Display * display;
    display = Kbdd_initialize_display();
    Kbdd_initialize_listeners(display);
    Kbdd_default_loop();
    Kbdd_clean();
}


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
        fprintf("Error: setsid failed: %s\n", strerror(errno));
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
    setpgrp();
}



int main(int argc, char * argv[])
{
    main_fork();
    main_proc();
    return (EXIT_SUCCESS);
}


