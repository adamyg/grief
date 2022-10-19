#ifndef GR_REGEXP_H_INCLUDED
#define GR_REGEXP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_regexp_h,"$Id: regexp.h,v 1.18 2022/08/10 15:44:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regexp.h,v 1.18 2022/08/10 15:44:57 cvsuser Exp $
 * Internal regular expression engine.
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

#undef DEBUG_REGEXP
#include <edsym.h>

struct regopts {
    int                 flags;                  /* Original flags. */
    int                 mode;                   /* Regular expression mode, RE_XXX. */
    int                 case_sense;             /* Case sensitive search, when true otherwise case-fold. */
    int                 regexp_chars;           /* Regular expression characters. */
        /*
         *      1   Match patterns from left to right, with minimal closure.
         *      2   Match patterns in same direction as search direction, with minimal closure.
         *      3   Match patterns from right toleft, with minimal closure.
         *      0   Treat as a normal characters; not regular expression.
         *     -1   Match patterns from left to right, with maximum closure.
         *     -2   Match patterns in same direction as search direction, with maximum closure.
         *     -3   Match patterns from right toleft, with maximum closure.
         */
    int                 fwd_search;             /* Forward search when true otherwise backwards. */
    int                 prompt;                 /* Prompt for replacement text. */
    int                 line;                   /* Line mode. */
};

#define RE_NSUBEXP          10                  /* Sub expression limit. */

typedef struct REGEXP {
    MAGIC_t             magic;                  /* Structure magic */
#define REGEXP_MAGIC        MKMAGIC('R','g','E','p')

    struct regopts      options;                /* Active options. */
    const char *        startp[RE_NSUBEXP];     /* Group start. */
    const char *        endp[RE_NSUBEXP];       /* and associated end. */
    int                 groupno;                /* Group upper range, -1 if none. */
    int                 length;                 /* Length of matched string. */
    const char *        start;                  /* Start of matched arena. */
    const char *        end;                    /* End of matched arena. */
    const char *        setpos;                 /* \\c anchor position. */
    const void *        program;                /* Expression byte-code. */
    void *              _dmem;
    size_t              _dsiz;
} REGEXP;

__CBEGIN_DECLS

extern REGEXP *             regexp_comp(const struct regopts *options, const char *pattern);
extern REGEXP *             regexp_comp2(REGEXP *prog, const struct regopts *options, const char *pattern);
extern int                  regexp_exec(REGEXP *prog, const char *buf, int buflen, int offset);
extern int                  regexp_free(REGEXP *prog);

__CEND_DECLS

#endif /*GR_REGEXP_H_INCLUDED*/
