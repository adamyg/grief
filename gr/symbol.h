#ifndef GR_SYMBOL_H_INCLUDED
#define GR_SYMBOL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_symbol_h,"$Id: symbol.h,v 1.24 2020/04/21 00:01:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: symbol.h,v 1.24 2020/04/21 00:01:57 cvsuser Exp $
 * Symbol management.
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

#define SYM_FUNCNAME_LEN    128                     /* MAGIC - system limit */

extern void                 sym_init(void);
extern void                 sym_globals(void);
extern void                 sym_errno_constants(void);
extern void                 sym_shutdown(void);

extern void                 sym_move(SPTREE *sym_tbl, const char *why);
extern void                 sym_create(SYMBOL *sp, const char *str, int type);
extern SYMBOL *             sym_push(int global, const char *name, int type, uint16_t flags);
extern void                 sym_destroy(SYMBOL *sp);
extern void                 sym_free(SYMBOL *sp);
extern SPBLK *              sym_alloc(const char *str, int type);
extern void                 sym_attach(BUFFER_t *bp);
extern void                 sym_detach(BUFFER_t *bp);
extern void                 sym_local_build(void);
extern void                 sym_local_delete(int outer);
extern void                 sym_macro_delete(SPTREE *sym_tbl);
extern void                 sym_table_delete(SPTREE *sym_tbl);

extern void                 sym_rassociate(int idx, SYMBOL *sp);
extern SYMBOL *             sym_rlookup(int idx);

extern SYMBOL *             sym_elookup(const char *name);
extern SYMBOL *             sym_lookup(const char *name);
extern SYMBOL *             sym_access(SYMBOL *sp);
extern SYMBOL *             sym_global_lookup(const char *name);
extern SYMBOL *             sym_local_lookup(const char *name);
extern void                 sym_dump(void);

extern int                  sym_isconstant(SYMBOL *sp, const char *msg);
extern void                 sym_assign_list(SYMBOL *sp, const LIST *list);
extern void                 sym_donate_list(SYMBOL *sp, LIST *lp, int llen);
extern void                 sym_assign_ref(SYMBOL *sp, ref_t *rp);
extern void                 sym_donate_ref(SYMBOL *sp, ref_t *rp);
extern void                 sym_assign_str(SYMBOL *sp, const char *str);
extern void                 sym_assign_nstr(SYMBOL *sp, const char *str, int len);
extern void                 sym_assign_int(SYMBOL *sp, accint_t value);
extern void                 sym_assign_float(SYMBOL *sp, accfloat_t value);

extern void                 argv_assign_list(int argi, const LIST *list);
extern void                 argv_donate_list(int argi, LIST *lp, int llen);
extern int                  argv_assign_str(int argi, const char *val);
extern void                 argv_assign_nstr(int argi, const char *val, int len);
extern void                 argv_assign_int(int argi, accint_t val);
extern void                 argv_assign_float(int argi, accfloat_t val);

extern int                  system_call(int ret);
extern void                 system_errno(int ret);

extern ref_t *              x_halt_list;
extern accint_t *           x_errno_ptr;
extern int                  x_nest_level;

extern SPTREE *             x_gsym_tbl;
extern SPTREE *             x_lsym_tbl[];

__CEND_DECLS

#endif /*GR_SYMBOL_H_INCLUDED*/
