#ifndef GR_EVAL_H_INCLUDED
#define GR_EVAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_eval_h,"$Id: eval.h,v 1.20 2020/04/21 00:01:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: eval.h,v 1.20 2020/04/21 00:01:55 cvsuser Exp $
 * Expression evaluation.
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

extern int                  eval(const LIST *lp, LISTV *lpv);
extern int                  eval2(register const LIST *lp);

extern int                  listv_null(const LISTV *lp);
extern int                  listv_int(const LISTV *lp, accint_t *val);
extern int                  listv_float(const LISTV *lp, accfloat_t *val);
extern int                  listv_str(const LISTV *lp, const char **val);
extern int                  listv_list(const LISTV *lp, const LIST **val);
extern const char *         listv_xstr(const LISTV *lvp);

extern int                  isa_undef(int argi);        /* NULL or undefined */
extern int                  isa_null(int argi);         /* optional NULL */
extern int                  isa_integer(int argi);
extern int                  isa_float(int argi);
extern int                  isa_string(int argi);
extern int                  isa_list(int argi);

extern const char *         get_str(int argi);
extern const char *         get_xstr(int argi);
extern int                  get_strlen(int argi);
extern int                  get_xcharacter(int argi);
extern int                  get_integer(int argi);
extern int                  get_xinteger(int argi, int undef);
extern accint_t             get_accint(int argi);
extern accint_t             get_xaccint(int argi, accint_t undef);
extern accfloat_t           get_accfloat(int argi);
extern accfloat_t           get_xaccfloat(int argi, accfloat_t undef);
extern const LIST *         get_list(int argi);
extern int                  get_listlen(int argi);
extern const LIST *         get_xlist(int argi);
extern SYMBOL *             get_symbol(int argi);

extern int                  get_iarg1(const char *str, accint_t *val);
extern const char *         get_xarg(int argi, const char *str, char *buf, int bufsiz);
extern const char *         get_arg1(const char *str, char *buf, int bufsiz);

extern void                 do_cvt_to_object(void);

__CEND_DECLS

#endif /*GR_EVAL_H_INCLUDED*/
