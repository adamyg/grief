#ifndef GR_EDOPCODE_H_INCLUDED
#define GR_EDOPCODE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edopcode_h,"$Id: edopcode.h,v 1.23 2022/09/13 14:15:35 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edopcode.h,v 1.23 2022/09/13 14:15:35 cvsuser Exp $
 * List types.
 *
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

typedef unsigned char LIST;             /* LIST atoms */

/*
 *  atom types recognized and generated within cm files.
 *
 *  See Also: sizeof_atoms and nameof_atoms, word.c
 */
typedef enum opcodes {
    F_ERROR     = -1,                   /* Non-existant case */
    F_HALT      = 0,                    /* End of List */

    F_INT       = 1,                    /* Integer number */
    F_FLOAT     = 2,                    /* Floating point number */
    F_STR       = 3,                    /* Unquoted string (run-time only) */
    F_LIT       = 4,                    /* Literal string */
    F_LIST      = 5,                    /* List */
    F_ARRAY     = 6,                    /* Array (experimental) */
    F_NULL      = 7,                    /* Used as destination of loops */
    F_RSTR      = 8,                    /* Reference string */
    F_RLIST     = 9,                    /* Reference list */
    F_RARRAY    = 10,                   /* Reference array (experimental) */

    F_ID        = 11,                   /* Keyword; 16-bit */
    F_SYM       = 12,                   /* Symbol */
    F_REG       = 13,                   /* Register */

    F_MAX       = 14,                   /* Opcode's upper limit */
    F_OPDATA    = F_RARRAY,             /* Last data-type opcode */

    /*
     *  Specials
     */
    F_INT_DYNAMIC = 97,                 /* Dynamic 'F_INT' value */
    F_REFERENCE = 98,                   /* Reference to another symbol */
    F_POLY      = 99                    /* Symbol is polymorphic */
} OPCODE;

#endif /*GR_EDOPCODE_H_INCLUDED*/

