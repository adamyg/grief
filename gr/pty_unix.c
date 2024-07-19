#include <edidentifier.h>
__CIDENT_RCSID(gr_pty_unix_c,"$Id: pty_unix.c,v 1.27 2024/07/19 05:04:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: pty_unix.c,v 1.27 2024/07/19 05:04:22 cvsuser Exp $
 * PTY interface for Unix and Unix-like environments.
 *
 *      Linux
 *      Solaris
 *      AIX
 *      HP/UX
 *      Mac OS/X
 *      Cygwin -
 *          Functional
 *
 *      DJGPP -
 *          Neither pipe nor spawn NOWAIT are fully implemented.
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

#if (defined(linux) || defined(__CYGWIN__)) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE       600                 /* grantpt() */
#endif
#if defined(linux) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <editor.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#if defined(unix) || defined(__unix__) || defined(__APPLE__)

#if !defined(ED_LEVEL)
#define ED_LEVEL            1                   /* debug/trace level */
#endif

#if defined(HAVE_PTY_H)
#include <pty.h>                                /* psuedo tty interface */
#endif
#if defined(HAVE_STROPTS_H)
#include <stropts.h>                            /* stream options */
#endif
#if defined(HAVE_GRP_H)
#include <grp.h>                                /* group interface */
#endif

#if defined(HAVE_VFORK) && defined(HAVE_WORKING_VFORK)
#if defined(HAVE_VFORK_H)
#include <vfork.h>
#endif
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

#include <edtermio.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "buffer.h"
#include "debug.h"
#include "echo.h"
#include "m_pty.h"
#include "main.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"

#define IPC_PTY             1
#define IPC_PIPE            2

static const char           x_ptymajor[] = "pqrstuvwxyzPQRSTabced";
static const char           x_ptyminor[] = "0123456789abcdef";

static int                  create_pty(int *sendfd, int *recvfd, int lines, int cols, const char *shell, const char *cwd);
static int                  create_pipe(int *sendfd, int *recvfd, int lines, int cols, const char *shell, const char *cwd);
static void                 close_pipe(int *fds, int check);
#if defined(HAVE_SPAWN)
static int                  pty_spawn(int *pipe1, int *pipe2, const char *shell, const char *cwd);
#endif
static void                 pty_perror(const char *fmt, ...);

int                         sys_pty_openpt(void);
int                         sys_pty_grantpt(int fd);
int                         sys_pty_unlockpt(int fd);
const char *                sys_pty_ptsname(int fd, char *buf, int len);
const char *                sys_pty_bsd(int *master, int *slave, char *buf, int len);

static const char *         pty_bsd_name(char *name, int major, int minor);
static const char *         pty_tty_name(char *name, int major, int minor);


/*
 *  pty_send_signal ---
 *      Send signal to process group.
 */
int
pty_send_signal(int pid, int value)
{
    return kill(-pid, value);
}


/*
 *  pty_send_term ---
 *      Send a terminate signal to the specified process.
 */
void
pty_send_term(int pid)
{
    kill(pid, SIGKILL);
}


/*
 *  pty_died ---
 *      Determine whether the attached process has completed/died.
 */
int
pty_died(BUFFER_t *bp)
{
    DISPLAY_t *dp = bp->b_display;

    proc_check();                               /* process any SIGCLD's */
    if (dp) {
        if (0 == kill(dp->d_pid, 0)) {
            return FALSE;
        }
        ED_TRACE(("==> dp:%p died : %d\n", dp, errno))
        pty_cleanup(bp);
    }
    return TRUE;
}


int
pty_connect(DISPLAY_t *dp, const char *shell, const char *cwd)
{
    const char *comma, *bipc = NULL;
    int pid = -1;

    /*
     *  Create the IPC mechanism (pty and/or pipe) and fork a child
     */
    if (NULL != (bipc = ggetenv("GRIPC"))) {
        if (0 == bipc[0]) {
            bipc = "pipe";                      /* pipe only */
        }
    } else {
        bipc = "pty,pipe";                      /* default */
    }

    ED_TRACE(("==> bipc:%s\n", bipc))

    while (*bipc && pid < 0) {

        if (0 == strncmp(bipc, "pty", 3)) {
            pid = create_pty(&dp->d_pipe_out, &dp->d_pipe_in,
                        dp->d_rows, dp->d_cols, shell, cwd);
            bipc += 3;

        } else if (0 == strncmp(bipc, "pipe", 4)) {
            pid = create_pipe(&dp->d_pipe_out, &dp->d_pipe_in,
                        dp->d_rows, dp->d_cols, shell, cwd);
            bipc += 4;
        }

        if (pid < 0) {                          /* next type */
            if (NULL == (comma = strchr(bipc, ','))) {
                break;
            }
            bipc = ++comma;
        }
    }

    if ((dp->d_pid = pid) < 0) {
        ewprintf("cannot create IPC.");
        return -1;                              /* error */
    }

    /*
     *  At this point we have created the IPC mechanism
     */
#if defined(SIGTTIN)
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
#endif

#if defined(F_GETFL)
    fcntl(dp->d_pipe_in, F_SETFL, fcntl(dp->d_pipe_in, F_GETFL, 0) | O_NDELAY);

    if (dp->d_pipe_in != dp->d_pipe_out) {
        fcntl(dp->d_pipe_out, F_SETFL, fcntl(dp->d_pipe_out, F_GETFL, 0) | O_NDELAY);
    }
#endif

    return TRUE;                                /* connected */
}


/*
 *  pty_read ---
 *      non-blocking PTY read ...
 */
int
pty_read(BUFFER_t *bp, char *buf, int count)
{
    DISPLAY_t *dp = bp->b_display;
    int cnt = -1;

    if (dp) {
        int fd = dp->d_pipe_in;
#if defined(F_GETFL)
        int flags = fcntl(fd, F_GETFL, 0);
#endif

#if defined(F_GETFL)
        if (0 == (O_NDELAY & flags)) {
            fcntl(fd, F_SETFL, flags | O_NDELAY);
        }
#endif
        cnt = read(fd, buf, count);
#if defined(F_GETFL)
        if (0 == (O_NDELAY & flags)) {
            fcntl(fd, F_SETFL, flags);
        }
#endif
    }
    return cnt;
}


/*  Function:           create_pty
 *      pseudo tty implementation.
 *
 *   Parameters:
 *      sendfd - send file descriptor.
 *      recvfd - recieve file descriptor.
 *      lines - Screen lines
 *      cols - Columns.
 *      shell - Shell specification.
 *      cwd - Optional current working directory.
 *
 *  Returns:
 *      Zero on success, otherwise -1.
 */
static int
create_pty(int *sendfd, int *recvfd, int lines, int cols, const char *shell, const char *cwd)
{
    int master = -1, slave = -1;
    char pts_name[256] = {0};
    pid_t pid;

    ED_TRACE(("create_pty(lines:%d,cols:%d,shell:\"%s\")\n", lines, cols, shell))

    /*
     *  pseudo tty
     */
#if defined(HAVE_GETPT) || defined(HAVE_POSIX_OPENPT)
    if ((master = sys_pty_openpt()) < 0) {
        goto pty_error;
    }

    if (sys_pty_grantpt(master) < 0 ||          /* XXX - block signals */
            sys_pty_unlockpt(master) < 0 ||
            NULL == sys_pty_ptsname(master, pts_name, sizeof(pts_name)))  {
        goto pty_error;
    }

    if ((slave = open(pts_name, O_RDWR)) < 0) {
        pty_perror("open(\"%s\")", pts_name);
        goto pty_error;
    }

#if defined(HAVE_STROPTS_H) && \
        defined(I_FIND) && defined(I_PUSH)
    if (isastream(slave)) {
        if (0 == ioctl(slave, I_FIND, "ptem") && ioctl(slave, I_PUSH, "ptem") < 0) {
            pty_perror("ioctl(fd, I_PUSH, \"ptem\")");
            goto pty_error;
        }

        if (0 == ioctl(slave, I_FIND, "ldterm") && ioctl(slave, I_PUSH, "ldterm") < 0) {
            pty_perror("ioctl(fd, I_PUSH, \"ldterm\")");
            goto pty_error;
        }

#if !defined(sgi) && !defined(__sgi)
        if (0 == ioctl(slave, I_FIND, "ttcompat") && ioctl(slave, I_PUSH, "ttcompat") < 0) {
            pty_perror("ioctl(fd, I_PUSH, \"ttcompat\")");
            goto pty_error;
        }
#endif  /*sgi*/
    }
#endif  /*HAVE_STROPTS_H*/

#elif defined(HAVE_OPENPTY)
    if (openpty(&master, &slave, NULL, NULL, NULL) < 0) {
        pty_perror("openpty()");
        goto pty_error;
    }

#else   /*classic BSD style */
#define SYS_PTY_BSD
    if (NULL == sys_pty_bsd(&master, &slave, pts_name, sizeof(pts_name))) {
        goto pty_error;
    }

#endif

    /*
     *  terminal settings
     */
    sys_pty_mode(master);

#if defined(TIOCSWINSZ)
    if (lines > 0 || cols > 0) {                /* size window */
        struct winsize size = {0};

        size.ws_row = (unsigned short)(lines > 0 ? lines : 0);
        size.ws_col = (unsigned short)(cols > 0 ? cols : 0);
        (void) ioctl(master, TIOCSWINSZ, &size);
    }
#endif

    /*
     *   create sub-process
     */
#if defined(HAVE_VFORK) && defined(HAVE_WORKING_VFORK)
#define FORK vfork
#else
#define FORK fork                               /* XXX/AIX - f_fork? */
#endif

    if (-1 == (pid = FORK())) {
        pty_perror("fork");
        goto pty_error;
    }

    if (0 == pid) {
        pid = getpid();

        close(master);                          /* child closes master */

        if (setuid(getuid()) < 0) {
            fprintf(stderr, "could not set user identifier (pid=%d) : %d\n", pid, errno);
        }
        if (setgid(getgid()) < 0) {
            fprintf(stderr, "could not set group identifier (pid=%d) : %d\n", pid, errno);
        }
#if defined(HAVE_SETSID)
        if (setsid() < 0) {                     /* create a new session */
            fprintf(stderr, "could not set session leader (pid=%d) : %d\n", pid, errno);
        }

#else   /* classic method */
#if defined(TIOCSPGRP)
        ioctl(slave, TIOCSPGRP, &pid);
#endif
#if defined(HAVE_SETPGID)
        setpgid(pid, pid);
#elif defined(HAVE_SETPGRP)
        setpgrp();
#endif
#endif  /*HAVE_SETSID*/

        if (dup2(slave, STDIN_FILENO) < 0       /* redirect stdin, stdout, stderr to slave */
                || dup2(slave, STDOUT_FILENO) < 0
                || dup2(slave, STDERR_FILENO) < 0) {
            fprintf(stderr, "could not setup std in/out or err : %d)\n", errno);
            _exit(EXIT_FAILURE);
        }

        if (slave > STDERR_FILENO) {
            close(slave);
        }

#if defined(TIOCSCTTY)                          /* unset the controlling terminal */
        if (ioctl(STDIN_FILENO, TIOCSCTTY, 0)) {
            fprintf(stderr, "could not set controlling tty\n");
        }
#endif

        sys_pty_mode(STDIN_FILENO);             /* echo-off and raw-mode */
        if (lines > 0)
            gputenvi("LINES", lines);
        if (cols > 0)
            gputenvi("COLUMNS", cols);
        gputenv2("BPTY", pts_name);              /* programs can find us */

#if defined(SIGTTIN)
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
#endif
#if defined(SIGCLD)
        signal(SIGCLD, SIG_DFL);
#endif

#if defined(SYS_PTY_BSD)
        if (-1 == chown(pts_name, getuid(), getgid())) {
            fprintf(stderr, "warning: could not chown('%s') : %d (%s)\n", pts_name, errno, strerror(errno));
        }
        if (-1 == chmod(pts_name, 0622)) {
            fprintf(stderr, "warning: could not chmod('%s') : %d (%s)\n", pts_name, errno, strerror(errno));
        }
#endif

        if (cwd && -1 == chdir(cwd)) {
            fprintf(stderr, "could not set cwd '%s' : %s (%d)", cwd, str_error(errno), errno);
        }

#if defined(DOSISH)
        if (proc_shell_iscmd(shname)) {
            execlp(shell, shell, (char *) NULL);
        } else
#endif
            execlp(shell, shell, "-i", (char *) NULL);
        fprintf(stderr, "could not exec '%s' : %s (%d)\n", shell, str_error(errno), errno);
        _exit(EXIT_FAILURE);
    }

    /* parent process */
    close(slave);
    fcntl(master, F_SETFL, O_NONBLOCK);
    fcntl(master, F_SETFL, fcntl(master, F_GETFL, 0) | O_NDELAY);
    *recvfd = *sendfd = master;

    ED_TRACE(("==> tty:%s, pid:%d, fd:%d\n", pts_name, (int)pid, master))
    return pid;

pty_error:
    if (slave >= 0) close(slave);
    if (master >= 0) close(master);
    ED_TRACE(("==> pid:-1\n"))
    return -1;
}


static int
create_pipe(int *sendfd, int *recvfd, int lines, int cols, const char *shell, const char *cwd)
{
    int pipeto[2] = {-1, -1}, pipefrom[2] = {-1, -1};
    int pid = -1;

    ED_TRACE(("create_pipe()\n"))

    if (pipe(pipeto) < 0) {
        pty_perror("pipe(1)");

    } else if (pipe(pipefrom) < 0) {
        pty_perror("pipe(2)");
        close_pipe(pipeto, -1);

    } else {
#if defined(HAVE_SPAWN)
        if ((pid = pty_spawn(pipeto, pipefrom, shell, NULL)) >= 0) {
            *sendfd = pipeto[1]; *recvfd = pipefrom[0];
        } else {
            close_pipe(pipeto, -1);
            close_pipe(pipefrom, -1);
        }

#else
        if ((pid = fork()) < 0) {               /* error */
            pty_perror("fork");
            close_pipe(pipeto, -1);
            close_pipe(pipefrom, -1);

        } else if (0 == pid) {                  /* child */
            pid = getpid();

#if defined(HAVE_SETSID)
            if (setsid() < 0) {                 /* create a new session */
                fprintf(stderr, "could not set session leader (pid=%d) : %d\n", pid, errno);
            }
#else   /* classic method */
#if defined(TIOCSPGRP)
            ioctl(slave, TIOCSPGRP, &pid);
#endif
#if defined(HAVE_SETPGID)
            setpgid(pid, pid);
#elif defined(HAVE_SETPGRP)
            setpgrp();
#endif
#endif  /*HAVE_SETSID*/
                                                /* redirect stdin, stdout, stderr */
            if (dup2(pipeto[0], STDIN_FILENO) < 0
                    || dup2(pipefrom[1], STDOUT_FILENO) < 0
                    || dup2(pipefrom[1], STDERR_FILENO) < 0) {
                fprintf(stderr, "could not setup std in/out or err : %d)\n", errno);
                _exit(EXIT_FAILURE);
            }

            close_pipe(pipeto, STDERR_FILENO);
            close_pipe(pipefrom, STDERR_FILENO);

            sys_pty_mode(STDIN_FILENO);         /* echo-off and raw-mode */
            if (lines > 0) gputenvi("LINES", lines);
            if (cols > 0) gputenvi("COLUMNS", cols);
            gputenv("BPIPE=1");                 /* programs can find us */

            if (cwd && -1 == chdir(cwd)) {
                fprintf(stderr, "could not set cwd '%s' : %s (%d)", cwd, str_error(errno), errno);
            }

#if defined(DOSISH)
            if (proc_shell_iscmd(shell)) {
                execlp(shell, shell, (char *) NULL);
            } else
#endif
                execlp(shell, shell, "-i", (char *) NULL);
            fprintf(stderr, "could not exec '%s' : %s (%d)\n", shell, str_error(errno), errno);
            _exit(EXIT_FAILURE);

        } else {                                /* parent */
            close(pipeto[0]); pipeto[0] = -1;
            close(pipefrom[1]); pipefrom[1] = -1;

            *sendfd = pipeto[1];
            *recvfd = pipefrom[0];
        }
#endif
    }

    ED_TRACE(("==> pid:%d,sendfd:%d,recvfd:%d\n", pid, *sendfd, *recvfd))
    return pid;
}


#if defined(HAVE_SPAWN)
static int
pty_spawn(int *pipeto, int *pipefrom, const char *shell, const char *cwd)
{
    int saved_stdin, saved_stdout, saved_stderr;
    int pid = 1;

    saved_stdin = dup(STDIN_FILENO); sys_noinherit(saved_stdin);
    saved_stdout = dup(STDOUT_FILENO); sys_noinherit(saved_stdout);
    saved_stderr = dup(STDERR_FILENO); sys_noinherit(saved_stderr);

    if (dup2(pipeto[0], STDIN_FILENO) < 0       /* redirect stdin, stdout, stderr */
            || dup2(pipefrom[1], STDOUT_FILENO) < 0
            || dup2(pipefrom[1], STDERR_FILENO) < 0) {
        pty_perror("could not setup std in/out or err");
        return -1;
    }

    sys_noinherit(pipeto[0]);
    sys_noinherit(pipefrom[1]);

#if defined(DOSISH)
    if (proc_shell_iscmd(shell)) {
        pid = spawnlp(P_NOWAIT, shell, shell, (char *) NULL);
    } else
#endif
        pid = spawnlp(P_NOWAIT, shell, shell, "-i", (char *) NULL);

    if (pid < 0) {
        pty_perror("cannot spawn \"%s\"", shell);
    }

    dup2(saved_stdin, STDIN_FILENO); close(saved_stdin);
    dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout);
    dup2(saved_stderr, STDERR_FILENO); close(saved_stderr);

    if (pid < 0) {
        return -1;                              /* error */
    }

    close(pipeto[0]);
    close(pipefrom[1]);
    return pid;
}
#endif  /*HAVE_SPAWN*/


