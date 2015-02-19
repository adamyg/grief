#ifndef GR_WIN32_CHILD_H_INCLUDED
#define GR_WIN32_CHILD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_child_h,"$Id: win32_child.h,v 1.7 2015/02/19 00:17:33 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * child process support
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <sys/cdefs.h>
#include <stdio.h>
#include <win32_include.h>

__BEGIN_DECLS

typedef struct win32_spawn {
    const char *        cmd;
    const char **       argv;
    const char **       envp;
    const char *        dir;
#define W32_SPAWNDETACHED               0x01
    int                 flags;
    unsigned long       _dwFlags;               /* reserved */
    unsigned long       _dwProcessId;           /* reserved */
} win32_spawn_t;

#if !defined(WNOHANG)
#define WNOHANG         1
#endif

const char *            w32_getshell (void);
const char *            w32_gethome (void);

int                     w32_iscommand (const char *);
int                     w32_shell (const char *shell, const char *cmd,
                                        const char *fstdin, const char *fstdout, const char *fstderr);

int                     w32_spawn (win32_spawn_t *args, int Stdout, int Stderr, int *Stdin);
int                     w32_spawn2 (win32_spawn_t *args, int *Stdin, int *Stdout, int *Stderr);

HANDLE                  w32_child_exec (struct win32_spawn *args, HANDLE hStdin, HANDLE hStdOut, HANDLE hStdErr);
int                     w32_child_wait (HANDLE hProc, int *status, int nowait);

FILE *                  w32_popen (const char *cmd, const char *mode);
int                     w32_pclose (FILE *file);
int                     w32_pread_err (FILE *file, char *buf, int length);

ssize_t                 pread (int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t                 pwrite (int fildes, const void *buf, size_t nbyte, off_t offset);

__END_DECLS

#endif /*GR_WIN32_CHILD_H_INCLUDED*/
