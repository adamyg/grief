#ifndef DBHASH_NAMESPACE_H_INCLUDED
#define DBHASH_NAMESPACE_H_INCLUDED
/* $Id: namespace.h,v 1.7 2015/02/19 00:17:00 ayoung Exp $
 *
 * libbsdb <namespace.h>
 *
 * Copyright (c) 2012-2015 Adam Young.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ==end==
 */

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <config.h>

#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable:4996)                   /* 'xxx' was declared deprecated */
#elif defined(__WATCOMC__)
#pragma disable_message(124)                    /* Comparison result always 0 */
#pragma disable_message(136)                    /* Comparison equivalent to 'unsigned == 0' */
#pragma disable_message(201)                    /* Unreachable code */
#pragma disable_message(202)                    /* Unreferenced */
#endif

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <edendian.h>
#if defined(WIN32)
#include <sys/utypes.h>
#include <win32_io.h>
#endif
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef _DIAGASSERT
#define _DIAGASSERT(_a)         /**/
#endif

#if !defined(BIG_ENDIAN)
#define LITTLE_ENDIAN           1234
#define BIG_ENDIAN              4321
#if defined(HOST_LITTLE_ENDIAN)
#define BYTE_ORDER              LITTLE_ENDIAN
#elif defined(HOST_BIG_ENDIAN)
#define BYTE_ORDER              BIG_ENDIAN
#else
#error Unsupported/unknown host endian
#endif
#endif

#if defined(MODULE_STATIC)
#   define MODULE_LINKAGE
#   define MODULE_ENTRY
#elif defined(_WIN32) || defined(WIN32)
#   if defined(__MODULE_BUILD)
#       define MODULE_LINKAGE __declspec(dllexport)
#   else
#       define MODULE_LINKAGE __declspec(dllimport)
#   endif
#   define MODULE_ENTRY __cdecl
#else
#   define MODULE_LINKAGE
#   define MODULE_ENTRY
#endif

#ifndef __CONCAT
#if defined(__STDC__) || defined(_MSC_VER) || defined(__WATCOMC__) || \
            defined(__cplusplus)
#define __CONCAT(x,y)           x ## y
#else
#define __CONCAT(x,y)           x/**/y
#endif
#endif

#ifndef __UNCONST
#define __UNCONST(a)            ((void *)(unsigned long)(const void *)(a))
#endif

#ifndef MAX
#define MIN(__a, __b)           ((__a) < (__b) ? (__a) : (__b))
#define MAX(__a, __b)           ((__a) > (__b) ? (__a) : (__b))
#endif

#if defined(WIN32)
#ifndef snprintf
#define snprintf                _snprintf
#define vsnprintf               _vsnprintf
#endif
#ifndef open
#define open                    _open
#define read                    _read
#define write                   _write
#endif
#ifndef strdup
#define strdup                  _strdup
#endif
#endif /*WIN32*/

#ifndef EFTYPE
#define EFTYPE                  EPROTONOSUPPORT
#endif

#endif /*DBHASH_NAMESPACE_H_INCLUDED*/


