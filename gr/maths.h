#ifndef GR_MATH_H_INCLUDED
#define GR_MATH_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_math_h,"$Id: maths.h,v 1.13 2020/04/21 00:01:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: maths.h,v 1.13 2020/04/21 00:01:57 cvsuser Exp $
 * Basic math primitives.
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

extern int                  com_equ(int op, SYMBOL *sp, const LISTV *lvp);

extern void                 do_cast(int type);
extern void                 do_com_equ(int op);
extern void                 do_com_op(int op);
extern void                 do_minusminus(void);
extern void                 do_plusplus(void);
extern void                 do_post_minusminus(void);
extern void                 do_post_plusplus(void);
extern void                 do_lnot(void);
extern void                 do_andand(void);
extern void                 do_oror(void);
extern void                 do_abs(void);

__CEND_DECLS

#endif /*GR_MATH_H_INCLUDED*/
