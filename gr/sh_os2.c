#include <edidentifier.h>
__CIDENT_RCSID(gr_sh_os2_c,"$Id: sh_os2.c,v 1.14 2024/08/18 10:52:14 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sh_os2.c,v 1.14 2024/08/18 10:52:14 cvsuser Exp $
 *
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

#include <editor.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#if defined(__OS2__)

#include "accum.h"
#include "builtin.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "getkey.h"
#include "mouse.h"
#include "main.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"


static const char *             outdirect(const char *path, int *mode);

/*
 *  UNDEFUNCT
 */
int
sys_shell(const char *cmd, const char *macro, 
        const char *fstdin, const char *fstdout, const char *fstderr, int mode)
{
    int fd = -1, old_stdin = -1, old_stdout = -1, old_stderr = -1;
    int xstdout = 0, xstderr = 0;
    int status = 0;

    void (*oisig)();
    void (*oqsig)();

    fstdout = outdirect(fstdout, &xstdout);
    fstderr = outdirect(fstderr, &xstderr);
    if (mode <= 0) {
        mode = 0600;                            /* default creation mode */
    }

    if (fstdin && -1 != (fd = open(fstdin, O_RDONLY))) {
        old_stdin = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        sys_noinherit(old_stdin);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
                                                /* stdout */
    if (fstdout && -1 != (fd = open(fstdout, xstdout, mode))) {
        old_stdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        sys_noinherit(old_stdout);
        if (!fstderr) {
            old_stderr = dup(STDERR_FILENO);
            dup2(fd, STDERR_FILENO);
            sys_noinherit(old_stderr);
        }
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
                                                /* stderr */
    if (fstderr && -1 != (fd = open(fstderr, xstderr, mode))) {
        old_stderr = dup(STDERR_FILENO);
        dup2(fd, STDERR_FILENO);
        sys_noinherit(old_stderr);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }

    proc_prep_stop(TRUE);
    oisig = signal(SIGINT, SIG_IGN);
#if defined(SIGQUIT)
    oqsig = signal(SIGQUIT, SIG_IGN);
#endif

    status = system(cmd);

    if (-1 != old_stderr) {
        dup2(old_stderr, STDERR_FILENO);
        close(old_stderr);
    }

    if (-1 != old_stdout) {
        dup2(old_stdout, STDOUT_FILENO);
        close(old_stdout);
    }

    if (-1 != old_stdin) {
        dup2(old_stdin, STDIN_FILENO);
        close(old_stdin);
    }

    signal(SIGINT, oisig);
#if defined(SIGQUIT)
    signal(SIGQUIT, oqsig);
#endif
    return status;
}


static const char *
outdirect(const char *path, int *mode)
{
    *mode = O_WRONLY|O_CREAT|O_TRUNC;
    if (path) {
        if (*path == '>') {                     /* ">name" */
            ++path;
            if (*path == '>') {                 /* ">>name" */
                *mode = O_WRONLY|O_CREAT|O_APPEND;
                ++path;
            }
        }
    }
    return path;
}

#endif  /*__OS2__*/
