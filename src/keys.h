#ifndef _KEYS_H
#define _KEYS_H

#define MODKEY Mod1Mask

static Key keys[] = {
    { MODKEY|ShiftMask, XK_1, _set_current_window_layout, {.ui=0} },
    { MODKEY|ShiftMask, XK_2, _set_current_window_layout, {.ui=1} },
    { MODKEY|ShiftMask, XK_3, _set_current_window_layout, {.ui=2} },
    { MODKEY|ShiftMask, XK_4, _set_current_window_layout, {.ui=3} }
};
#endif
