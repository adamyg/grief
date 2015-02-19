/* $Id: namespace.h,v 1.3 2012/09/03 23:10:15 ayoung Exp $
 *
 * libiconv <namespace.h>
 *
 * Copyright (c) 2012 Adam Young.
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
#pragma warning(disable:4996)                   /* 'xxx' was declared deprecated */
#endif

#include <sys/types.h>                          /* system types */
#include <sys/utypes.h>                         /* unix/bsd types */
#include <sys/param.h>                          /* system parameters */
#include <string.h>
#include <unistd.h>

#ifndef u_int32_t
#define u_int32_t       uint32_t
#define u_int16_t       uint16_t
#define u_int8_t        uint8_t
#endif

#ifndef __dead
#define __dead          /*__attribute__((noreturn))*/
#endif

#ifndef snprintf
#define snprintf        _snprintf
#define vsnprintf       _vsnprintf
#endif
#ifndef open
#define open            _open
#define read            _read
#define write           _write
#endif
#ifndef strdup
#define strdup          _strdup
#endif

#if defined(_MSC_VER)
#define LC_MESSAGES     (LC_MAX + 1)
#endif

/*end*/
