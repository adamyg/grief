#include <edidentifier.h>
__CIDENT_RCSID(gr_sh_unix_c,"$Id: sh_unix.c,v 1.18 2014/10/22 02:33:19 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sh_unix.c,v 1.18 2014/10/22 02:33:19 ayoung Exp $
 *
 *      Linux
 *      Sun Solaris
 *      AIX
 *      HP/UX
 *      Mac OS/X
 *      Cygwin
 *          - working
 *
 *      DJGPP
 *          - Does not work, neither pipe nor spawn NOWAIT are fully implemented.
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

#if defined(unix) || defined(__APPLE__)

#if defined(HAVE_SPAWN_H)
#include <spawn.h>                              /* posix spawn interface */
#elif defined(HAVE_VFORK_H) && defined(HAVE_WORKING_VFORK)
#include <vfork.h>
#endif

#if !defined(HAVE_ENVIRON_DECL)
extern char **environ;                          /* current environment */
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO        0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO       1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO       2
#endif
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "main.h"
#include "mouse.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"

static const char *         outdirect(const char *path, int *mode);


/*  Function:               sys_shell
 *      System specfic shell interface.
 *
 *  Parameters:
 *      cmd -                   Command line.
 *      macro -                 Completion macro.
 *      fstdin -                stdin redirection.
 *      fstdout -               stdout redirection.
 *      fstderr -               stderr redirection.
 *      mode -                  file creation mode.
 *
 *  Returns:
 *      Exit status.
 */
int
sys_shell(const char *cmd, const char *macro,
        const char *fstdin, const char *fstdout, const char *fstderr, int mode)
{
    static char const *sharg[] = {
        /* UNIX 0,1 */ "-c", "-i",
        /* DOS  2,3 */ "/C", ""
        };
    const char *shname = proc_shell_get(),
            *name = sys_basename(shname);
    void (*oisig)(int);
    void (*oqsig)(int);
    int xstdout = 0, xstderr = 0;
    int interactive = 0;
    int status = 0;

#if defined(DOSISH)
    if (proc_shell_iscmd(shname)) {
        interactive = 2;                        /* change base */
    }
#endif
    if (NULL == cmd || !*cmd) {
        ++interactive;                          /* command or interactive */
    }

    fstdout = outdirect(fstdout, &xstdout);
    fstderr = outdirect(fstderr, &xstderr);
    if (mode <= 0) {
        mode = 0600;                            /* default creation mode */
    }

    oisig = signal(SIGINT, SIG_IGN);
#if defined(SIGQUIT)
    oqsig = signal(SIGQUIT, SIG_IGN);
#endif

    trace_log("sys_shell(sh:%s,options:%s,cmd:%s,macro:%s,in:%s,out:%s,err:%s)\n", shname, sharg[interactive], \
        (cmd ? cmd : ""), (macro ? macro : ""), (fstdin ? fstdin : ""), (fstdout ? fstdout : ""), (fstderr ? fstderr : ""));

#if defined(HAVE_SPAWN)
    {                                           /* system() */
        int old_stdin, old_stdout, old_stderr, fd;

        old_stdin = old_stdout = old_stderr = fd = -1;

                                                /* stdin */
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

        status = system(cmd ? cmd : "");        /* execute command */

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

        if (macro) {                            /* completion macro */
            char *tmpstr = chk_alloc(strlen(macro) + 16);
            const char *mp;
            char *cp;

            cp = tmpstr;
            for (mp = macro; *mp && !isspace(*mp);) {
                *cp++ = *mp++;
            }
            *cp++ = ' ';
            sprintf(cp, "%d", status);
            execute_str(tmpstr);
            chk_free(tmpstr);
        }
    }

#elif defined(HAVE_POSIX_SPAWNP)
    {                                           /* posix_spawn() */
        sigset_t blocked_signals = {0};
        posix_spawn_file_actions_t actions = {0};
        posix_spawnattr_t attrs = {0};
        int haveactions = FALSE, haveattrs = FALSE;
        char *argv[4] = {NULL};
        pid_t pid = -1;
        int err;

        if (!fstderr) {                         /* stderr, otherwise redirect to stdout */
            fstderr = fstdout, xstderr = xstdout;
        }

        argv[0] = (char *) name;
        argv[1] = (char *) sharg[interactive];
        argv[2] = (char *) cmd;
        argv[3] = NULL;

        if (0 != (err = posix_spawn_file_actions_init(&actions))
                ||  (haveactions = TRUE,
	                    (fstdin  &&
                            0 != (err = posix_spawn_file_actions_addopen(&actions,
                                            STDIN_FILENO, fstdin, O_RDONLY, 0)))
	
                    || (fstdout &&
                            0 != (err = posix_spawn_file_actions_addopen(&actions,
                                            STDOUT_FILENO, fstdout, xstdout, mode)))
	
                    || (fstderr &&
                            0 != (err = posix_spawn_file_actions_addopen(&actions,
                                            STDERR_FILENO, fstderr, xstderr, mode)))

	                 || (0 != (err = posix_spawnattr_init(&attrs))
		                      ||  (haveattrs = TRUE,
                                0 != (err = posix_spawnattr_setsigmask(&attrs, &blocked_signals))
		                          || 0 != (err = posix_spawnattr_setflags(&attrs, POSIX_SPAWN_SETSIGMASK))))
	
                    || 0 != (err = posix_spawnp(&pid, shname, &actions, (haveattrs ? &attrs : NULL), argv, environ))))
        {
            if (haveattrs) {
                posix_spawnattr_destroy(&attrs);
            }
            if (haveactions) {
                posix_spawn_file_actions_destroy(&actions);
            }
            ewprintx("shell: spawn error '%s%s%s'", shname, (cmd ? " " : ""), (cmd ? cmd : ""));
            return -1;
        }

        proc_add(pid, macro, NULL, NULL, 0);
        if (NULL == macro) {
            status = proc_wait(pid);
        }

        posix_spawnattr_destroy(&attrs);
        posix_spawn_file_actions_destroy(&actions);
    }

#else
    {                                           /* fork() */
#if defined(HAVE_VFORK) && defined(HAVE_WORKING_VFORK)
#define FORK            vfork
#else
#define FORK            fork
#endif
        pid_t pid;

        if (-1 == (pid = FORK())) {
            /*
             *  error
             */
            ewprintx("shell: fork error: %d (%s)");
            status = -1;

        } else if (0 == pid) {
            /*
             *  child
             */
            int xerrno;

#if defined(SIGCLD)
            (void) signal(SIGCLD, SIG_DFL);
#endif
            (void) signal(SIGINT, SIG_DFL);

            if (fstdin && NULL == freopen(fstdin, "r", stdin)) {
                xerrno = errno;
                printf("shell: cannot redirect from \"%s\": %d (%s)\n", fstdin, xerrno, str_error(xerrno));
            }

            if (fstdout && NULL == freopen(fstdout, ((xstdout & O_APPEND) ? "a" : "w"), stdout)) {
                xerrno = errno;
                printf("shell: cannot redirect to \"%s\": %d (%s)\n", fstdout, xerrno, str_error(xerrno));
            }

            if (fstdout || fstderr) {
                if (!fstderr) {
                    fstderr = fstdout, xstderr = xstdout;
                }
                if (NULL == freopen(fstderr, ((xstderr & O_APPEND) ? "a" : "w"), stderr)) {
                    const int xerrno = errno;
                    printf("shell: cannot redirect to \"%s\": %d (%s)\n", fstderr, xerrno, str_error(xerrno));
                }
            }
            execl(shname, name, sharg[interactive], cmd, (char *) NULL);
            xerrno = errno;
            printf("shell: exec error '%s %s': %d (%s)",  shname, (cmd ? cmd : ""), xerrno, str_error(xerrno));
            _exit(1);

        } else {
            /*
             *  parent
             */
            proc_add(pid, macro, NULL, NULL, 0);
            if (NULL == macro) {
                status = proc_wait(pid);
            }
        }
    }
#endif  /*HAVE_SPAWN|HAVE_POSIX_SPAWN|FORK*/

    signal(SIGINT, oisig);
#if defined(SIGQUIT)
    signal(SIGQUIT, oqsig);
#endif

    trace_log("==> %d\n", status);
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


int
sys_waitpid(int pid, int *statusp, int options)
{
    int status = 0, wait_status = 0, ret;

#if defined(HAVE_WAITPID)
    ret = waitpid(pid, &wait_status, options);

#elif defined(HAVE_WAIT4) && defined(HAVE_WAIT3)
    if (pid < 0) {
        pid = wait3(&wait_status, options, (struct rusage *)NULL);
    } else {
        pid = wait4(pid, &wait_status, options, (struct rusage *)NULL);
    }

#else
#error waitpid not wait4/wait3 not available ...
#endif

    if (ret >= 0) {
        if (WIFSIGNALED(wait_status)) {         /* signal */
            status = -WTERMSIG(wait_status);

        } else if (WIFEXITED(wait_status)) {    /* normal exit */
            status = WEXITSTATUS(wait_status);

        } else {                                /* unknown condition */
            status = 999;
        }
    } else {
        status = -999;
    }

    if (statusp) {
        *statusp = status;
    }
    return ret;
}

#endif  /*unix*/
