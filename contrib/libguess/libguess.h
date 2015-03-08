#ifndef LIBGUESS_H_INCLUDED
#define LIBGUESS_H_INCLUDED
/*
 *   libguess-1.1 - this code is derivative of guess.c of Gauche-0.8.3.
 *   Copyright (c) 2000-2003 Shiro Kawai
 *   Copyright (c) 2005-2010 Yoshiki Yazawa
 *   Copyright (c) 2007-2010 William Pitcock
 *   Copyright (c) 2012-2015 Adam Young
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <../contrib_config.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#if defined(LIBGUESS_STATIC)
#   define LIBGUESS_LINKAGE
#   define LIBGUESS_ENTRY
#elif defined(_WIN32) || defined(WIN32)
#   if defined(__LIBGUESS_BUILD)
#       define LIBGUESS_LINKAGE __declspec(dllexport)
#   else
#       define LIBGUESS_LINKAGE __declspec(dllimport)
#   endif
#   define LIBGUESS_ENTRY __cdecl
#else
#   define LIBGUESS_LINKAGE
#   define LIBGUESS_ENTRY
#endif

#if defined(__LIBGUESS_BUILD)
/*
 *  libguess internals ...
 */
#if defined(_WIN32) || defined(WIN32)
#include <sys/utypes.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_INLINE)
#elif defined(HAVE___INLINE)
#define inline              __inline
#elif defined(_MSC_VER) || defined(__WATCOMC__)
#define inline              __inline
#elif defined(__GNUC__)
#define inline              __inline
#else
#define inline
#endif

typedef const char *(*guess_impl_f)(const char *buf, int len);

const char *                guess_jp(const char *buf, int buflen);
const char *                guess_tw(const char *buf, int buflen);
const char *                guess_cn(const char *buf, int buflen);
const char *                guess_kr(const char *buf, int buflen);
const char *                guess_ru(const char *buf, int buflen);
const char *                guess_ar(const char *buf, int buflen);
const char *                guess_tr(const char *buf, int buflen);
const char *                guess_gr(const char *buf, int buflen);
const char *                guess_hw(const char *buf, int buflen);
const char *                guess_pl(const char *buf, int buflen);
const char *                guess_bl(const char *buf, int buflen);

#endif  /*__LIBGUESS_BUILD*/

#define GUESS_REGION_JP     "japanese"
#define GUESS_REGION_TW     "taiwanese"
#define GUESS_REGION_CN     "chinese"
#define GUESS_REGION_KR     "korean"
#define GUESS_REGION_RU     "russian"
#define GUESS_REGION_AR     "arabic"
#define GUESS_REGION_TR     "turkish"
#define GUESS_REGION_GR     "greek"
#define GUESS_REGION_HW     "hebrew"
#define GUESS_REGION_PL     "polish"
#define GUESS_REGION_BL     "baltic"

__BEGIN_DECLS

LIBGUESS_LINKAGE int LIBGUESS_ENTRY libguess_validate_utf8(const char *buf, int buflen);

LIBGUESS_LINKAGE const char *LIBGUESS_ENTRY libguess_determine_encoding(const char *buf, int buflen, const char *langset);

__END_DECLS

#endif /*LIBGUESS_H_INCLUDED*/