static void
close_pipe(int *fds, int check)
{
    if (fds) {
        if (fds[0] > check) close(fds[0]);
        if (fds[1] > check) close(fds[1]);
        fds[0] = fds[1] = -1;
    }
}


static void
pty_perror(const char *fmt, ...)
{
    const int xerrno = errno;
    char iobuf[512];
    va_list ap;

    va_start(ap, fmt);
    vsxprintf(iobuf, sizeof(iobuf), fmt, ap);
    ED_TRACE(("==> %s : %s (%d)\n", iobuf, str_error(xerrno), xerrno))
    ewprintf("connect: %s : %s (%d)", iobuf, str_error(xerrno), xerrno);
    va_end(ap);
}


/*  Function:           sys_pty_openpt
 *      Open psuedo tty
 *
 *   Parameters:
 *      none
 *
 *  Returns:
 *      File descriptor on success, otherwise -1.
 */
int
sys_pty_openpt(void)
{
    int fd = -1;

#if defined(HAVE_GETPT)
    /*
     *  GNU/libc
     */
    fd = getpt();

#elif defined(HAVE_POSIX_OPENPT)
    /*
     *  POSIX.1-2001
     */
    fd = posix_openpt(O_RDWR | O_NOCTTY);

#else
    /*
     *  Attempt to allocate a pty by opening the pty multiplex
     */
    static const char *multiplexers[] = {
            "/dev/ptmx",                        /* solaris/linux/bsd */
            "/dev/ptm",
            "/dev/ptc",                         /* aix */
            "/dev/ptym/clone",                  /* hpux */
            NULL
            };
    const char **ptmx;

    for (ptmx = multiplexers; *ptmx; ++ptmx) {
        if ((fd = open(*ptmx, O_RDWR | O_NOCTTY)) >= 0 || ENOENT != errno) {
            break;                              /* success or !ENOENT */
        }
    }
#endif

    return fd;
}


