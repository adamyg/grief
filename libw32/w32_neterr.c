#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_neterr_c,"$Id: w32_neterr.c,v 1.6 2015/02/19 00:17:30 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 network errno mapping support
 *
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include <unistd.h>
#include <time.h>


int
w32_errno_net(void)
{
    int t_errno = WSAGetLastError();            /* last network error */

    switch (t_errno) {
    /*
     *  must map a few errno's as the errno namespaces are not clean.
     */
    case WSAEINTR: t_errno = EINTR; break;
    case WSAEBADF: t_errno = EBADF; break;
    case WSAEACCES: t_errno = EACCES; break;
    case WSAEFAULT: t_errno = EFAULT; break;
    case WSAEINVAL: t_errno = EINVAL; break;
    case WSAEMFILE: t_errno = EMFILE; break;
    case WSAENAMETOOLONG: t_errno = ENAMETOOLONG; break;
    case WSAENOTEMPTY: t_errno = ENOTEMPTY; break;
    default:
        break;
    }
    errno = t_errno;
    return t_errno;
}
