/*-
 * Copyright (c) 1998-1999 Brett Lymn
 *                         (blymn@baea.com.au, brett_lymn@yahoo.com.au)
 * All rights reserved.
 *
 * This code has been donated to The NetBSD Foundation by the Author.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

#include <assert.h>

/*
 *  internal definition of tinfo structure - just a pointer to the malloc'ed
 *  buffer for now.
 */
struct tinfo {
        char *info;
        char *up;                       /* for use by tgoto */
        char *bc;                       /* for use by tgoto */
        struct tbuf {                   /* for use by t_agetstr() */
                struct tbuf *next;      /* pointer to next area */
                char *data;             /* pointer to beginning of buffer */
                char *ptr;              /* current data pointer */
                char *eptr;             /* pointer to the end of buffer */
        } *tbuf;
};

#define TMSPC10SIZE     15

extern const short      __tmspc10[TMSPC10SIZE];

/*__BEGIN_DECLS*/
/*
 *  localize cgetxxx() implementations/
 *	Under BSD derived environments (including OS/X) these are available within stdlib.h
 */
#define cgetset         termlib_cgetset
#define cgetcap         termlib_cgetcap
#define cgetent         termlib_cgetent
#define cgetmatch       termlib_cgetmatch
#define cgetclose       termlib_cgetclose
#define cgetstr         termlib_cgetstr
#define cgetustr        termlib_cgetustr
#define cgetnum         termlib_cgetnum

int                     cgetset(const char *ent);
char *                  cgetcap(char *buf, const char *cap, int type);
int                     cgetent(char **buf, const char * const *db_array, const char *name);
int                     cgetmatch(const char *buf, const char *name);
int                     cgetclose(void);
int                     cgetstr(char *buf, const char *cap, char **str);
int                     cgetustr(char *buf, const char *cap, char **str);
int                     cgetnum(char *buf, const char *cap, long *num);

int                     t_putws(struct tinfo *info, const wchar_t *cp, int affcnt, void (*outc)(wchar_t, void *), void *args);

/*__END_DECLS*/

#ifndef strlcpy         /*__APPLE__*/
#define strlcpy(a,b,c)  strncmp(a,b,c)
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define snprintf _snprintf
#endif

#undef  _DIAGASSERT
#define _DIAGASSERT(x)  assert(x)

/*end*/