/*  Function:           sys_pty_unlockpt
 *      grantpt call for cloning pty implementation.
 *
 *      Change UID and GID of slave pty associated with master pty whose
 *      fd is provided, to the real UID and real GID of the calling thread.
 *
 *   Parameters:
 *      fd - File descriptor.
 *
 *  Returns:
 *      Zero on success, otherwise -1.
 */
int
sys_pty_grantpt(int fd)
{
#ifdef HAVE_GRANTPT
    return grantpt(fd);

#elif defined(TIOCPTYGRANT)
    return ioctl(fd, TIOCPTYGRANT);

#else
    return 0;
#endif
}


/*  Function:           sys_pty_unlockpt
 *      unlockpt implementation
 *
 *   Parameters:
 *      fd - File descriptor.
 *
 *  Returns:
 *      Zero on success, otherwise -1.
 */
int
sys_pty_unlockpt(int fd)
{
#if defined(HAVE_UNLOCKPT)
    return unlockpt(fd);

#elif defined(TIOCSPTLCK)
    int zero = 0;
    return ioctl(fd, TIOCSPTLCK, &zero);

#elif defined(TIOCPTYUNLK)
    return ioctl(fd, TIOCPTYUNLK);

#else
    return -1;
#endif
}


/*  Function:           sys_pty_ptsname
 *      ptsname call for cloning pty implementation.
 *
 *  Parameters:
 *      fd - File descriptor.
 *      buf - Buffer, should be >= 128 bytes.
 *      len - Length of buffer in bytes.
 *
 *  Returns:
 *      Psuedo tty name.
 */
