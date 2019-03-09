#ifndef GR_ASCIIDEFS_H_INCLUDED
#define GR_ASCIIDEFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_asciidefs_h,"$Id: asciidefs.h,v 1.9 2018/10/04 01:27:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: asciidefs.h,v 1.9 2018/10/04 01:27:59 cvsuser Exp $
 * ASCII character value definitions
 * For portibality avoid where possible use of c/+cc escapes (ie. '\n').
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#define ASCIIDEF_BEL        0x07                /* bell/alert */
#define ASCIIDEF_BS         0x08                /* backspace */
#define ASCIIDEF_HT         0x09                /* horizontal tab */
#define ASCIIDEF_LF         0x0a                /* line-feed/new-line */
#define ASCIIDEF_VT         0x0b                /* vertical tab */
#define ASCIIDEF_FF         0x0c                /* form-feed/new-page*/
#define ASCIIDEF_CR         0x0d                /* carriage-return */
#define ASCIIDEF_SUB        0x1a                /* substitute/ctrlz */
#define ASCIIDEF_ESC        0x1b                /* escape */
#define ASCIIDEF_DEL        0x7f                /* delete */
#define ASCIIDEF_NEL        0x85                /* next-line */

#endif /*GR_ASCIIDEFS_H_INCLUDED*/
