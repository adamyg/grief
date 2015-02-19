#ifndef GR_SPELL_H_INCLUDED
#define GR_SPELL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_spell_h,"$Id: spell.h,v 1.12 2014/10/26 22:13:13 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spell.h,v 1.12 2014/10/26 22:13:13 ayoung Exp $
 * Speller interface.
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

#include <edsym.h>

__CBEGIN_DECLS

typedef struct _spell {
    MAGIC_t                 sf_magic;
    const char *            sf_description;
    int                   (*sf_language)(struct _spell *spell, const char *name, int enable);
    const char **         (*sf_dictionaries)(struct _spell *spell, int all);
    int                   (*sf_check)(struct _spell *spell, int flags, const char *word, int length);
    const char **         (*sf_suggest)(struct _spell *spell, int flags, const char *word, int length);
    int                   (*sf_add)(struct _spell *spell, int flags, const char *word, const char *affix);
    int                   (*sf_remove)(struct _spell *spell, int flags, const char *word);
    void                  (*sf_close)(struct _spell *spell);
} Spell_t;

extern void                 spell_init(void);
extern void                 spell_close(void);
extern int                  spell_check(const char *word, int len);
extern int                  spell_dblookup(const char *word);
extern const char *         spell_nextword(BUFFER_t *bp, const char *buffer, const int length, 
                                    int *wordlen, int *offset, int *chars, int *column);

extern Spell_t *            spell_enchant_init(const char **langs);
extern Spell_t *            spell_hunspell_init(const char **langs, const char **bdictionaries);
extern Spell_t *            spell_aspell_init(const char **langs);

__CEND_DECLS

#endif /*GR_SPELL_H_INCLUDED*/
