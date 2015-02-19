#ifndef GR_BSD_WORDEXP_H_INCLUDED
#define GR_BSD_WORDEXP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_wordexp_h,"$Id: bsd_wordexp.h,v 1.4 2014/10/27 23:27:59 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*-
 * Copyright (c) 2002 Tim J. Robbins.
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
 */

#include <edtypes.h>

typedef struct {
        size_t              we_wordc;   /* count of words matched */
        const char * const *we_wordv;   /* pointer to list of words */
        size_t              we_offs;    /* slots to reserve in we_wordv */
                                        /* following are internals */
        const char *        we_strings; /* storage for wordv strings */
        size_t              we_nbytes;  /* size of we_strings */
} bsd_wordexp_t;

/*
 * Flags for wordexp().
 */
#define WRDE_APPEND     0x1             /* append to previously generated */
#define WRDE_DOOFFS     0x2             /* we_offs member is valid */
#define WRDE_NOCMD      0x4             /* disallow command substitution */
#define WRDE_REUSE      0x8             /* reuse wordexp_t */
#define WRDE_SHOWERR    0x10            /* don't redirect stderr to /dev/null */
#define WRDE_UNDEF      0x20            /* disallow undefined shell vars */

/*
 * Return values from wordexp().
 */
#define WRDE_BADCHAR    1               /* unquoted special character */
#define WRDE_BADVAL     2               /* undefined variable */
#define WRDE_CMDSUB     3               /* command substitution not allowed */
#define WRDE_NOSPACE    4               /* no memory for result */
#if (_XOPEN_SOURCE - 0) >= 4 || defined(_NETBSD_SOURCE)
#define WRDE_NOSYS      5               /* obsolete, reserved */
#endif
#define WRDE_SYNTAX     6               /* shell syntax error */
#define WRDE_ERRNO      7               /* other errors see errno */

__BEGIN_DECLS
int     bsd_wordexp(const char * __restrict, bsd_wordexp_t * __restrict, int);
void    bsd_wordfree(bsd_wordexp_t *);
__END_DECLS

#endif /*GR_BSD_WORDEXP_H_INCLUDED*/
