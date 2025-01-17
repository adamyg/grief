#ifndef GR_MACROS_H_INCLUDED
#define GR_MACROS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_macros_h,"$Id: macros.h,v 1.21 2025/01/17 12:38:29 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: macros.h,v 1.21 2025/01/17 12:38:29 cvsuser Exp $
 * Macro definitions management.
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

extern void                 macro_init(void);
extern void                 macro_shutdown(void);

extern const char *         module_identifier(void);
extern const char *         module_namespace(const char *module);
extern SPTREE *             module_symbols(const char *module);

extern MACRO *              macro_find(const char *name, const char *module);
extern int                  macro_exist(const char *name, const char *msg);
extern MACRO *              macro_lookup(const char *name);
extern MACRO *              macro_lookup2(const char *name, const char *module);
extern SPTREE *             macro_symbols(const char *name);
extern const char *         macro_resolve(const char *name);
extern const char **        macro_list(unsigned *count);
extern int                  macro_define(const char *name, int flags, const LIST *list);
extern int                  macro_delete(const LIST *list);
extern int                  macro_startup(const char *name);
extern int                  macro_load(const char *fname);
extern int                  macro_autoload(MACRO *mptr, int resolve);
extern int                  macro_loaded(const char * fname);

extern void                 do_autoload(void);
extern void                 do_bless(void);
extern void                 do_delete_macro(void);
extern void                 do_find_macro(void);
extern void                 do_load_macro(void);
extern void                 do_macro(int replacement);
extern void                 do_module(void);
extern void                 do_require(void);
extern void                 inq_macro(void);
extern void                 inq_module(void);

__CEND_DECLS

#endif /*GR_MACROS_H_INCLUDED*/
