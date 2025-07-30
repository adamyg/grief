#ifndef GR_PRNTF_H_INCLUDED
#define GR_PRNTF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_prntf_h,"$Id: prntf.h,v 1.7 2025/02/07 03:03:21 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: prntf.h,v 1.7 2025/02/07 03:03:21 cvsuser Exp $
 * Print formatter.
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

extern const char *         print_formatted(int offset, size_t *len, size_t *width);

__CEND_DECLS

#endif /*GR_PRNTF_H_INCLUDED*/
