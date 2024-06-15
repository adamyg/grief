#ifndef LIBONIGRX_CONFIG_H_INCLUDED
#define LIBONIGRX_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.1 2024/06/15 18:41:57 cvsuser Exp $
 * libonigrx config.h
 *
 *
 */

#if defined(_WIN32) || defined(WIN32)

#include "../libw32/config.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define inline _inline
#endif

#if defined(__WATCOMC__)
#define xvsnprintf _vsnprintf /*regint.h*/
#define xsnprintf _snprintf /*regint.h/regposerr.c*/
#define xstrncpy strncpy /*regposerr.c*/
#endif

#else
#include "../include/config.h"
#endif

#endif /*LIBONIGRX_CONFIG_H_INCLUDED*/

/*end*/

