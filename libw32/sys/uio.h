#ifndef LIBW32_SYS_UIO_H_INCLUDED
#define LIBW32_SYS_UIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_uio_h,"$Id: uio.h,v 1.8 2018/09/29 02:25:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 *  win32 sys/uio.h
 *
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

#include <sys/cdefs.h>

#include <stddef.h>         /* size_t */
#include <limits.h>         /* INT_MAX */

#define IOV_MAX             64
#define SSIZE_MAX           INT_MAX

__BEGIN_DECLS

typedef struct iovec {
    void *     iov_base;
    int        iov_len;
} iovec_t;

LIBW32_API size_t           readv(int, const struct iovec *, int);
LIBW32_API size_t           writev(int, const struct iovec *, int);

__END_DECLS

#endif /*LIBW32_SYS_UIO_H_INCLUDED */
