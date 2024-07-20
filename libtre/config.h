#ifndef LIBTRE_CONFIG_H_INCLUDED
#define LIBTRE_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.7 2024/07/20 17:41:35 cvsuser Exp $
 * libtre config.h
 *
 *
 */

#if defined(_WIN32) || defined(WIN32)

#include "../libw32/config.h"
#include <malloc.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define inline _inline
#endif

#if defined(_MSC_VER)
#define alloca _alloca
#if (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#endif /*_MSC_VER*/

#else
#include "../include/config.h"

#if defined(__sun)
#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#endif
#endif /*__sun*/

#endif /*!WIN32*/

#endif /*LIBTRE_CONFIG_H_INCLUDED*/

/*end*/
