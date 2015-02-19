#ifndef GR_MACROLIB_H_INCLUDED
#define GR_MACROLIB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_macrolib_h,"$Id: macrolib.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: macrolib.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $
 * Macro library support.
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

extern void                 macrolib_init(void);
extern void                 macrolib_shutdown(void);
extern int                  macrolib_search(const char *lpath, const char *fname);

__CEND_DECLS

#endif /*GR_MACROLIB_H_INCLUDED*/
