#ifndef GR_DIFF_H_INCLUDED
#define GR_DIFF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_diff_h,"$Id: diff.h,v 1.9 2014/10/22 02:32:57 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: diff.h,v 1.9 2014/10/22 02:32:57 ayoung Exp $
 * Differ interface.
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

/*--export--defines--*/
/*
 *  Differ control options
 */
#define DIFF_IGNORE_WHITESPACE      0x0001
#define DIFF_IGNORE_CASE            0x0002

#define DIFF_COMPRESS_WHITESPACE    0x0010
#define DIFF_SUPPRESS_LEADING       0x0020
#define DIFF_SUPPRESS_TRAILING      0x0040
#define DIFF_SUPPRESS_LFCR          0x0080
/*--end--*/

extern void                 do_diff_buffers(void);
extern void                 do_diff_lines(void);
extern void                 do_diff_strings(void);

extern uint32_t             diff_hash(const void *buf, uint32_t length, uint32_t flags);

__CEND_DECLS

#endif /*GR_DIFF_H_INCLUDED*/
