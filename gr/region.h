#ifndef GR_REGION_H_INCLUDED
#define GR_REGION_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_region_h,"$Id: region.h,v 1.17 2014/10/22 02:33:17 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: region.h,v 1.17 2014/10/22 02:33:17 ayoung Exp $
 * Buffer regions.
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
#include "anchor.h"

__CBEGIN_DECLS

/*
 *  region buffer management
 */
typedef struct _region {
    int     r_type;         /* Mark type. */
    LINENO  r_startline;    /* Origin. */
    LINENO  r_endline;
    LINENO  r_startcol;
    LINENO  r_endcol;
    LINENO  r_savedcol;
    FSIZE_t r_size;         /* Length in characters. */
    LINENO  r_loffset;
    LINENO  r_cwidth;
} REGION_t;

extern int                  region_get(const int del, const int undo, const ANCHOR_t *source, REGION_t *region);
extern int                  region_delete(const REGION_t *region);
extern int                  region_write(FILE *fp, const char *str, const REGION_t *region);
extern int                  region_copy(const char *str, const REGION_t *region);

extern void                 do_delete_block(void);
extern void                 do_transfer(void);
extern void                 inq_mark_size(void);
extern void                 do_paste(void);
extern void                 do_copy(void);
extern void                 do_cut(void);
extern void                 do_get_region(void);
extern void                 do_inside_region(void);

__CEND_DECLS

#endif /*GR_REGION_H_INCLUDED*/
