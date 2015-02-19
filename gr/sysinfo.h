#ifndef GR_SYSINFO_H_INCLUDED
#define GR_SYSINFO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_sysinfo_h,"$Id: sysinfo.h,v 1.11 2014/10/22 02:33:21 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sysinfo.h,v 1.11 2014/10/22 02:33:21 ayoung Exp $
 * System information services.
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

extern const char *         sysinfo_homedir(char *buf, int len);
extern const char *         sysinfo_tmpdir(void);
extern const char *         sysinfo_execname(const char *arg0);
extern const char *         sysinfo_username(char *buf, int len);
extern const char *         sysinfo_hostname(char *buf, int len);
extern const char *         sysinfo_domainname(char *buf, int len);

__CEND_DECLS

#endif /*GR_SYSINFO_H_INCLUDED*/
