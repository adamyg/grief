#ifndef GR_LISP_H_INCLUDED
#define GR_LISP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_lisp_h,"$Id: lisp.h,v 1.22 2025/02/07 03:03:21 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: lisp.h,v 1.22 2025/02/07 03:03:21 cvsuser Exp $
 * List implementation and primitives.
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

#define LIST_SIZEOF(a)      (((a) * 9) + 1)     /* FIXME */

extern int                  atom_size(const LIST *lp);
extern LIST *               atom_push_sym(LIST *lp, const char *svalue);
extern LIST *               atom_push_str(LIST *lp, const char *svalue);
extern LIST *               atom_push_nstr(LIST *lp, const char *svalue, size_t length);
extern LIST *               atom_push_const(LIST *lp, const char *svalue);
extern LIST *               atom_push_int(LIST *lp, accint_t value);
extern LIST *               atom_push_float(LIST *lp, accfloat_t value);
extern LIST *               atom_push_ref(LIST *lp, ref_t *ref);
extern LIST *               atom_push_null(LIST *lp);
extern LIST *               atom_push_halt(LIST *lp);

extern const LIST *         atom_next(const LIST *lp);
extern const LIST *         atom_next_nonnull(const LIST *lp);
extern int                  atom_number(const LIST *lp);
extern const LIST *         atom_nth(const LIST *lp, int idx);
extern int                  atom_xnull(const LIST *lp);
extern int                  atom_xint(const LIST *lp, accint_t *val);
extern int                  atom_xfloat(const LIST *lp, accfloat_t *val);
extern const char *         atom_xstr(const LIST *lp);
extern const LIST *         atom_xlist(const LIST *lp);

extern int                  atom_assign_acc(const LIST *lp);
extern int                  atom_assign_sym(const LIST *lp, SYMBOL *sp);

extern int                  argv_size(const LISTV *lvp);
extern int                  argv_copy(LIST *lp, const LISTV *lvp);
extern size_t               argv_make(LISTV *lvp, const LIST *lp);
extern LIST *               argv_list(const LISTV *lvp, int atoms, size_t *llenp);

extern LIST *               lst_alloc(size_t len, int atoms);
extern LIST *               lst_size(LIST *lp, size_t newlen, int newatoms);
extern LIST *               lst_expand(LIST *lp, size_t llen);
extern LIST *               lst_extend(LIST *lp, int leninc, int atomsinc);
extern int                  lst_isnull(const LIST *lp);
extern int                  lst_atoms_get(const LIST *lp);
extern size_t               lst_check(const LIST *lp);
extern size_t               lst_sizeof(const LIST* lp);
extern size_t               lst_length(const LIST *lp);
extern LIST *               lst_join(const LIST *lp, size_t llen, const LISTV *lvp, size_t *llenp);
extern void                 lst_free(LIST *lp);
extern LIST *               lst_clone(const LIST *lp, size_t *llenp);
extern LIST *               lst_build(const LIST *lp, size_t llen);

extern ref_t *              rlst_create(LIST *lp, int llen);
extern ref_t *              rlst_clone(const LIST *lp);
extern ref_t *              rlst_build(const LIST *lp, size_t llen);
extern ref_t *              rlst_splice(ref_t *rp, int nth, int datoms, const LISTV *lvp, int natoms, int flat);

extern void                 do_make_list(void);
extern void                 do_nth(void);
extern void                 do_get_nth(void);
extern void                 do_car(void);
extern void                 do_cdr(void);
extern void                 do_quote_list(void);
extern void                 do_length_of_list(void);
extern void                 do_is_type(int type);
extern void                 do_typeof(void);
extern void                 do_put_nth(void);
extern void                 do_delete_nth(void);
extern void                 do_splice(void);
extern void                 do_shift(void);
extern void                 do_unshift(void);
extern void                 do_push(void);
extern void                 do_pop(void);
extern void                 do_list_each(void);
extern void                 do_list_extract(void);
extern void                 do_list_reset(void);

__CEND_DECLS

#endif /*GR_LISP_H_INCLUDED*/
