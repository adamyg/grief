#ifndef GR_KEYWD_H_INCLUDED
#define GR_KEYWD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_keywd_h,"$Id: keywd.h,v 1.13 2014/10/22 02:32:59 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: keywd.h,v 1.13 2014/10/22 02:32:59 ayoung Exp $
 * Keyword table.
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
#include <edmacros.h>

__CBEGIN_DECLS

extern void                 builtin_init(void);
extern struct BUILTIN *     builtin_lookup(char const *str);
extern int                  builtin_index(char const *str);

extern const unsigned       builtin_count;
extern uint32_t             builtin_signature;
extern struct BUILTIN       builtin[];

extern const int            cm_version;

__CEND_DECLS

#endif /*GR_KEYWD_H_INCLUDED*/
