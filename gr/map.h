#ifndef GR_MAP_H_INCLUDED
#define GR_MAP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_map_h,"$Id: map.h,v 1.14 2014/10/22 02:33:13 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: map.h,v 1.14 2014/10/22 02:33:13 ayoung Exp $
 * Character/line mapping ulitlies.
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

#define vm_lock_line(__ln)  linep(__ln)
#define vm_lock_linex(__bp,__ln) \
                            linepx(__bp, __ln)
#define vm_unlock(__ln)
#define vm_unlockx(__bp,__ln)

#define vm_lock_head(__bp)  lhead(__bp)
#define vm_lock_tail(__bp)  ltail(__bp)
#define vm_lock_next(__lp,__ln) \
                            lforw(__lp)
#define vm_lock_prev(__lp,__ln) \
                            lback(__lp)

enum {
    LOFFSET_LASTBYTE        =-2,                /* last byte, including ESC/combined */
    LOFFSET_FIRSTBYTE       =-1,                /* first byte */
    LOFFSET_NORMAL          =0,                 /* first normal character roffset */
    LOFFSET_NORMAL_MATCH    =1,                 /* first normal and match cursor on completion */
    LOFFSET_FILL_VSPACE     =2,                 /* fill virtual-space and EOL */
    LOFFSET_FILL_SPACE      =3                  /* fill virtual-space, tabs and EOL */
};

extern int                  character_decode(int pos, const LINECHAR *cp, const LINECHAR *end, int *lengthp, int32_t *chp, int32_t *rawp);

extern LINE_t *             linep(LINENO line);
extern LINE_t *             linepx(BUFFER_t *bp, LINENO line);
extern void                 linep_flush(BUFFER_t *bp);

extern int                  line_sizeregion(const LINE_t *lp, int col, int dot, int characters, LINENO *lengthp, LINENO *colp);

extern int                  line_current_status(int *value, int count);

extern int                  line_current_offset(int fill);
extern int                  line_offset(const int line, const int col, int fill);
extern int                  line_offset2(LINE_t *lp, const int line, const int col, int fill);

extern int                  line_current_column(int offset);
extern int                  line_column_eol(int line);
extern int                  line_column(const int line, int offset);
extern int                  line_column2(const LINE_t *lp, const int line, int offset);

extern void                 line_tab_backfill(void);

__CEND_DECLS

#endif /*GR_MAP_H_INCLUDED*/
