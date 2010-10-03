#include "libkbdd.h"
#include <stdio.h>
#include <X11/Xlib.h>

main()
{
    Kbdd_init();
    Display * display = Kbdd_initialize_display();
    Kbdd_initialize_listeners(display);
    Kbdd_default_loop();
    Kbdd_clean();
}


