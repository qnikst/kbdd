#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H

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

// common settings for all KBDD source files
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//void current() {
//    size_t s=strftime(_tout,12,"[%H:%M:%S]",localtime(NULL));
//    _tout[s+1] = NULL;
//}
#define dbg(fmtstr,args...) \
    do { \
     /*char _tout[64]; \
     time_t * _tmt = time(NULL); \
     struct tm _tm = localtime(_tmt); \
     size_t s=strftime((char *)&_tout,12,"[%H:%M:%S]",&_tm); \
     _tout[s+1] = NULL; \
     printf("%s %s " fmtstr "\n",(char *)_tout,__func__,##args);*/ \
     printf(fmtstr "\n",##args); \
     } while (0)
#else
#define dbg(dummy...)
#endif

#define DEFAULT_FST_LAYOUT 0
#define DEFAULT_SND_LAYOUT 1

#endif

//vim:ts=4:expandtab
