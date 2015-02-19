#ifndef LIBTRE_CONFIG_H_INCLUDED
#define LIBTRE_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.2 2014/07/15 15:04:38 ayoung Exp $
 * libtre config.h
 *
 *
 */


#if defined(WIN32) && !defined(__MINGW32__)
#include "../libw32/config.h"
#include <malloc.h>
#define  inline   _inline
#define  snprintf _snprintf

#else
#include "../include/config.h"
#endif

#endif  /*LIBTRE_CONFIG_H_INCLUDED*/
/*end*/
