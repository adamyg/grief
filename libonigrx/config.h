#ifndef LIBONIGRX_CONFIG_H_INCLUDED
#define LIBONIGRX_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.4 2024/08/24 15:11:13 cvsuser Exp $
 * libonigrx config.h
 *
 *
 */

#if defined(_WIN32) || defined(WIN32)

#include "../libw32/config.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
#if !defined(inline)
#define inline _inline
#endif
#endif

#if defined(__WATCOMC__)
#define xvsnprintf _vsnprintf /*regint.h*/
#define xsnprintf _snprintf /*regint.h/regposerr.c*/
#define xstrncpy strncpy /*regposerr.c*/
#endif

#else
#include "../include/config.h"

#if defined(__sun)
#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#endif
#endif /*__sun*/

#endif /*!WIN32*/

#endif /*LIBONIGRX_CONFIG_H_INCLUDED*/

/*end*/
