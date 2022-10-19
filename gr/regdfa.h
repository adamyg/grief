#ifndef GR_REGDFA_H_INCLUDED
#define GR_REGDFA_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_regdfa_h,"$Id: regdfa.h,v 1.11 2022/07/10 13:09:43 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regdfa.h,v 1.11 2022/07/10 13:09:43 cvsuser Exp $
 * DFA based regular expression engine.
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

#include <stdio.h>
#include <edsym.h>

__CBEGIN_DECLS

struct regdfa {             /* DFA construction */
    MAGIC_t                 magic;              /* Structure magic */
    struct SyntaxTable *    owner;              /* Owning table */
    struct s_dfastate *     start;              /* Initial state */
    unsigned                slots;              /* Total table slots */
    unsigned                cursor;             /* Cursor into table */
    struct s_dfastate **    table;              /* Lookup indirection table */
    void *                  heap;               /* working storage, for life of expression */
    unsigned                flags;
#define REGDFA_ICASE            0x0001          /* ignore case */
};

extern int                  regdfa_check(const char *pattern);

extern struct regdfa *      regdfa_create(const char **patterns, int num_patterns, unsigned flags);
extern struct regdfa *      regdfa_patterns(const char *patterns, const char *end, unsigned flags);
extern void                 regdfa_destroy(struct regdfa *re);

extern void                 regdfa_export(struct regdfa *regex, FILE *fd);

extern struct regdfa *      regdfa_import(FILE *fd);

extern int                  regdfa_match(struct regdfa *re, const char *str, const char *end, int sol, 
                                    const char **startp, const char **endp);

extern int                  regdfa_pmatch(struct regdfa *re, const char *str, int sol, 
                                    const char **startp, const char **endp);

extern struct regdfa *      regdfa_load(const char *filename);
extern int                  regdfa_save(struct regdfa *re, const char *filename);

__CEND_DECLS

#endif /*GR_REGDFA_H_INCLUDED*/
