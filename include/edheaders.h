#ifndef GR_EDHEADERS_H_INCLUDED
#define GR_EDHEADERS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edheaders_h,"$Id: edheaders.h,v 1.21 2024/07/14 05:01:46 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edheaders.h,v 1.21 2024/07/14 05:01:46 cvsuser Exp $
 * System headers.
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_ENDIAN_H
#include <endian.h>                             /* BYTE_ORDER */
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#if defined(O_NONBLOCK) && !defined(O_NDELAY)
#define O_NDELAY O_NONBLOCK
#endif
#endif

#ifdef O_BINARY
#define OPEN_R_BINARY       O_BINARY
#define OPEN_W_BINARY       O_BINARY
#define FOPEN_R_BINARY      "rb"
#define FOPEN_W_BINARY      "wb"
#else
#define OPEN_R_BINARY       0
#define OPEN_W_BINARY       0
#define FOPEN_R_BINARY      "r"
#define FOPEN_W_BINARY      "w"
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#if defined(SIGCHLD) && !defined(SIGCLD)
#define SIGCLD              SIGCHLD
#endif
#if defined(SIGCLD) && !defined(SIGCHLD)
#define SIGCHLD             SIGCLD
#endif
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define sig_args            void
#define sig_arg
#define nw_sig()
#elif (RETSIGTYPE == void)
#define sig_args            int i
#define sig_arg             0
#define nw_sig()            (void)i
#else
#define sig_args            void
#define sig_arg
#define nw_sig()
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if defined(HAVE_SYS_IOCTL_COMPAT_H) && defined(HAVE_MOUSE)
#include <sys/ioctl_compat.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_WAIT_H
#if defined(sun)
#include <sysinfo.h>
#endif
#include <wait.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if !defined(WNOHANG) && !defined(SOLARIS2)
#define WNOHANG 1
#endif

#ifndef HAVE_WAITPID
#define waitpid(pid,statusp,options)  wait(statusp)
#endif

#ifdef HAVE_MACHINE_CONSOLE_H
#define HAVE_EGAVGA
#include <machine/console.h>
#undef F
#endif

#ifdef HAVE_SYS_AT_ANSI_H
#define HAVE_EGAVGA
#include <sys/at_ansi.h>
#include <sys/kd.h>
#undef F
#undef DOWN
#undef UP
#undef REVERSE
#undef CTRL
#undef SHIFT
#undef ALT
#undef NORMAL
#endif

#include <assert.h>
#include <ctype.h>

#if !defined(MAX_PATH)                          /* WIN32 */
#if defined(MAXPATHLEN)                         /* bsd/sysv */
#define MAX_PATH            MAXPATHLEN
#elif defined(PATH_MAX)                         /* posix */
#define MAX_PATH            PATH_MAX
#endif
#ifndef MAX_PATH
#define MAX_PATH            1024
#endif
#endif

#if defined(MAXNAMELEN)                         /* bsd/sysv */
#define MAX_NAME            MAXNAMELEN
#elif defined(PATH_MAX)                         /* posix */
#define MAX_NAME            PATH_MAX
#endif
#ifndef MAX_NAME
#define MAX_NAME            256
#endif

/*end*/
#endif /*GR_EDHEADERS_H_INCLUDED*/