const char *
sys_pty_ptsname(int fd, char *buf, int len)
{
    if (buf && len >= 128) {
#if defined(HAVE_PTSNAME_R) && defined(linux)
        if (0 == ptsname_r(fd, buf, len - 1)) {
            buf[len - 1] = 0;
            return buf;
        }

#elif defined (HAVE_PTSNAME)
        char *namebuf;

        if (NULL != (namebuf = ptsname(fd))) {
            const int namelen = strlen(namebuf);

            if (namelen < len) {
                strcpy(buf, namebuf);
                return buf;
            }
        }

#elif defined (TIOCGPTN)
        struct stat sb;
        int pty = 0;

        if (0 == ioctl(fd, TIOCGPTN, &pty)) {
            sxprintf(buf, len, "/dev/pts/%d", pty);
            if (0 != stat(buf, &sb)) {
                pty_perror("warning: stat(\"%s\")", buf);
            }
            return buf;
        }

#elif defined (TIOCPTYGNAME)
        struct stat sb;
        int ret;

        if (0 == ioctl(fd, TIOCPTYGNAME, buf)) {
            if (0 == stat(buf, &sb)) {
                return buf;
            }
        }

#else
        errno = ENOSYS;
        pty_perror("ptname(%d)", fd);

#endif
    }
    return NULL;
}


