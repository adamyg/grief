#ifndef GR_M_SYSINFO_H_INCLUDED
#define GR_M_SYSINFO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_sysinfo_h,"$Id: m_sysinfo.h,v 1.3 2014/10/22 02:33:09 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_sysinfo.h,v 1.3 2014/10/22 02:33:09 ayoung Exp $
 * System information primitives.
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

extern void                 do_uname(void);
extern void                 inq_home(void);
extern void                 inq_tmpdir(void);
extern void                 inq_hostname(void);
extern void                 inq_username(void);

__CEND_DECLS

#endif /*GR_M_SYSINFO_H_INCLUDED*/
