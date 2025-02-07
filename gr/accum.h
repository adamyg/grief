#ifndef GR_ACCUM_H_INCLUDED
#define GR_ACCUM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_accum_h,"$Id: accum.h,v 1.21 2025/02/07 03:03:20 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: accum.h,v 1.21 2025/02/07 03:03:20 cvsuser Exp $
 * Accumulator manipulating.
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

extern void *               acc_expand(size_t len);
extern void                 acc_assign_str(const char *str);
extern void                 acc_assign_nstr(const char *str, size_t len);
extern void                 acc_assign_strlen(size_t len);
extern void                 acc_assign_str2(const char *str1, size_t len1, const char *str2, size_t len2);
extern void                 acc_assign_lit(const char *str);
extern void                 acc_assign_argv(const LISTV *lvp);
extern void                 acc_clear(void);
extern void                 acc_assign_ref(ref_t *rp);
extern void                 acc_assign_list(const LIST *lp, size_t llen);
extern void                 acc_donate_list(LIST *lp, size_t llen);
extern void                 acc_assign_null(void);
extern void                 acc_assign_int(accint_t val);
extern void                 acc_assign_float(accfloat_t val);
extern accfloat_t           acc_get_fval(void);
extern accint_t             acc_get_ival(void);
extern const char *         acc_get_sval(void);
extern ref_t *              acc_get_ref(void);
extern char *               acc_get_sbuf(void);
extern OPCODE               acc_get_type(void);
extern void                 acc_trace(void);

__CEND_DECLS

#endif /*GR_ACCUM_H_INCLUDED*/
