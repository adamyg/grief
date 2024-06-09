#ifndef GR_EDSYM_H_INCLUDED
#define GR_EDSYM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edsym_h,"$Id: edsym.h,v 1.26 2024/04/08 15:07:03 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edsym.h,v 1.26 2024/04/08 15:07:03 cvsuser Exp $
 * Symbol management.
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edtypes.h>
#include <edopcode.h>
#include <edrefobj.h>                           /* Reference object utilities */

#define SYMLEN          32

typedef struct define {
    const char *        name;
    const char *        value;
    int                 length;
    struct define *     next;
} DEFINE;


/*
 *  com_op() and com_equ() operators.
 */
enum _aops {
    MOP_NOOP,
    MOP_PLUS,
    MOP_MINUS,
    MOP_MULTIPLY,
    MOP_DIVIDE,
    MOP_MODULO,
    MOP_EQ,
    MOP_NE,
    MOP_LT,
    MOP_LE,
    MOP_GT,
    MOP_GE,
    MOP_ABOVE,
    MOP_ABOVE_EQ,
    MOP_BELOW,
    MOP_BELOW_EQ,
    MOP_BNOT,
    MOP_BAND,
    MOP_BOR,
    MOP_BXOR,
    MOP_LSHIFT,
    MOP_RSHIFT,
    MOP_CMP             /* 12/01/07 <=> operator */
};


/*
 *  Defines a symbol
 */
typedef struct SYMBOL {
    char                s_name[SYMLEN];
    OPCODE              s_type;
    uint8_t             s_flags;
    void              (*s_dynamic)(struct SYMBOL *);
    union {
        accfloat_t      fval;
        accint_t        ival;
        ref_t *         obj;
        struct SYMBOL * sym;
    } s_v;

#define s_float         s_v.fval
#define s_int           s_v.ival
#define s_obj           s_v.obj
#define s_sym           s_v.sym
} SYMBOL;

#define SYMSET(s,f)     ((s)->s_flags |= (f))
#define SYMCLR(s,f)     ((s)->s_flags &= (uint8_t)(~(f)))
#define SYMTST(s,f)     (((s)->s_flags & (f)) ? 1 : 0)

/*
 *  List-macro definitions
 */
typedef struct LISTV {
    OPCODE              l_flags;
    union {
        SYMBOL *        sym;
        accfloat_t      fval;
        accint_t        ival;
        const LIST *    list;
        const char *    sval;
        ref_t *         obj;
    } u;

#define l_float         u.fval
#define l_int           u.ival
#define l_list          u.list
#define l_ref           u.obj
#define l_str           u.sval
#define l_sym           u.sym

} LISTV;

#define lnext(lp)       ((lp)->l_next ? (lp) + (lp)->l_next : 0)
#define lnull(lp)       (lp->l_next == 0)
#define llist(lp)       ((lp)->l_int ? (lp) + (lp)->l_int : 0)

#endif /*GR_EDSYM_H_INCLUDED*/

