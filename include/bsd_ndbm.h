#ifndef GR_BSD_NDBM_H_INCLUDED
#define GR_BSD_NDBM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_ndbm_h,"$Id: bsd_ndbm.h,v 1.3 2014/07/11 21:25:10 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$NetBSD: ndbm.h,v 1.15 2010/02/03 15:34:40 roy Exp $
 *	@(#)ndbm.h	8.1 (Berkeley) 6/2/93
 */

#include <edtypes.h>
#include <bsd_db.h>

#define DBM_RDONLY		O_RDONLY

/* Flags to dbm_store(). */
#define DBM_INSERT		0
#define DBM_REPLACE		1

/*
 * The db(3) support for ndbm(3) always appends this suffix to the
 * file name to avoid overwriting the user's original database.
 */
#define	DBM_SUFFIX		".db"

typedef struct {
	void *dptr;
	size_t dsize;		/* XPG4.2 */
} datum;

typedef DB DBM;

__CBEGIN_DECLS
void	 bsddbm_close(DBM *);
DBM	*bsddbm_open(const char *, int, mode_t);
int	 bsddbm_error(DBM *);
int	 bsddbm_clearerr(DBM *);
int	 bsddbm_dirfno(DBM *);
#define  bsddbm_pagfno(a)	BSDDBM_PAGFNO_NOT_AVAILABLE
int	 bsddbm_delete(DBM *, datum);
datum	 bsddbm_fetch(DBM *, datum);
datum	 bsddbm_firstkey(DBM *);
datum	 bsddbm_nextkey(DBM *);
int	 bsddbm_store(DBM *, datum, datum, int);
__CEND_DECLS

#endif /*GR_BSD_NDBM_H_INCLUDED*/
