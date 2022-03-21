#ifndef GR_CHKALLOC_H_INCLUDED
#define GR_CHKALLOC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_chkalloc_h,"$Id: chkalloc.h,v 1.24 2022/03/21 14:55:27 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: chkalloc.h,v 1.24 2022/03/21 14:55:27 cvsuser Exp $
 * Memory management interface.
 *
 *  #define CHKALLOC_DEBUG 1
 *
 *      Enable memory management features, as defined below.
 *
 *      Warning usage maybe mixed across source modules, yet buffers allocated under
 *      CHKALLOC_DEBUG *must* also only be managed, realloced and freed within the same
 *      or other CHKALLOC_DEBUG enabled modules.
 *
 *  Memory profiling/checks are enable using check_configure(), as follows
 *
 *      CHKALLOC_TAIL
 *          Enable heap tail checks post memory operations.
 *
 *      CHKALLOC_FILL
 *          Force malloc'ed memory regions to be initialised with a non-zero pattern.
 *
 *      CHKALLOC_ZERO
 *          Force malloc'ed memory regions to be zero initialised.
 *
 *      CHKALLOC_UNINIT
 *          Force freee memory regions to be 0xDEADBEEF filled.
 *
 *      CHKALLOC_WHERE
 *          Extended trace information.
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <stdlib.h>
#include <edsym.h>

/*
 *  Memory management ...
 */

__CBEGIN_DECLS

extern void             chk_native(void);
extern void             chk_rtc(void);
extern void             chk_stats(void);
extern void *           chk_alloc(size_t size);
extern void *           chk_calloc(size_t elem, size_t size);
extern void             chk_free(void *p);
extern void *           chk_realloc(void *p, size_t size);
extern void *           chk_recalloc(void *p, size_t osize, size_t nsize);
extern size_t           chk_shrink(void *p, size_t size);
extern size_t           chk_expand(void *p, size_t size);
extern void *           chk_salloc(const char *s);
extern void *           chk_snalloc(const char *s, size_t size);

extern void             chk_leak(const void *p);
extern int              chk_isleak(const void *p);

__CEND_DECLS

/*
 *  Memory tracing ...
 */

#if defined(CHKALLOC_DEBUG) && (CHKALLOC_DEBUG)
#define chk_alloc(size)             check_alloc(size, __FILE__, __LINE__)
#define chk_calloc(elem, size)      check_calloc(elem, size, __FILE__, __LINE__)
#define chk_realloc(ptr, size)      check_realloc(ptr, size, __FILE__, __LINE__)
#define chk_recalloc(ptr, osize, nsize) check_recalloc(ptr, osize, nsize, __FILE__, __LINE__)
#define chk_shrink(ptr, size)       check_shrink(ptr, size, __FILE__, __LINE__)
#define chk_expand(ptr, size)       check_expand(ptr, size, __FILE__, __LINE__)
#define chk_leak(ptr)               check_leak(ptr, __FILE__, __LINE__)
#define chk_free(ptr)               OBcheck_free(ptr, __FILE__, __LINE__)
#define chk_salloc(ptr)             check_salloc(ptr, __FILE__, __LINE__)
#define chk_snalloc(ptr, size)      check_snalloc(ptr, size, __FILE__, __LINE__)
#endif

__CBEGIN_DECLS

#define CHKALLOC_TAIL   0x01
#define CHKALLOC_FILL   0x02
#define CHKALLOC_ZERO   0x04
#define CHKALLOC_WHERE  0x08
#define CHKALLOC_UNINIT 0x10

extern unsigned         check_configure(unsigned flags);
extern void *           check_alloc(size_t len, const char *file, unsigned line);
extern void *           check_calloc(size_t elem, size_t len, const char *file, unsigned line);
extern void *           check_realloc(void *p, size_t len, const char *file, unsigned line);
extern void *           check_recalloc(void *p, size_t olen, size_t nlen, const char *file, unsigned line);
extern size_t           check_shrink(void *p, size_t newlen, const char *file, unsigned line);
extern size_t           check_expand(void *p, size_t newlen, const char *file, unsigned line);
extern void             check_free(void *p, const char *file, unsigned line);
extern void *           check_salloc(const char *s, const char *file, unsigned line);
extern void *           check_snalloc(const char *s, size_t len, const char *file, unsigned line);

extern void             check_leak(const void *p, const char *file, unsigned line);
extern int              check_isleak(const void *p);

__CEND_DECLS

#endif /*GR_CHKALLOC_H_INCLUDED*/

