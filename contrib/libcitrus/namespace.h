#ifndef LOCAL_NAMESPACE_H_INCLUDED
#define LOCAL_NAMESPACE_H_INCLUDED
/* $Id: namespace.h,v 1.8 2018/09/29 02:25:20 cvsuser Exp $
 *
 * libcitrus <namespace.h>
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

#ifdef  _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable:4996)                   // 'xxx' was declared deprecated
#elif defined(__WATCOMC__)
#pragma disable_message(124)                    // Comparison result always 0
#pragma disable_message(136)                    // Comparison equivalent to 'unsigned == 0'
#pragma disable_message(201)                    // Unreachable code
#pragma disable_message(202)                    // Unreferenced
#endif

#define  _BSD_SOURCE
#include <config.h>
#include <sys/types.h>                          /* system types */
#include <sys/utypes.h>                         /* unix/bsd types */
#include <sys/param.h>                          /* system parameters */
#include <sys/endian.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#define _I18N_DYNAMIC 1
/*#define _I18N_STATIC 1        * issues/incomplete */
/*#define _I18N_NONE 1*/
#if defined(_I18N_STATIC)
#define _I18N_STATIC_BIG5
#define _I18N_STATIC_EUC
#define _I18N_STATIC_EUCTW
#define _I18N_STATIC_ISO2022
#define _I18N_STATIC_MSKanji
#define _I18N_STATIC_UTF8
#endif

#if defined(MODULE_STATIC)
#   define MODULE_LINKAGE
#   define MODULE_ENTRY
#elif defined(WIN32)
#   define MODULE_LINKAGE       __declspec(dllexport)
#   define MODULE_ENTRY         __cdecl
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

#if defined(_MSC_VER)
#define __restrict      /**/
#endif

#ifndef u_int32_t
#define u_int64_t       uint64_t
#define u_int32_t       uint32_t
#define u_int16_t       uint16_t
#define u_int8_t        uint8_t
#define __uint8_t       uint8_t
#endif

#ifndef snprintf
#if (_MSC_VER != 1500)  /* MSVC 2008 (v9.0) */
#define vsnprintf       _vsnprintf
#endif /*1500*/
#define snprintf        _snprintf
#endif

#ifndef open
#if (_MSC_VER != 1500)  /* MSVC 2008 (v9.0) */
#define open            _open
#define read            _read
#define write           _write
#endif
#endif

#ifndef strdup
#define strdup          _strdup
#endif

#define bcopy(_s,_d,_l) memmove(_d,_s,_l)

#ifdef  EPROTONOSUPPORT
#define EFTYPE          EPROTONOSUPPORT
#else
#define EFTYPE          WSAEPROTONOSUPPORT
#endif

#if defined(_MSC_VER)
#define LC_MESSAGES     (LC_MAX + 1)
#endif

#endif  /*LOCAL_NAMESPACE_H_INCLUDED*/
