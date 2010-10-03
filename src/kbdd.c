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
#include <X11/Xlib.h>

#include "libkbdd.h"

int main(int argc, char * argv[])
{
    Kbdd_init();
    Display * display;
    display = Kbdd_initialize_display();
    Kbdd_initialize_listeners(display);
    Kbdd_default_loop();
    Kbdd_clean();
    return (EXIT_SUCCESS);
}


