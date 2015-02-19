#ifndef GR_MMAN_H_INCLUDED
#define GR_MMAN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_mman_h,"$Id: mman.h,v 1.8 2015/02/19 00:17:38 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mmap implementation
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <stddef.h>                             /* size_t */

/*
 * Protections are chosen from these bits, or-ed together.
 * Note - not all implementations literally provide all possible
 * combinations.  PROT_WRITE is often implemented as (PROT_READ |
 * PROT_WRITE) and (PROT_EXECUTE as PROT_READ | PROT_EXECUTE).
 * However, no implementation will permit a write to succeed
 * where PROT_WRITE has not been set.  Also, no implementation will
 * allow any access to succeed where prot is specified as PROT_NONE.
 */

#define PROT_READ       0x1                     /* pages can be read */
#define PROT_WRITE      0x2                     /* pages can be written */
#define PROT_EXEC       0x4                     /* pages can be executed */
#define PROT_NONE       0x0                     /* pages cannot be accessed */

/* sharing types:  must choose either SHARED or PRIVATE */
#define MAP_SHARED      1                       /* share changes */
#define MAP_PRIVATE     2                       /* changes are private */
#define MAP_TYPE        0xf                     /* mask for share type */

/* mapping type */
#define MAP_FILE        0                       /* regular file */

/* other flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE) */
#define MAP_FIXED       0x10                    /* user assigns address */
#define MAP_NORESERVE   0x40                    /* don't reserve needed swap area */

/* these flags not yet implemented */
#define MAP_RENAME      0x20                    /* rename private pages to file */

/* return value on failure */
#if !defined (MAP_FAILED)                       /* Failure return value. */
#define MAP_FAILED      ((void *) -1)
#endif


/* flags to msync */
#define MS_SYNC         0x4                     /* wait for msync */
#define MS_ASYNC        0x1                     /* return immediately */
#define MS_INVALIDATE   0x2                     /* invalidate caches */

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

void * __PDECL          mmap __P((void *addr, size_t len, int prot, int flags, int fildes, off_t off));
int __PDECL             mprotect __P((void *addr, size_t len, int prot));
int __PDECL             msync __P((void *addr, size_t len, int flags));
int __PDECL             munmap __P((void *addr, size_t len));
int __PDECL             mlock __P((const void *, size_t));
int __PDECL             munlock __P((const void *, size_t));

__END_DECLS

#endif /*GR_MMAN_H_INCLUDED*/
