#ifndef GR_REGPROG_H_INCLUDED
#define GR_REGPROG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_regprog_h,"$Id: regprog.h,v 1.5 2014/10/22 02:33:17 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regprog.h,v 1.5 2014/10/22 02:33:17 ayoung Exp $
 * Regular expression program interface.
 *
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

#define HAVE_LIBTRE                             /* optional/experimental */

#define  ONIG_ESCAPE_REGEX_T_COLLISION
#include <oniguruma.h>
#if defined(HAVE_LIBTRE)
#include <tre.h>
#endif
#include "regexp.h"                             /* local implementation */
#include "regrpl.h"

struct regprog {
    char *              working;                /* Working buffer (during capture operations). */

    const char *        buf;                    /* Search buffer start address. */
    const char *        bufend;                 /* and associated end address. */
    const char *        setpos;                 /* Anchor position (BRIEF/UNIX only). */
    const char *        start;                  /* Start of captured region. */
    const char *        end;                    /* End of captured region. */
    int                 groupno;                /* Number of captured regions. */

    enum {REGPROG_NONE = 0, REGPROG_REGEXP = 1, REGPROG_ONIG, REGPROG_TRE}
                        engine;                 /* Engine type. */
    int                 hasgrps;                /* Pattern contains capture groups. */

    union {
        REGEXP          regexp;
        struct re_onig {
            OnigRegex   regex;                  /* Expression. */
            OnigRegion *region;                 /* Results. */
        } onig;
#if defined(HAVE_LIBTRE)
#define TRE_REGIONS     32
        struct re_tre {
            regex_t     regex;                  /* Expression. */
            regmatch_t  regions[TRE_REGIONS];   /* Results. */
        } tre;
#endif
    } rx;                                       /* Engine state. */

    int               (*exec)(struct regprog *prog, const char *buf, int buflen, int offset);
    const char *      (*capture)(const struct regprog *prog, int group, const char **end);
    void              (*destroy)(struct regprog *prog);
};

#endif /*GR_REGPROG_H_INCLUDED*/
