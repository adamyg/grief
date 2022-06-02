#ifndef GR_SYS_RWLOCK_H_INCLUDED
#define GR_SYS_RWLOCK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_rwlock_h,"$Id: rwlock.h,v 1.11 2022/06/02 10:37:09 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <rwlock.h> implementation
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

typedef struct rwlock {
    unsigned int        opaque[16];
} rwlock_t;

#define RWLOCK_INITIALIZER      {0xffff}

__BEGIN_DECLS

LIBW32_API void         rwlock_init(struct rwlock *rw);
LIBW32_API void         rwlock_destroy(struct rwlock *rw);
LIBW32_API void         rwlock_rdlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrlock(struct rwlock *rw);
LIBW32_API void         rwlock_rdunlock(struct rwlock *rw);
LIBW32_API void         rwlock_wrunlock(struct rwlock *rw);
LIBW32_API void         rwlock_unlock(struct rwlock *rw);
LIBW32_API int          rwlock_status(struct rwlock *rw);

__END_DECLS

#endif /*GR_SYS_RWLOCK_H_INCLUDED*/
