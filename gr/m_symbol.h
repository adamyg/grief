#ifndef GR_M_SYMBOL_H_INCLUDED
#define GR_M_SYMBOL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_symbol_h,"$Id: m_symbol.h,v 1.5 2020/04/21 00:01:56 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_symbol.h,v 1.5 2020/04/21 00:01:56 cvsuser Exp $
 * Symbol primitives.
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

extern void                 do_arg_list(void);
extern void                 do_const(void);
extern void                 do_declare(int flag);
extern void                 do_extern(void);
extern void                 do_get_parm(void);
extern void                 do_global(void);
extern void                 do_make_local_variable(void);
extern void                 do_put_parm(void);
extern void                 do_ref_parm(void);
extern void                 do_register(void);
extern void                 do_static(void);

extern void                 inq_symbol(void);

__CEND_DECLS

#endif /*GR_M_SYMBOL_H_INCLUDED*/
