#ifndef LIBTRE_CONFIG_H_INCLUDED
#define LIBTRE_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.4 2020/06/18 13:16:58 cvsuser Exp $
 * libtre config.h
 *
 *
 */

#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)

#include "../libw32/config.h"
#include <malloc.h>

#define inline _inline
#if defined(_MSC_VER)
#define alloca _alloca
#if (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#endif //_MSC_VER

#else
#include "../include/config.h"
#endif

#endif  /*LIBTRE_CONFIG_H_INCLUDED*/
/*end*/
