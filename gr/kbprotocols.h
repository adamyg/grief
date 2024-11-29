#ifndef GR_KBPROTOCOLS_H_INCLUDED
#define GR_KBPROTOCOLS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_kbprotocols_h,"$Id: kbprotocols.h,v 1.1 2024/11/18 13:42:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbprotocols.h,v 1.1 2024/11/18 13:42:22 cvsuser Exp $
 * Keyboard input protocols.
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

extern void                 kbprotocols_init(void);
extern int                  kbprotocols_parse(const char *buf, unsigned buflen, int force);

extern int                  key_protocolid(const char *name, int namelen);
extern const char *         key_protocolname(int mode, const char *def);

__CEND_DECLS

#endif /*GR_KBPROTOCOLS_H_INCLUDED*/
