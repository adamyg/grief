#ifndef LIBTRE_CONFIG_H_INCLUDED
#define LIBTRE_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.3 2017/01/23 00:22:38 cvsuser Exp $
 * libtre config.h
 *
 *
 */


#if defined(WIN32) && !defined(__MINGW32__)
#include "../libw32/config.h"
#include <malloc.h>
#define  inline   _inline
#if (_MSC_VER < 1900)
#define  snprintf _snprintf
#endif

#else
#include "../include/config.h"
#endif

#endif  /*LIBTRE_CONFIG_H_INCLUDED*/
/*end*/
