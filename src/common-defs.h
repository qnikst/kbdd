#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H
/**
 * KBDD simple keyboad daemon lib
 *
 * For license purposes see LICENSE file
 *
 * © Alexander V Vershilov & collaborators
 */
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "KBDD"
#endif

#ifdef DEBUG
#define dbg(fmtstr,args...) \
    (printf(PACKAGE_NAME" :%s " fmtstr "\n",__func__,##args))
#else
#define dbg(dummy...)
#endif

#endif
