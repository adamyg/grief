#ifndef GR_KBNAME_H_INCLUDED
#define GR_KBNAME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_kbname_h,"$Id: kbname.h,v 1.2 2024/11/29 13:37:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbname.h,v 1.2 2024/11/29 13:37:22 cvsuser Exp $
 * Key names.
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

extern int                  kbmulti_allocate(void);
extern void                 kbmulti_assign(KEY key, const char *seq);

#define KBNAMELEN 64        /* Alt-Ctrl-Shift-Keypad-Equals=28 */

extern const char *         kbname_fromkey(int key, char *buf, unsigned buflen);
extern int                  kbname_tokey(const char *name, int *lenp);

__CEND_DECLS

#endif /*GR_KBNAME_H_INCLUDED*/
