#ifndef CONTRIB_CONFIG_H_INCLUDED
#define CONTRIB_CONFIG_H_INCLUDED
/* $Id: contrib_config.h,v 1.12 2018/10/18 00:52:41 cvsuser Exp $
 * contrib <config.h> ...
 *
 *
 * Copyright (c) 1998 - 2014, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

/*config.h*/
#if defined(unix)
#undef _WIN32
#undef _WIN32_NT
#elif !defined(_WIN32)
#define _WIN32
#define _WIN32_NT=0x400
#endif
#define APPLICATIONDIR  "Grief"                 /* FIXME, edconfig.h */

#if defined(__MINGW32__) || defined(unix)
#include "../../include/config.h"
#else
#include "../../libw32/config.h"
#include <sys/utypes.h>                         /* unix types */
#endif

#if !defined(HAVE_STRTOUL)
#error  HAVE_STRTOUL
#define HAVE_STRTOUL
#endif
#if !defined(HAVE_RENAME)
#error  HAVE_RENAME
#define HAVE_RENAME
#endif
#if !defined(HAVE_STRERROR)
#error  HAVE_STRERROR
#define HAVE_STRERROR
#endif

/*warnings*/
#if defined(__WATCOMC__)
#if !defined(__cplusplus)
#pragma disable_message(124)                    /* Comparison result always 0 */
#pragma disable_message(136)                    /* Comparison equivalent to 'unsigned == 0' */
#endif
#pragma disable_message(201)                    /* Unreachable code */
#pragma disable_message(202)                    /* Symbol 'xxx' has been defined, but not referenced */

#elif defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1              /* Disable deprecate warnings */
#endif
#endif

/*support macros*/
#ifndef __DECONST
#define __DECONST(__t,__a)      ((__t *)(const void *)(__a))
#endif
#ifndef __UNCONST
#define __UNCONST(__a)          ((void *)(const void *)(__a))
#endif

#ifndef __UNVOLATILE
#define __UNVOLATILE(__a)       ((void *)(unsigned long)(volatile void *)(__a))
#endif

/*types*/
    /*inttypes.h*/
#if !defined(HAVE_INTTYPES_H)
#if !defined(__MINGW32__)
typedef long long intmax_t;
typedef unsigned long long uintmax_t;
#endif
#endif /*HAVE_INTTYPES_H*/

    /*stdtypes.h*/
#if defined(HAVE_STDINT_H)
#include <stdint.h>
#if !defined(HAVE_DECL_SIZE_MAX)
#define HAVE_DECL_SIZE_MAX 1
#endif
#if !defined(HAVE_DECL_UINT32_MAX)
#define HAVE_DECL_UINT32_MAX 1
#define HAVE_DECL_UINT64_MAX 1
#define HAVE_DECL_INT64_MAX 1
#define HAVE_DECL_INT64_MIN 1
#endif
#if defined(_MSC_VER)
typedef int ssize_t;
#endif

#else /*!HAVE_STDINT_H*/
#define INTMAX_MIN LLONG_MAX
#define INTMAX_MAX LLONG_MIN
#define UINTMAX_MAX ULLONG_MAX
#if defined(_MSC_VER)
typedef int ssize_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#endif
#endif /*HAVE_STDINT_H*/

#if !defined(HAVE_U_INT32_T)
#if !defined(__WATCOMC__)
typedef unsigned int u_int32_t;
typedef unsigned short u_int16_t;
#endif
typedef unsigned short u_int16_t;
typedef unsigned char u_int8_t;
#endif

    /*others*/
#if !defined(HAVE_PID_T)
#define HAVE_PID_T
typedef int pid_t;
#endif
#if defined(_MSC_VER)
#if !defined(HAVE_UID_T)
#if !defined(uid_t) && !defined(gid_t)
typedef int uid_t;
typedef int gid_t;
#endif
#define HAVE_UID_T
#endif /*uid_t, gid_t*/
#if !defined(HAVE_MODE_T)
#if !defined(mode_t)
typedef unsigned short mode_t;
#endif
#define HAVE_MODE_T
#endif /*mode_t*/
#if !defined(HAVE_ID_T)
#if !defined(id_t)
typedef int id_t;                               /* general identifier; can contain least a pid_t, uid_t, or gid_t. */
#endif
#define HAVE_ID_T
#endif /*id_t*/
#endif /*_MSC_VER*/

/*function mappings*/
#if !defined(__cplusplus)
#if defined(_MSC_VER) || defined(__WATCOMC__)
#if (_MSC_VER < 1500)	/* MSVC 2008 */
#define vsnprintf _vsnprintf
#endif
#if (_MSC_VER < 1700)	/* MSVC 2012 */
#define snprintf _snprintf
#endif /*1500*/
#define strdup _strdup
#define stricmp _stricmp
#define mktemp _mktemp
#if (_MSC_VER < 1500)	/* MSVC 2008 */
#define open _open
#define close _close
#define read _read
#define write _write
#endif /*1500*/
#define access _access
#define lseek _lseek
#define unlink _unlink
#define lstat w32_lstat
#define readlink w32_readlink
#endif /*_MSC_VER || __WATCOMC__*/
#endif /*__cplusplus*/

#endif /*CONTRIB_CONFIG_H_INCLUDED*/
