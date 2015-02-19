#ifndef GR_TAGS_H_INCLUDED
#define GR_TAGS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_tags_h,"$Id: tags.h,v 1.8 2014/10/22 02:33:22 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tags.h,v 1.8 2014/10/22 02:33:22 ayoung Exp $
 * Tag interface.
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

#define TAG_FPARTIALMATCH   0x01
#define TAG_FIGNORECASE     0x02

extern int                  tags_check(const char *word, int wordlen);
extern void                 tags_shutdown(void);

extern void                 do_tagdb_open(void);
extern void                 do_tagdb_close(void);
extern void                 do_tagdb_search(void);

__CEND_DECLS

#endif /*GR_TAGS_H_INCLUDED*/
