#ifndef GR_EDCM_H_INCLUDED
#define GR_EDCM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edcm_h,"$Id: edcm.h,v 1.22 2022/03/21 14:55:27 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edcm.h,v 1.22 2022/03/21 14:55:27 cvsuser Exp $
 * clisp macro constructs.
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

#include <chkalloc.h>                           /* Memory allocation interface. */
#include <libsplay.h>                           /* splay binary trees. */
#include <libmisc.h>                            /* Miscellaneous functions. */

#include <edmacros.h>                           /* Macro constants. */
#include <edrefobj.h>                           /* Reference object utilities. */

#define CM_VERSION      31                      /* Interface version 3.1 */
    /* 3.1 - 1/04/2020, register variables */

#define CM_MAGIC        ((')' << 8) | ')')      /* lower/upper bytes must be same. */
#define CM_EXTENSION    ".cm"                   /* Object extension. */

#define CM_ATOMS_MAX    32000                   /* Maximum number of atoms in a macro definition. */
#define CM_STRINGS_MAX  2048                    /* Maximum strings in macro file. */
#define CM_GLOBALS_MAX  256                     /* Maximum number of global statements in program. */

#include <edpack1.h>
typedef struct __CPACKED_PRE__ CM {
    uint16_t            cm_magic;               /* Magic file number. */
    uint16_t            cm_version;             /* Version number. */
    uint32_t            cm_builtin;             /* Number of builtin macros. */
    uint32_t            cm_num_macros;          /* Number of macro sections. */
    uint32_t            cm_num_atoms;           /* Number of atoms in all macros. */
    uint32_t            cm_globals;             /* Offset to start of globals table (defunct). */
    uint32_t            cm_num_globals;         /* Size of globals array (defunct). */
    uint32_t            cm_num_strings;         /* Number of strings. */
    uint32_t            cm_signature;           /* Builtin signature (crc32). */

/*
 *  Followed by:
 *
 *  uint32_t            cm_moffsets[cm_num_macros];
 *  uint32_t            cm_string_offset;
 *      <MACROS>
 *      <STRINGS>
 *
 */
} __CPACKED_POST__ CM_t;
#define CM_STRUCT_SIZE  ((2 * 2) + (7 * 4))
#include <edpack0.h>

#endif  /*GR_EDCM_H_INCLUDED*/
