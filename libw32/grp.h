#ifndef GR_GRP_H_INCLUDED
#define GR_GRP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_grp_h,"$Id: grp.h,v 1.5 2015/02/19 00:17:25 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1998 - 2015, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

struct group {
    const char *        gr_name;
    const char *        gr_passwd;
    int                 gr_gid;
    const char **       gr_mem;
};

struct group *          getgrent(void);
struct group *          getgrgid(int);
struct group *          getgrnam(const char *);
void                    setgrent(void);
void                    endgrent(void);

__END_DECLS

#endif /*GR_GRP_H_INCLUDED*/
