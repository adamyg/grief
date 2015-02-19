#ifndef GR_BSD_CDBR_H_INCLUDED
#define GR_BSD_CDBR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_cdbr_h,"$Id: bsd_cdbr.h,v 1.4 2014/10/19 23:45:09 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*-
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Joerg Sonnenberger.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $NetBSD: cdbr.h,v 1.1 2010/04/25 00:54:45 joerg Exp $
 */

#include <edtypes.h>

#define CDBR_DEFAULT    0

struct cdbr;

__CBEGIN_DECLS

extern struct cdbr *    bsd_cdbr_open(const char *, int);
extern uint32_t         bsd_cdbr_entries(struct cdbr *);
extern int              bsd_cdbr_get(struct cdbr *, uint32_t, const void **, size_t *);
extern int              bsd_cdbr_find(struct cdbr *, const void *, size_t, const void **, size_t *);
extern void             bsd_cdbr_close(struct cdbr *);

__CEND_DECLS

#endif /*GR_BSD_CDBR_H_INCLUDED*/
