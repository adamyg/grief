#include <edidentifier.h>
__CIDENT_RCSID(gr_pty_os2_c,"$Id: pty_os2.c,v 1.20 2017/01/19 17:09:30 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: pty_os2.c,v 1.20 2017/01/19 17:09:30 cvsuser Exp $
 *
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
#include <edtermio.h>
#include <edalt.h>

#if defined(__OS2__)
#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "line.h"
#include "m_pty.h"
#include "mac1.h"
#include "main.h"
#include "map.h"
#include "procspawn.h"
#include "symbol.h"
#include "system.h"
#include "tty.h"
#include "word.h"

#define INCL_DOS
#include <os2.h>

static int              pty_thread_create(DISPLAY_t *dp);

static int              pipe1[2], pipe2[2];


int
pty_send_signal(int pid, int value)
{
    return kill(pid, value);
}


int
pty_send_term(int pid)
{
    kill(pid, SIGTERM);
}


int
pty_died(BUFFER_t *bp)
{
    if (child_sig)
        proc_wait(-1);

    if (bp->b_display->d_dead) {
        p_cleanup(bp);
        return TRUE;
    }
    return FALSE;
}


int
pty_connect(DISPLAY_t *dp, const char *shell, const char *cwd)
{
    int ipc_type;

    if ((dp->d_pid = create_ipc(&dp->d_pipe_out, &dp->d_pipe_in, &ipc_type)) < 0) {
        pty_free(dp);
        ewprintf("Couldn't create IPC.");
        return -1;

    } else {
#if defined(HAVE_SPAWN)
        int saved_stdin = dup(0);
        int saved_stdout = dup(1);
        int saved_stderr = dup(2);
        int sh_len = strlen(shell);

        sys_noinherit(saved_stdin);
        sys_noinherit(saved_stdout);
        sys_noinherit(saved_stderr);

        dup2(pipe1[0], 0);
        dup2(pipe2[1], 1);
        dup2(pipe2[1], 2);

        sys_noinherit(dp->d_pipe_out);
        sys_noinherit(dp->d_pipe_in);
        sys_noinherit(pipe1[0]);
        sys_noinherit(pipe2[1]);
#if defined(ICANON)
        sys_pty_mode(0);
#endif

        /*
         *  Try to detect the type of shell
         *  A hack to get this to work with UNIX type shells.
         */
        if (strchr(shell, '/') != NULL || str_icmp(shell+sh_len-2, "sh") == 0 ||
                    (sh_len >= 6 && str_icmp(shell+sh_len-6, "sh.exe") == 0)) {
            dp->d_pid = spawnlp(P_NOWAIT, shell, shell, "-i", (char *) NULL);
        } else {
            dp->d_pid = spawnlp(P_NOWAIT, shell, shell, (char *) NULL);
        }

        dup2(saved_stdin, 0);
        close(saved_stdin);
        dup2(saved_stdout, 1);
        close(saved_stdout);
        dup2(saved_stderr, 2);
        close(saved_stderr);

        if (dp->d_pid < 0) {
            ewprintf("Couldn't spawn %s.", shell);
            goto disp_err;
        }

        if (ipc_type)
            close(pipe1[0]);

        if (pty_thread_create(dp) < 0) {
            ewprintf("Cannot create thread.");
disp_err:
            close(pipe1[0]);
            close(pipe2[0]);
            close(pipe1[1]);
            close(pipe2[1]);
            pty_free(dp);
            return -1;
        }

#else
        /*
         *  Child gets to exec a shell
         */
        if (0 == dp->d_pid) {
#if defined(SIGCLD)
            signal(SIGCLD, SIG_DFL);
#endif
            if (cwd) chdir(cwd);
#if defined(DOSISH)                             /* exec */
            execlp(shell, shell, (char *) NULL);
#else
            execlp(shell, shell, "-i", (char *) NULL);
#endif
            trace_log("exec failed");
            _exit(1);
        }
#endif
        proc_add(dp->d_pid, NULL, NULL, NULL, 0);

        /*
         *  Parent gets to tidy up
         */
#if defined(F_GETFL)
        fcntl(dp->d_pipe_in, F_SETFL, fcntl(dp->d_pipe_in, F_GETFL, 0) | O_NDELAY);
        fcntl(dp->d_pipe_out, F_SETFL, fcntl(dp->d_pipe_out, F_GETFL, 0) | O_NDELAY);
#endif

        acc_assign_int(0);
        curbp->b_display = dp;
        if (dp->d_pid >= 0)
            infof("Buffer connected.");
        wait_state |= WAIT_PTY;
        io_device_add(dp->d_pipe_in);
        pty_poll();
    }
    return 0;
}


static int
create_ipc(int *_send, int *_recv, int *ipc_type)
{
    int pid;

    /*
     *  Try to use a pty first. If that fails, then try and use a pipe.
     *  On systems with pty support but running a binary with the pty
     *  code compiled in, then we should work just fine.
     */
    *ipc_type = 1;
    if ((pid = create_pty(_send, _recv)) < 0) {
        --(*ipc_type);

        if ((pid = create_pipe(_send, _recv)) < 0) {
            --(*ipc_type);
            return -1;
        }
    }

    /*
     *  At this point we have created the IPC mechanism, and we are
     *  running in both the parent and child, via vfork().
     */
#if defined(SIGTTIN)
    if (0 == pid) {
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
    }
#endif
    return pid;
}


int
create_pty(int *_send, int *_recv)
{
#if !defined(NO_PTY)
    int xx, c1, c2;
    char pty_name[20];
    char tty_name[20];

#define PTY_NAME            "/dev/ptyXX"
#define TTY_NAME            "/dev/ttyXX"
#define TTY_MODE            0622

    if (getenv("GRIPC"))                        /* Force pipe method */
        return -1;

    strcpy(pty_name, PTY_NAME);
    strcpy(tty_name, TTY_NAME);

    for (xx = 0; pty_name[xx] != 'X';)
        xx++;

    for (c1 = 0; c1 < 8; c1++) {
        tty_name[xx] = pty_name[xx] = "pqrsPQRS"[c1];

        for (c2 = 0; c2 < 32; c2++) {

#if defined(__OS2__)
            ULONG action;
#endif

            tty_name[xx + 1] = pty_name[xx + 1] = "0123456789abcdefghijklmnopqrstyv"[c2];
#if defined(__OS2__)

            /*
             *  Need special handling for XF86SUP pty driver
             */
            if (DosOpen(pty_name, (PHFILE)_send, &action, 0, FILE_NORMAL, FILE_OPEN,
                    OPEN_FLAGS_FAIL_ON_ERROR|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE, (PEAOP2)NULL) == 0) {

                if (DosOpen(tty_name, (PHFILE)_recv, &action, 0, FILE_NORMAL, FILE_OPEN,
                        OPEN_FLAGS_FAIL_ON_ERROR|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE, (PEAOP2)NULL) != 0)
#else
            if ((*_send = open(pty_name, O_RDWR)) >= 0) {

                if ((*_recv = open(tty_name, O_RDWR)) < 0)
#endif
                    close(*_send);
                else
                {
#if defined(HAVE_FORK)
                    int pid;

                    fcntl(*_send, F_SETFD, 1);
                    if ((pid = fork()) < 0) {
                        close(*_send);
                    } else {
                        if (pid == 0) {
#if defined(TIOCSPGRP) || defined(HAVE_SETPGID)
                            pid_t mypid = getpid();
#endif
                            (void)mypid;
#if defined(SIGTTIN)
                            signal(SIGTTIN, SIG_DFL);
                            signal(SIGTTOU, SIG_DFL);
#endif
                            signal(SIGINT, SIG_DFL);
#if defined(TIOCNOTTY)
                            ioctl(*_recv, TIOCNOTTY);
#endif
                            close(0);
                            close(1);
                            close(2);
                            close(*_send);

                            /*
                             *  Change our process group so we get the tty as a controlling tty
                             */
                            if ((*_recv = open(tty_name, O_RDWR)) != 0)
                                _exit(0);

                            dup(*_recv);
                            dup(*_recv);

#if defined(TIOCSCTTY) && defined(HAVE_SETSID)
                            setsid();
                            ioctl(0, TIOCSCTTY, 0);
#else
#if defined(TIOCSPGRP)
                            ioctl(0, TIOCSPGRP, (char *) &mypid);
#endif
#if defined(HAVE_SETPGID)
                            setpgid(mypid, mypid);
#elif defined(HAVE_SETPGRP)
                            setpgrp();
#endif
#endif
#if defined(HAVE_CHOWN)
                            chown(tty_name, getuid(), getgid());
#endif
                            chmod(tty_name, TTY_MODE);
#if defined(ICANON)
                            sys_pty_mode(0);
#endif
                        } else {
                            close(*_recv);
                            *_recv = *_send;
#if defined(SIGTTIN)
                            signal(SIGTTIN, SIG_IGN);
                            signal(SIGTTOU, SIG_IGN);
#endif
                        }
                        return pid;
                    }

#else   /* Assume HAVE_SPAWN */

                    pipe1[0] = pipe2[1] = *_recv;       /* Our send, child's stdin */
                    *_recv = *_send;
                    return 0;
#endif
                }
            }
        }
    }
#endif

    return -1;
}


static int
create_pipe(int * _send, int * _recv)
{
    if (pipe(pipe1) < 0)
        return -1;

    if (pipe(pipe2) < 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        return -1;

    } else {
#if defined(HAVE_FORK)
        int pid;

        if ((pid = vfork()) < 0) {
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            return -1;
        }

        if (pid == 0) {
            /*
             *  Set the process group to be the same as the current PID
             *  so we can send it signals. We pass an argument so this
             *  will work on SYSV and BSD
             */
#if defined(HAVE_SETPGID)
            setpgid(getpid(), getpid());
#elif defined(HAVE_SETPGRP)
            setpgrp();
#endif

            dup2(pipe1[0], 0);
            dup2(pipe2[1], 1);
            dup2(pipe2[1], 2);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

        } else {
            close(pipe2[1]);
            close(pipe1[0]);
            *_send = pipe1[1];
            *_recv = pipe2[0];
        }
        return pid;

#else   /* Assume HAVE_SPAWN */
        *_send = pipe1[1];
        *_recv = pipe2[0];
        return 0;
#endif
    }
}


#if defined(__FLAT__)

static void
pty_thread(ULONG arg)
{
    ULONG pCount;
    DISPLAY_t *dp = global_dp;

    (void)arg;
    DosPostEventSem(pty_sem);
    dp->rcvlen = 0;
    dp->d_dead = FALSE;

    while (1) {
        ULONG rcvlen = PTY_BUFSIZ -  dp->rcvlen;

        if (DosRead(dp->d_pipe_in, dp->d_buf + dp->rcvlen, rcvlen, (PULONG)rcvlen) != 0) {
            dp->d_dead = TRUE;
            DosExit(EXIT_THREAD, 0);
        }

        dp->d_rcvlen += rcvlen;
        DosPostEventSem(dp->d_sema);
        DosWaitEventSem(dp->d_wait, SEM_INDEFINITE_WAIT);
        DosResetEventSem(dp->d_wait, &pCount);
    }
}


static int
pty_thread_create(DISPLAY_t * dp)
{
    TID ptytid;

    global_dp = dp;
    dp->d_dead = TRUE;
    dp->d_sema = 0;
    dp->d_wait = 0;
    DosCreateEventSem(NULL, &pty_sem, 0L, 0);
    DosCreateEventSem(NULL, &dp->d_sema, 0L, 0);
    DosCreateEventSem(NULL, &dp->d_wait, 0L, 0);

    if (DosCreateThread(&ptytid, &pty_thread, 0L, 1L, PTY_STACK) == 0)
        dp->d_dead = FALSE;
    else
    {
        ewprintf("Cannot create thread.");
        return -1;
    }
    DosResumeThread(ptytid);
    DosWaitEventSem(pty_sem, SEM_INDEFINITE_WAIT);
    DosCloseEventSem(pty_sem);
    return 0;
}


int
pty_read(BUFFER_t *bp, char *buf, int count)
{
    DISPLAY_t *dp = bp->b_display;
    ULONG pCount;

    if (dp->d_dead)
        return -1;

    if (DosQueryEventSem(dp->d_sema, &pCount) != 0 || pCount == 0)
        return 0;

    DosResetEventSem(dp->d_sema, &pCount);
    if (dp->d_rcvlen == 0)
        count = 0;
    else
    {
        if (dp->d_rcvlen < count)
        count = dp->d_rcvlen;
        memcpy(buf, dp->d_buf, count);
        if ((dp->d_rcvlen -= count) > 0)
            memmove(dp->d_buf, dp->d_buf + count, dp->d_rcvlen);
        DosPostEventSem(dp->d_wait);
        DosSleep(0L);                           /* Give other thread chance to run. */
    }
    return count;
}

#else   /* !__FLAT__ */

static void
pty_thread(void)
{
    DISPLAY_t *dp = global_dp;

    DosSemClear(&pty_sem);

    while (1) {
        USHORT rcvlen = PTY_BUFSIZ - dp->d_rcvlen;

        if (DosRead(dp->d_pipe_in, dp->d_buf, rcvlen, &rcvlen) != 0) {
            dp->d_dead = TRUE;
            return;
        }
        dp->d_rcvlen += rcvlen;
        DosSemClear(&dp->d_sema);
        DosSemRequest(&dp->d_wait, SEM_INDEFINITE_WAIT);
    }
}


static int
pty_thread_create(DISPLAY_t * dp)
{
    global_dp = dp;
    dp->d_dead = FALSE;
    DosSemSet(&pty_sem);
    dp->d_sema = 0;
    dp->d_wait = 0;
    DosSemSet(&dp->d_sema);
    DosSemSet(&dp->d_wait);

    if (DosCreateThread(pty_thread, &tid, &dp->d_stack[PTY_STACK]) != 0) {
        ewprintf("Cannot create thread.");
        return -1;
    }
    DosSemWait(&pty_sem, SEM_INDEFINITE_WAIT);
    return 0;
}


int
pty_read(BUFFER_t *bp, char *buf, int amount)
{
    DISPLAY_t *dp = bp->b_display;

    if (dp->d_sema)
        return 0;

    DosSemSet(&dp->d_sema);
    if (dp->d_rcvlen == 0) {
        count = 0;
    } else {
        if (dp->d_rcvlen < count)
        count = dp->d_rcvlen;
        memcpy(buf, dp->d_buf, count);
        if ((dp->d_rcvlen -= count) > 0)
            memmove(dp->d_buf, dp->d_buf + count, dp->d_rcvlen);
        DosSemClear(&dp->d_wait);
        DosSleep(0L);                           /* Give other thread chance to run. */
    }
    return count;
}
#endif

#endif  /*__OS2__*/