#ifndef LIBW32_GRP_H_INCLUDED
#define LIBW32_GRP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_grp_h,"$Id: grp.h,v 1.8 2019/03/15 23:12:09 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1998 - 2019, Adam Young.
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

LIBW32_API struct group *getgrent(void);
LIBW32_API struct group *getgrgid(int);
LIBW32_API struct group *getgrnam(const char *);
LIBW32_API void         setgrent(void);
LIBW32_API void         endgrent(void);

__END_DECLS

#endif /*LIBW32_GRP_H_INCLUDED*/
