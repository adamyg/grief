#ifndef ICONV_H_INCLUDED
#define ICONV_H_INCLUDED
/* $Id: iconv.h,v 1.3 2012/09/03 23:10:14 ayoung Exp $
 *
 * win32 <iconv.h> - libiconv
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

#include <sys/cdefs.h>
#include <sys/types.h>

#if defined(LIBICONV_STATIC)
#   define LIBICONV_LINKAGE
#   define LIBICONV_ENTRY
#elif defined(WIN32)
#   if defined(__LIBICONV_BUILD)
#       define LIBICONV_LINKAGE __declspec(dllexport)
#   else
#       define LIBICONV_LINKAGE __declspec(dllimport)
#   endif
#   define LIBICONV_ENTRY __cdecl
#else
#   define LIBICONV_LINKAGE
#   define LIBICONV_ENTRY
#endif

__BEGIN_DECLS

typedef void *iconv_t;

LIBICONV_LINKAGE iconv_t LIBICONV_ENTRY     iconv_open(const char *dstname, const char *srcname);

LIBICONV_LINKAGE size_t LIBICONV_ENTRY      iconv(iconv_t cd, const char ** __restrict src, size_t * __restrict srcleft,
                                                    char ** __restrict dst, size_t * __restrict dstleft);

LIBICONV_LINKAGE int LIBICONV_ENTRY         iconv_close(iconv_t cd);

LIBICONV_LINKAGE int LIBICONV_ENTRY         iconv_errno(void);

LIBICONV_LINKAGE size_t LIBICONV_ENTRY      __iconv(iconv_t handle, const char **in, size_t *szin, char **out,
	                                                size_t *szout, unsigned flags, size_t *invalids);

LIBICONV_LINKAGE int LIBICONV_ENTRY         __iconv_get_list(char ***rlist, size_t *rsz);

LIBICONV_LINKAGE void LIBICONV_ENTRY        __iconv_free_list(char **list, size_t sz);

LIBICONV_LINKAGE const char *LIBICONV_ENTRY __iconv_PATH_ESDB(void);

LIBICONV_LINKAGE const char *LIBICONV_ENTRY __iconv_PATH_CSMAPPER(void);

LIBICONV_LINKAGE const char *LIBICONV_ENTRY __iconv_PATH_ICONV(void);

__END_DECLS

#endif /*ICONV_H_INCLUDED*/

