#ifndef GR_TAGSE_H_INCLUDED
#define GR_TAGSE_H_INCLUDED
__CIDENT_RCSID(gr_tagse_h,"$Id: tagse.h,v 1.8 2014/10/22 02:33:22 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */

/*
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

#include <edsym.h>

__CBEGIN_DECLS

#ifndef TAG_FPARTIALMATCH
#define TAG_FPARTIALMATCH   0x01
#define TAG_FIGNORECASE     0x02
#endif

typedef struct {
    const char *name;           /* name of tag */

    const char *def;            /* full definition */

    const char *file;           /* path of source file containing definition of tag */

    const char *pattern;        /* pattern for locating source line
                                 * (may be NULL if not present) */

    unsigned long line;         /* line number in source file of tag definition
                                 * (may be zero if not known) */
} etagEntry;

extern void *               etagsOpen(const char *const filePath);
extern int                  etagsSetSortType(void *const file, const int sorted);
extern int                  etagsFirst(void * const vfile, etagEntry *const entry);
extern int                  etagsNext(void * const vfile, etagEntry *const entry);
extern int                  etagsFind(void *const file, etagEntry *const entry, const char *const name, const int options);
extern int                  etagsFindNext(void *const file, etagEntry *const entry);
extern int                  etagsClose(void *const file);

__CEND_DECLS

#endif /*GR_TAGSE_H_INCLUDED*/
