#ifndef GR_M_MCHAR_H_INCLUDED
#define GR_M_MCHAR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_mchar_h,"$Id: m_mchar.h,v 1.9 2021/06/13 16:01:34 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_mchar.h,v 1.9 2021/06/13 16:01:34 cvsuser Exp $
 * Multiple-byte/local primitives.
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

extern void                 do_set_encoding(void);
extern void                 inq_encoding(void);
extern void                 inq_encodings(void);

extern void                 do_wcwidth(void);
extern void                 do_set_unicode_version(void);
extern void                 inq_unicode_version(void);

__CEND_DECLS

#endif /*GR_M_MCHAR_H_INCLUDED*/
