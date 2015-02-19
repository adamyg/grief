#ifndef GR_M_GETOPT_H_INCLUDED
#define GR_M_GETOPT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_getopt_h,"$Id: m_getopt.h,v 1.6 2014/10/22 02:33:03 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_getopt.h,v 1.6 2014/10/22 02:33:03 ayoung Exp $
 * Command line optional support.
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

extern void                 getopt_shutdown(void);
extern void                 getsubopt_shutdown(void);

extern void                 do_getopt(void);
extern void                 do_getsubopt(void);
extern void                 do_split_arguments(void);

__CEND_DECLS

#endif /*GR_M_GETOPT_H_INCLUDED*/
