/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: grief_head.h,v 1.9 2014/10/27 23:28:21 ayoung Exp $
 * Common GRIEF macro definitions -- head
 * Utilised by the makeinc.pl script.
 *
 *
 */

/*--export--*/
#ifndef MACSRC_GRIEF_H_INCLUDED
#define MACSRC_GRIEF_H_INCLUDED
/* -*- mode: cr; indent-width: 4; -*- */
/* grief.h --- common definitions
 *
 *  An auto-generated file, do not modify
 */

#include "alt.h"                            /* keycodes */

#if defined(OS2)
#define HPFS
#endif

#if defined(UNIX)  && !defined(unix)
#define unix
#endif
#if defined(LINUX) && !defined(linux)
#define linux
#elif defined(SUN) && !defined(sun)
#define sun
#endif

/*
 *  Booleans
 */
#ifndef TRUE
#define TRUE                    1
#define FALSE                   0
#endif

#define CLICK_TIME_MS           250

#define APPNAME                 "GRIEF"     /* our name */

/*--end--*/
