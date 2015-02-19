#ifndef GR_BSD_CDBW_H_INCLUDED
#define GR_BSD_CDBW_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_cdbw_h,"$Id: bsd_cdbw.h,v 1.3 2014/07/11 21:25:09 ayoung Exp $")
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
 * $NetBSD: cdbw.h,v 1.1.10.1 2012/06/23 22:54:56 riz Exp $
 */

#include <edtypes.h>

struct cdbw;

__CBEGIN_DECLS

extern struct cdbw *    bsd_cdbw_open(void);
extern int              bsd_cdbw_put(struct cdbw *, const void *, size_t, const void *, size_t);
extern int              bsd_cdbw_put_data(struct cdbw *, const void *, size_t, uint32_t *);
extern int              bsd_cdbw_put_key(struct cdbw *, const void *, size_t, uint32_t);
extern uint32_t         bsd_cdbw_stable_seeder(void);
extern int              bsd_cdbw_output(struct cdbw *, int, const char[16], uint32_t (*)(void));
extern void             bsd_cdbw_close(struct cdbw *);

__CEND_DECLS

#endif /*GR_BSD_CDBW_H_INCLUDED*/