/*  Function:           sys_pty_bsd
 *      BSD style psuedo tty interface
 *
 *      The slave device file, which generally has a nomenclature of
 *      /dev/tty[p-za-e][0-9a-f], has the appearance and supported system calls of any
 *      text terminal. Thus it has the understanding of a login session and session
 *      leader process (which is typically the shell program).
 *
 *      The master device file, which generally has a nomenclature of
 *      /dev/pty[p-za-e][0-9a-f], is the endpoint for communication with the terminal
 *      emulator. It receives the control requests and information from the other party
 *      over this interface and responds accordingly.
 *
 *      With the typical [p-za-e] naming, there can be at most 256 tty pairs. Also,
 *      finding the first free pty master is not entirely race-free. For that reason,
 *      modern BSD operating systems, such as FreeBSD, implement Unix98 PTYs.[1]
 *
 *  Parameters:
 *      sendfd - send file descriptor.
 *      recvfd - recieve file descriptor.
 *      buf - Buffer address.
 *      len - Size of the buffer, in bytes.
 *
 *  Returns:
 *      Buffer address on sucess, otherwise NULL.
 */
const char *
sys_pty_bsd(int *sendfd, int *recvfd, char *buf, int len)
{
    int master = -1, slave = -1;
    char ptyname[256] = {0};
    struct stat statbuf;
    int major, minor;

    struct group *gptr;                         /* tty group look up */
    int gid;                                    /* tty group id */

    if ((gptr = getgrnam("tty")) != 0) {
        gid = gptr->gr_gid;
    } else {
        gid = -1;
    }

    for (major = 0; major < (int)sizeof(x_ptymajor); ++major) {

        pty_bsd_name(ptyname, major, 0);
        if (stat(ptyname, &statbuf) < 0) {
            break;                              /* out of range */
        }

        for (minor = 0; minor < (int)sizeof(x_ptyminor); ++minor) {
            /*
             *  /dev/tty[p-za-e][0-9a-f]
             */
            pty_bsd_name(ptyname, major, minor);
            master = open(ptyname, O_RDWR);

            if (master >= 0) {
                char tty_name[256] = {0};

                pty_tty_name(tty_name, major, minor);
                if ((slave = open(ptyname, O_RDWR)) < 0) {
                    goto pipe_error;
                }

                if (chown(tty_name, getuid(), gid) < 0 ||
                        chmod(tty_name, S_IRUSR|S_IWUSR|S_IWGRP) < 0) {
                    goto pipe_error;
                }

                *sendfd = master;
                *recvfd = slave;
                strxcpy(buf, tty_name, len);
                return buf;
            }
        }
    }

pipe_error:
    if (slave >= 0)
        close(slave);
    if (master >= 0)
        close(master);
    *sendfd = -1;
    *recvfd = -1;
    return NULL;
}


static const char *
pty_bsd_name(char *name, int major, int minor)
{
#if defined(__hpux) || defined(hpux)
    if (0 == name[0]) {
        strcpy(name, "/dev/ptym/ptyXY");
    }
    name[13] = x_ptymajor[major];
    name[14] = x_ptyminor[minor];

#else
    if (0 == name[0]) {
        strcpy(name, "/dev/ptyXY");
    }
    name[8] = x_ptymajor[major];
    name[9] = x_ptyminor[minor];

#endif
    return name;
}


static const char *
pty_tty_name(char *name, int major, int minor)
{
#if defined(__hpux) || defined(hpux)
    if (0 == name[0]) {
        strcpy(name, "/dev/pty/ttyXY");
    }
    name[12] = x_ptymajor[major];
    name[13] = x_ptyminor[minor];

#else
    if (0 == name[0]) {
        strcpy(name, "/dev/ttyXY");
    }
    name[8] = x_ptymajor[major];
    name[9] = x_ptyminor[minor];

#endif
    return name;
}

#endif  /*unix*/
