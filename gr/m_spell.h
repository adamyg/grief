#ifndef GR_M_SPELL_H_INCLUDED
#define GR_M_SPELL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_spell_h,"$Id: m_spell.h,v 1.7 2014/10/22 02:33:08 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_spell.h,v 1.7 2014/10/22 02:33:08 ayoung Exp $
 * Speller primitives.
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

/*--export--defines--*/
/*
 *  Spell control options
 */
#define SPELL_ADD           0x0001
#define SPELL_IGNORE        0x0002
#define SPELL_REPLACE       0x0003

#define SPELL_SAVE          0x0100
#define SPELL_LOAD          0x0101

#define SPELL_LANG_ADD      0x0200
#define SPELL_LANG_PRIMARY  0x0201
#define SPELL_LANG_REMOVE   0x0202

#define SPELL_DESCRIPTION   0x0300
#define SPELL_DICTIONARIES  0x0301
/*--end--*/

extern void                 do_spell_check(int mode);
extern void                 do_spell_suggest(void);
extern void                 do_spell_control(void);
extern void                 do_spell_dictionary(void);
extern void                 do_spell_distance(void);

__CEND_DECLS

#endif /*GR_M_SPELL_H_INCLUDED*/
