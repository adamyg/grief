#ifndef LIBW32_SYS_UTSNAME_H_INCLUDED
#define LIBW32_SYS_UTSNAME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_utsname_h,"$Id: utsname.h,v 1.8 2018/09/29 02:25:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1998 - 2018, Adam Young.
 * All rights reserved.
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

#define _UTSNAME_LENGTH 64

struct utsname {
    char sysname[_UTSNAME_LENGTH];
    char nodename[_UTSNAME_LENGTH];
    char release[_UTSNAME_LENGTH];
    char version[_UTSNAME_LENGTH];
    char machine[_UTSNAME_LENGTH];
};

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS
LIBW32_API int          uname(struct utsname *buf);
__END_DECLS

#endif  /*LIBW32_SYS_UTSNAME_H_INCLUDED*/

