#ifndef GR_EDTERMCAP_H_INCLUDED
#define GR_EDTERMCAP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edtermcap_h,"$Id: edtermcap.h,v 1.13 2019/03/15 23:03:10 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edtermcap.h,v 1.13 2019/03/15 23:03:10 cvsuser Exp $
 * Terminal interface.
 *
 *
 *
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

#include <sys/types.h>

#define TERMCAP_PATHDEF ".termcap /usr/share/misc/termcap"

__CBEGIN_DECLS

extern int              tgetent(char *bp, const char *name);
extern int              tgetflag(const char *id);
extern int              tgetnum(const char *id);
extern char *           tgetstr(const char *id, char **area);
extern char *           tgoto(const char *cap, int col, int row);
extern int              tputs(const char *id, int affcnt, int (*putc)(int));

struct tinfo;

extern int              t_getent(struct tinfo **, const char *);
extern int              t_getnum(struct tinfo *, const char *);
extern int              t_getflag(struct tinfo *, const char *);
extern char *           t_getstr(struct tinfo *, const char *, char **, size_t *);
extern char *           t_agetstr(struct tinfo *, const char *);
extern int              t_getterm(struct tinfo *, char **, size_t *);
extern int              t_goto(struct tinfo *, const char *, int, int, char *, size_t);
extern int              t_puts(struct tinfo *, const char *, int, void (*)(char, void *), void *);
extern void             t_freent (struct tinfo *);
extern int              t_setinfo(struct tinfo **, const char *);

extern short            ospeed;

__CEND_DECLS

#endif /*GR_EDTERMCAP_H_INCLUDED*/
