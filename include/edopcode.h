#ifndef GR_EDOPCODE_H_INCLUDED
#define GR_EDOPCODE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edopcode_h,"$Id: edopcode.h,v 1.17 2015/02/19 00:16:56 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edopcode.h,v 1.17 2015/02/19 00:16:56 ayoung Exp $
 * List types.
 *
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
 *  The following are the atom types we recognize and generate in the cm files.
 */
typedef enum opcodes {
    F_ERROR     = -1,                   /* Non-existant case */
    F_HALT      = 0,                    /* End of List */
    F_INT       = 1,                    /* 32-bit integer */
    F_STR       = 2,                    /* Unquoted string */
    F_LIST      = 3,                    /* List */
    F_NULL      = 4,                    /* Used as destination of loops */
    F_ID        = 5,                    /* 16-bit keyword */
    F_END       = 6,                    /* End of list */
    F_POLY      = 7,                    /* Symbol is polymorphic */
    F_LIT       = 8,                    /* Pointer to literal string */
    F_RSTR      = 9,                    /* Pointer to reference string */
    F_FLOAT     = 10,                   /* Floating point number */
    F_RLIST     = 11,                   /* A Reference list */
    F_MAX       = 12,                   /* Upper limit (used for assert tests) */

    F_INT_DYNAMIC = 98,                 /* Dynamic 'F_INT' value */
    F_REFERENCE = 99                    /* Reference to another symbol */
} OPCODE;

#endif /*GR_EDOPCODE_H_INCLUDED*/
