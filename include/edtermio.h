#ifndef GR_EDTERMIO_H_INCLUDED
#define GR_EDTERMIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edtermio_h,"$Id: edtermio.h,v 1.13 2019/03/15 23:03:10 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edtermio.h,v 1.13 2019/03/15 23:03:10 cvsuser Exp $
 * Terminal i/o related definitions.
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

#include <config.h>

#if defined(HAVE_TERMIOS_H) || defined(POSIX_TERMIOS)
#include <termios.h>
typedef struct termios TERMIO;
#define HAVE_TERMIOS
#else
#if defined(HAVE_TERMIO_H)
#include <termio.h>
typedef struct termio TERMIO;
#define HAVE_TERMIO
#else
#if defined(HAVE_SGTTY_H)
#include <sgtty.h>
typedef struct sgttyb TERMIO;
#define HAVE_SGTTY_H
#endif
#endif
#endif

#ifdef HAVE_SYS_PTY_H
#include <sys/pty.h>
#endif

#ifdef HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif

enum _ptyflags {
/*--export--enum--*/
/*
 *  Process flags
 */
    PF_ECHO             =0x0001,                /* Local echo on */
    PF_NOINSERT         =0x0002,                /* if on, dont insert to process */
    PF_LOCALECHO        =0x0004,
    PF_OVERWRITE        =0x4000,                /* overwrite process input (CR/LF conversion) */
    PF_WAITING          =0x8000,                /* waiting for text */
/*--end--*/
    PF_NOCR             =0x0010,                /* <CR> not encountered */
    PF_NOLF             =0x0020                 /* <LF> not encountered */
};

#define TTY_INFD        0                       /* in file descriptor */
#define TTY_OUTFD       1                       /* out file descriptor */

#if defined(NEED_FD_SET)
typedef long fd_set;

#define __FD_BIT(x)     (1 << (x))
#define FD_SET(n, p)    *p |= __FD_BIT(n)
#define FD_CLR(n, p)    *p &= ~__FD_BIT(n)
#define FD_ISSET(n, p)  *p & __FD_BIT(n)
#endif

#endif /*GR_EDTERMIO_H_INCLUDED*/
