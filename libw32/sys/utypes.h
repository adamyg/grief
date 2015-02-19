#ifndef GR_UTYPES_H_INCLUDED
#define GR_UTYPES_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_utypes_h,"$Id: utypes.h,v 1.20 2015/02/19 00:17:39 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* 
 * win32 unix types
 *
 * Copyright (c) 1998 - 2015, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#error utypes.h: untested MSVC c/c++ Version (CL 12.xx - 16.xx) only ...
#endif
#endif
#endif
#pragma warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error utypes.h: old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)

#else
#error utypes.h: Unknown compiler
#endif

#include <sys/types.h>                          /* System types */

#if !defined(_POSIX_SOURCE) && \
        !defined(_UNIXTYPES_T_DEFINED) && \
        !defined(u_char)
#define _UNIXTYPES_T_DEFINED
#if defined(_BSD_SOURCE)
#if !defined(_BSDTYPES_DEFINED)
typedef unsigned char   u_char;                 /* BSD compatibility */
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
#define _BSDTYPES_DEFINED                       /* winsock[2].h and others */
#endif /*_BSDTYPES_DEFINED*/
#endif /*_BSD_SOURCE*/
typedef unsigned char   uchar;                  /* Sys V compatibility */
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
#endif

/* [u]int8_t, [u]int16_t, [u]int32_t optional [u]int64_t */
#if defined(HAVE_STDINT_H)
#include <stdint.h>
#else
#if defined(_MSC_VER) && !defined(_MSC_STDINT_H_TYPES)
#if (_MSC_VER < 1300)
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#else
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#endif  /*1300*/
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define _MSC_STDINT_H_TYPES
#endif  /*_MSC_STDINT_H_TYPES*/
#endif  /*stdint.h*/

#if defined(_BSD_SOURCE)
#ifndef u_int64_t
#define u_int64_t uint64_t
#endif
#ifndef u_int32_t
#define u_int32_t uint32_t
#endif
#ifndef u_int16_t
#define u_int16_t uint16_t
#endif
#ifndef u_int8_t
#define u_int8_t uint8_t
#define __uint8_t uint8_t
#endif
typedef char *caddr_t;                          /* core address */
typedef long daddr_t;                           /* disk address */
typedef unsigned long fixpt_t;                  /* fixed point number */
#endif  /*BSD_SOURCE*/

/* system identifiers */
typedef int pid_t;                              /* process identifier */

typedef long suseconds_t;                       /* sys/types.h */

#if defined(_MSC_VER)
#if !defined(uid_t) && !defined(gid_t)
typedef int uid_t;
typedef int gid_t;
#endif
typedef int id_t;                               /* used as a general identifier; can contain least a pid_t, uid_t, or gid_t. */
typedef int ssize_t;
typedef unsigned short mode_t;

#elif defined(__MINGW32__)
#if !defined(uid_t) && !defined(gid_t)
typedef int uid_t;
typedef int gid_t;
#endif
#endif

typedef unsigned nlink_t;                       /* link count */

#ifndef major
#define major(devnum)   (((devnum) >> 8) & 0xff)
#endif
#ifndef minor
#define minor(devnum)   (((devnum) & 0xff))
#endif
#ifndef makedev
#define makedev(major,minor) \
                        ((((major) & 0xff) << 8) | ((minor) & 0xff))
#endif

#endif /*GR_UTYPES_H_INCLUDED*/
