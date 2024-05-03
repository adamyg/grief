#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_unix_c,"$Id: sys_unix.c,v 1.65 2024/04/27 15:22:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_unix.c,v 1.65 2024/04/27 15:22:03 cvsuser Exp $
 * System dependent functionality - UNIX.
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
#include <edtermio.h>
#if defined(HAVE_SYS_PRCTL_H)
#include <sys/prctl.h>                          /* prctl(), linux */
#endif
#if defined(linux) || defined(HAVE_SYS_SENDFILE_H)
#include <sys/sendfile.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>                           /* timeval */
#endif
#if defined(HAVE_SYS_RESOURCE_H)
#include <sys/resource.h>
#endif
#include <time.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edalt.h>                              /* MOUSE_KEY */
#include <libstr.h>                             /* str_...()/sxprintf() */

#if defined(__CYGWIN__)
#define  WINDOWS_MEAN_AND_LEAN
#include <w32api/windows.h>
#endif
#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "main.h"
#include "keyboard.h"
#include "system.h"                             /* sys_...() */
#include "tty.h"

#include "edstacktrace.h"
#include "m_pty.h"

#if !defined(_VMS) && !defined(__OS2__) && !defined(__MSDOS__)
#if defined(HAVE_MOUSE)
#if defined(HAVE_LIBGPM) && defined(HAVE_GPM_H)
#include <gpm.h>
#endif
#include "mouse.h"
#endif

static TERMIO           ot, nt;                 /* saved and current tty settings */

static int              ofdflags = 0;           /* saved keyboard fd flags. */
static int              nfdflags;               /* used to optimise calls to fcntl(). */

#if defined(M_VGA_C80x25)
static int              console_mode = -1;      /* only used on EGA screen. */
#endif

static int              ttyactivep = FALSE;

static int              blocking(int fd);
static int              nonblocking(int fd);

#if defined(HAVE_MOUSE)
    /*
     *  mouse support
     */
#define MAX_X               1000                /* max resolution (X11) */
#define MAX_Y               300

#define MOUSE_NONE          0x00                /* mouse types */
#define MOUSE_GPM           0x01
#define MOUSE_X11           0x02
#define MOUSE_XTERM         0x03

static const char *     mouse_dev   = NULL;     /* mouse device name/type */
static int              mouse_fd    = -1;       /* fd resource, if any */
static int              mouse_type  = MOUSE_NONE;
static int              mouse_oldx  = 0;        /* old co-ords. */
static int              mouse_oldy  = 0;

#endif /*HAVE_MOUSE*/


/*
 *  sys_initialise ---
 *      This function is used to set up the terminal modes, ie. no echo, single
 *      character input mode.
 *
 *      Unfortunately with the advent of POSIX we have about 4 or 5 different ways we
 *      COULD do things. Best thing is to move to the POSIX style and emulate on the
 *      other systems, but the Sun is a disaster area because it supports ALL types.
 *      This makes it very difficult to verify things properly.
 */
void
sys_initialise(void)
{
    /* If terminal already open then don't do anything */
    if (ttyactivep) {
        return;
    }

#if defined(__CYGWIN__)
    /*
     *  2002/9/21 - source ncurses
     *
     *      Work around a bug in Cygwin.  Full-screen subprocesses run from bash,
     *      in turn spawned from another full-screen process will dump core when
     *      attempting to write to stdout.  Opening /dev/tty explicitly seems to
     *      fix the problem  -- still needed??.
     */
    if (isatty(fileno(stdout))) {
        FILE *fp;

        if (NULL != (fp = fopen("/dev/tty", "w"))) {
            if (isatty(fileno(fp))) {
                fclose(stdout);
                dup2(fileno(fp), STDOUT_FILENO);
                stdout = fdopen(STDOUT_FILENO, "w");
            }
            fclose(fp);
        }
    }
#endif  /*__CYGWIN__*/

#if defined(M_VGA_C80x25) && defined(CONS_GET)
    {                                           /* old school 43 line mode */
        static int old_mode = -1;
        int val;
        if (-1 == old_mode) {
            old_mode = console_mode = ioctl(0, CONS_GET, &val);
        } else {
            tty_egaflag = -1;
        }
    }
#endif  /*M_VGA_C80x25 && CONS_GET*/

    /*
     *  Get current settings and save them
     */
#if defined(HAVE_TERMIOS)
    tcgetattr(0, &ot);
#else
    ioctl(0, TCGETA, &ot);
#endif
    nt = ot;

    /*
     *  Set the modes we're interested in
     */
#if defined(HAVE_TERMIO) || defined(HAVE_TERMIOS)
    /*
     *  Note that ^S/^Q and ^Z will be handled by system and not seen by Grief.
     *  If you want to see these characters you'll have to do input_mode() flow control.
     */
#if !defined(NULL_VALUE)
#define NULL_VALUE  0xff
#endif

 /* nt.c_cc[VEOF]   = 1;                        -* EOF marker ^D */
    nt.c_cc[VMIN]   = 1;                        /* one character read is OK. */
    nt.c_cc[VTIME]  = 0;                        /* Never time out. */

    nt.c_cc[VINTR]  = 'Y' & 0x1f;               /* Default interrupt key. */
    nt.c_cc[VQUIT]  = NULL_VALUE;
 /* nt.c_cc[VSUSP]  = NULL_VALUE;               -* to ignore ^Z */
#if defined(VDSUSP)
    nt.c_cc[VDSUSP] = NULL_VALUE;               /* to ignore ^Y */
#endif
#if defined(VLNEXT)
    nt.c_cc[VLNEXT] = NULL_VALUE;               /* to ignore ^V */
#endif
#if defined(VSWTCH)
    nt.c_cc[VSWTCH] = NULL_VALUE;
#endif

    nt.c_iflag |= IGNBRK;                       /* ignore BREAK condition */
    nt.c_iflag &= ~ICRNL;                       /* disable CR to NL on input */
    nt.c_iflag &= ~INLCR;                       /* disable map CR to NL on input */
    nt.c_iflag &= ~ISTRIP;                      /* dont strip 8th bit */
 /* nt.c_iflag &= ~IXOFF;                       -* disable stop/start input flow control */
 /* nt.c_iflag &= ~IXON;                        -* disable stop/start output flow control */

    nt.c_oflag &= ~OPOST;                       /* disable output processing. */

    nt.c_cflag |= CS8;                          /* allow 8th bit on input. */
    nt.c_cflag &= ~PARENB;                      /* don't check parity. */

    nt.c_lflag &= ~ECHO;                        /* disable local echo. */
    nt.c_lflag &= ~ICANON;                      /* disable canonical input. */
#if defined(IEXTEN)
    nt.c_lflag &= ~IEXTEN;                      /* disable extended input char processing. */
#endif
#else
    nt.sg_flags |= CBREAK | PASS8;
    nt.sg_flags &= ~(ECHO | CRMOD);
#endif  /*HAVE_TERMIO || HAVE_TERMIOS*/

    ofdflags = fcntl(0, F_GETFL, 0);            /* stdin flags */
    nfdflags = ofdflags;

#if defined(HAVE_TERMIOS)
 /* tcsetattr(0, TCSANOW, &nt);                 -* update terminal */
    tcsetattr(0, TCSADRAIN, &nt);
#else
    ioctl(0, TCSETA, &nt);
#endif

#if defined(TIOCLGET) && defined(LPASS8)
    {
        int flags;

        ioctl(0, TIOCLGET, &flags);
        flags |= LPASS8;
        ioctl(0, TIOCLSET, &flags);
    }
#endif
    ttyactivep = TRUE;
}


/*
 *  sys_shutdown ---
 *      This function gets called just before we go back home to the shell.
 *
 *      Put all of the terminal parameters back.
 */
void
sys_shutdown(void)
{
    if (! ttyactivep)
        return;

#if defined(HAVE_TERMIOS)
 /* tcsetattr(0, TCSANOW, &ot);                 -* restore stream */
    tcsetattr(0, TCSADRAIN, &ot);
#else
    ioctl(0, TCSETA, &ot);
#endif

    fcntl(0, F_SETFL, ofdflags);
    nfdflags = ofdflags;

#if defined(M_VGA_C80x25)
    if (console_mode) {
        ioctl(1, _IO('S', console_mode), 0);
    }
#endif

    ttyactivep = FALSE;
}


/*
 *  sys_enable_char ---
 *      Routine called by input_mode() primitive to enable certain keys to be seen
 *      by Grief, e.g. XON/XOFF, ^Z.
 */
int
sys_enable_char(int ch, int enable)
{
    int ret = 0;

    if (! ttyactivep)
        return 0;

#if defined(HAVE_TERMIO) || defined(HAVE_TERMIOS)
    switch (ch) {
    case 's' & 0x1f:
    case 'q' & 0x1f:
        ret = (nt.c_iflag & IXON) == 0;
        if (enable) {
            nt.c_iflag &= ~IXON;
        } else {
            nt.c_iflag |= IXON;
        }
#if defined(HAVE_TERMIOS)
        tcsetattr(0, TCSANOW, &nt);
#else
        ioctl(0, TCSETA, &nt);
#endif
        break;

    case 'z' & 0x1f:
        ret = (nt.c_lflag & ISIG) == 0;
        if (enable) {
            nt.c_lflag &= ~ISIG;
        } else {
            nt.c_lflag |= ISIG;
        }
#if defined(HAVE_TERMIOS)
        tcsetattr(0, TCSANOW, &nt);
#else
        ioctl(0, TCSETA, &nt);
#endif
        break;
    }
#endif
    return ret;
}


void
sys_timeout(int yes)
{
    if (! ttyactivep)
        return;

#if defined(SYSV) || defined(HAVE_TERMIOS)
    if (yes) {
        nt.c_cc[VTIME] = 5;                     /* x0.1 seconds */
        nt.c_cc[VMIN] = 0;
    } else {
        nt.c_cc[VTIME] = 0;
        nt.c_cc[VMIN] = 1;
    }
#if defined(HAVE_TERMIOS)
    tcsetattr(0, TCSANOW, &nt);
#else
    ioctl(0, TCSETA, &nt);
#endif
#endif
}


/*  Function:           sys_iocheck
 *      Check for an event input event.
 *
 *  Parameters:
 *      None
 *
 *  Returns:
 *      *true* or *false*.
 */
int
sys_iocheck(struct IOEvent *evt)
{
    if (ttyactivep) {
        unsigned char ch = 0;

        sys_tty_delay(TTY_INFD, 0);
        if (1 == read(TTY_INFD, (char *)&ch, 1)) {
            evt->type = EVT_KEYRAW;             /* uncooked key */
            evt->code = ch;
            return TRUE;
        }
    }
    return FALSE;
}


/*
 *  On System V using the VMIN/VTIME facility, there is a bug which causes the internal
 *  timeout not to be cleared when a key is pressed. Subsequently on the next timeout,
 *  we may delay a lot longer than we would really like.
 */
#if defined(SYSV)
static int                      alarm_handled = 0;

static void
getkey_alarm_handler(int sig)
{
    alarm_handled = sig;
}


/*  Function:           sys_getchar
 *      Retrieve the character from the status keyboard stream, within
 *      the specified timeout 'tmo'.
 *
 *  Parameters:
 *      fd -                File descriptor.
 *      buf -               Output buffer.
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      On success (1), otherwise (0) unless a timeout (-1).
 */
int
sys_getchar(int fd, int *ibuf, accint_t tmo)
{
    accint_t delta = 0;
    unsigned char buf[1];
    int n;

#define MAX_SYSV_TIMEOUT        (127 * 100)

    sys_tty_delay(TTY_INFD, 1);
    while (1) {
        if (! xf_usevmin) {
            if (tmo) {
                signal(SIGALRM, getkey_alarm_handler);
                n = (tmo < EVT_SECOND(1)) ? 1 : tmo / EVT_SECOND(1);
                alarm(n);
            }
            n = read(TTY_INFD, (char *) buf, 1);
            tmo = 0;
            alarm(0);

        } else {
            if (tmo == 0) {
                nt.c_cc[VMIN] = 1;
                nt.c_cc[VTIME] = 0;

            } else if (tmo < MAX_SYSV_TIMEOUT) {
                delta = tmo;
                nt.c_cc[VTIME] = tmo / 100;
                nt.c_cc[VMIN] = 0;

            } else {
                delta = MAX_SYSV_TIMEOUT;
                nt.c_cc[VTIME] = 127;
                nt.c_cc[VMIN] = 0;
            }
#if defined(HAVE_TERMIOS)
            tcsetattr(TTY_INFD, TCSANOW, &nt);
#else
            ioctl(TTY_INFD, TCSETA, &nt);
#endif
            n = read(TTY_INFD, (char *) buf, 1);
        }
        *ibuf = buf[0];
        if (n == 1) {
            return 1;
        }
        tmo -= delta;
        if (tmo <= 0) {
            return -1;
        }
    }
    return 0;
}
#endif  /*SYSV*/


/*  Function:           sys_cut
 *      Cut the current marked region to the 'system' clipboard/scrap.
 *
 *  Parameters:
 *      total -             xxx
 *      append -            xxx
 *      copy -              xxx
 *
 *  Returns;
 *      -1 as the service is not supported.
 */
int
sys_cut(int total, int append, void (*copy)(char *buf, int total))
{
    __CUNUSED(total)
    __CUNUSED(append)
    __CUNUSED(copy)
    return -1;
}


/*  Function:           sys_paste
 *      Paste the 'system' clipboard/scrap into the current buffer.
 *
 *  Parameters:
 *      paste -             Paste callback.
 *
 *  Returns:
 *      -1 as the service is not supported.
 */
int
sys_paste(void (*paste)(const char *, int))
{
    __CUNUSED(paste)
    return -1;
}


const char *
sys_delim(void)
{
    return "/";
}


const char *
sys_getshell(void)
{
    const char *shname;

    shname = ggetenv("shell");
    if (NULL == shname) {
        shname = ggetenv("SHELL");
    }
#if defined(__CYGWIN__)
    if (NULL == shname && 0 == access("/bin/bash", F_OK)) {
        shname = "/bin/bash";
    }
#endif
    if (NULL == shname) {
        shname = "/bin/sh";
    }
    return shname;
}


/*
 *  sys_tty_delay ---
 *      Control file delay (O_NDELAY) status only if not already set
 */
void
sys_tty_delay(int fd, int state)
{
    if (TTY_INFD == fd) {
        if (ttyactivep) {
            if (0 == state) {
                if (0 == (nfdflags & O_NDELAY)) {
                    nfdflags |= O_NDELAY;
                    fcntl(fd, F_SETFL, nfdflags);
                }
            } else {
                if (nfdflags & O_NDELAY) {
                    nfdflags &= ~O_NDELAY;
                    fcntl(fd, F_SETFL, nfdflags);
                }
            }
        }
    } else {
        if (0 == state) {
        } else {
        }
    }
}


/*
 *  sys_noinherit ---
 *      Set flag to indicate file descriptor should be closed on an exec.
 */
void
sys_noinherit(int fd)
{
#if defined(FIOCLEX)
    fcntl(fd, FIOCLEX, 0);
#endif
}


/*
 *  sys_deamonize ---
 *      turn this process into a daemon.
 */
static __CINLINE void
errquit(const char *loc, int daemon)
{
    extern const char *x_progname;

    fprintf(stderr, "%s - %s: %s\n", x_progname, loc, strerror(errno));
    fflush(NULL);                               /* flush all output buffers */
    exit(EXIT_SUCCESS);
}


void
sys_deamonize(void)
{
    pid_t pid = -1;
    struct rlimit fplim = {0};
    int fd, fd0, fd1, fd2;

    umask(0);                                   /* clear creation mask */

    if ((pid = fork()) < 0) {
        errquit("fork in daemonize", 0);
    } else if (pid != 0) {                      /* parent */
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {                         /* become a session leader, lose controlling terminal */
        errquit("setsid", 0);
    }

//  if (chdir("/") < 0) errquit("chdir", 0);    /* change working directory */

    if (getrlimit(RLIMIT_NOFILE, &fplim) < 0) {
        errquit("getrlimit", 0);
    }

    for (fd = 0; fd < fplim.rlim_max; ++fd) {
        close(fd);                              /* close all open files */
    }

    /* open stdin, stdout, stderr to /dev/null */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        errquit("unexpected file descriptors", 1);
    }
}


/*
 *  blocking ---
 *      Set blocking flags on the specified descriptor
 */
static int
blocking(int fd)
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0 ||
            (val & O_NONBLOCK) == 0) {
        return 0;
    }
    val &= ~O_NONBLOCK;
    (void) fcntl(fd, F_SETFL, val);
    return 1;
}


/*
 *  blocking ---
 *      Set nonblocking flags on the specified descriptor.
 */
static int
nonblocking(int fd)
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) < 0 ||
            (val & O_NONBLOCK) == O_NONBLOCK) {
        return 0;
    }
    val |= O_NONBLOCK;
    (void) fcntl(fd, F_SETFL, val);
    return 1;
}


/*
 *  sys_cwd ---
 *      Current working directory
 */
const char *
sys_cwd(char *buffer, int length)
{
    if (0 == getcwd(buffer, length)) {
        buffer[0] = 0;
    }
    return buffer;
}


/*
 *  sys_read ---
 *      Read from a file descriptor.
 */
int
sys_read(int fd, void *buf, int size)
{
    char *data = (char *)buf;
    int n, osize = size;

    while (size > 0) {
        if ((n = read(fd, data, size)) <= 0) {
            if (n < 0 && data == (void *)buf) {
                return -1;
            }
            break;
        }
        size -= n;
        data += n;
    }
    return (osize - size);
}


/*
 *  sys_write ---
 *      Write to a file descriptor.
 */
int
sys_write(int fd, const void *buf, int size)
{
    const int nblock = blocking(fd);
    const char *data = (const char *)buf;
    int n, cnt = 0;

    do {                                        /* EINTR safe */
        if ((n = write(fd, data, size)) >= 0) {
            data += n, cnt += n;
       if ((size -= n) <= 0) {
                if (nblock) {
                    nonblocking(fd);
                }
                return cnt;                     /* success */
       }
   }
    } while (n >= 0 || (n < 0 && errno == EINTR));
    if (nblock) {
        nonblocking(fd);
    }
    return -1;
}


/*
 *  sys_copy ---
 *      Copy a file.
 */
int
sys_copy(const char *src, const char *dst, int perms, int owner, int group)
{
#if defined(linux) || \
        (defined(HAVE_SENDFILE) && defined(HAVE_SYS_SENDFILE_H))
    struct stat sb = {0};
    int ifd, ofd;
    int ret = -1;

    if ((ifd = open(src, OPEN_R_BINARY | O_RDONLY)) >= 0 && 0 == fstat(ifd, &sb)) {

        if ((ofd = open(dst, OPEN_W_BINARY | O_WRONLY | O_CREAT | O_TRUNC, 0666)) >= 0) {
            off_t off = 0, n;

            ret = 0;                            /* kernel copy */
            if (sb.st_size == (n = sendfile(ofd, ifd, &off, sb.st_size))) {
                ret = 1;

            } else {                            /* basic copy */
                if (-1 == n && (EINVAL == errno || ENOSYS == errno)) {
                    char iobuf[BUFSIZ];

                    ret = 1;
                    while ((n = sys_read(ifd, iobuf, sizeof(iobuf))) > 0) {
                        if (n != sys_write(ofd, iobuf, n)) {
                            ret = 0;
                            break;
                        }
                    }
                } else {
                    ret = 0;
                }
            }

            if (1 == ret) {                     /* success */
                (void) chmod(dst, perms);
                if (-1 == chown(dst, owner, group)) {
                    ewprintf("warning: unable to chown(%s) : %d", dst, errno);
                }
            } else {
                ewprintf("warning: unable to copy(%s -> %s) : %d", dst, src, errno);
            }
            close(ofd);
            if (1 != ret) {
                unlink(dst);
            }
        }
        close(ifd);

    } else if (ENOENT == errno) {
        ret = 1;                                /* success */

    }
    return ret;

#else
    __CUNUSED(src)
    __CUNUSED(dst)
    __CUNUSED(perms)
    __CUNUSED(owner)
    __CUNUSED(group)
    return -1;                                  /* default */
#endif
}


/*  Function:           sys_xxx
 *      System i/o primitives.
 */
int
sys_mkdir(const char *path, int amode)
{
    return mkdir(path, amode);
}


int
sys_access(const char *path, int amode)
{
    return access(path, amode);
}


int
sys_chmod(const char *path, int mode)
{
    return chmod(path, mode);
}


int
sys_realpath(const char *name, char *buf, int size)
{
    if (size < PATH_MAX) {
        char t_realpath[PATH_MAX + 1];
        int reallen;

        if (NULL == realpath(name, t_realpath)) {
            return -1;
        }
        if ((reallen = strlen(t_realpath)) >= size) {
            if (buf && size > 1) {
                memcpy(buf, (const char *)t_realpath, size);
                buf[size-1] = 0;
            }
            errno = ENOMEM;
            return -1;
        }
        memcpy(buf, (const char *)t_realpath, reallen + 1);
        return 0;
    }
    return (NULL == realpath(name, buf) ? -1 : 0);
}


int
sys_stat(const char *path, struct stat *sb)
{
    return stat(path, sb);
}


int
sys_lstat(const char *path, struct stat *sb)
{
    return lstat(path, sb);
}


int
sys_readlink(const char *path, char *buf, int maxlen)
{
    return readlink(path, buf, maxlen);
}


int
sys_symlink(const char *name1, const char *name2)
{
    return symlink(name1, name2);
}


int
sys_unlink(const char *fname)
{
    return unlink(fname);
}


/*
 *  sys_time ---
 *      High resolution time, seconds plus milliseconds.
 */
time_t
sys_time(int *msec)
{
    struct timeval tv;

    (void) gettimeofday(&tv, NULL);
    if (msec) {
        *msec = (int)(tv.tv_usec / 1000);
    }
    return tv.tv_sec;
}


void
sys_cleanup(void)
{
}


/*
 *  sys_pty_mode ---
 *      Function called from PTY code to set up terminal modes when we create
 *      a new shell buffer. We put it here so we can ensure that when
 *      compiling with GNU C that we can turn on -traditional for this file
 *      (older ones with ioctl problems).
 */
void
sys_pty_mode(int fd)
{
#if !defined(ICANON)
    (void) fd;

#else
    TERMIO t;

#if defined(HAVE_TERMIOS)
    if (tcgetattr(fd, &t) >= 0)
#else
    if (ioctl(fd, TCGETA, &t) >= 0)
#endif
    {
        t.c_lflag &= ~(ECHO);                   /* echo off */
        t.c_lflag |= ICANON;                    /* uncooked */
#if defined(HAVE_TERMIOS)
        tcsetattr(fd, TCSANOW, &t);
#else
        ioctl(fd, TCSETA, &t);
#endif
    }
#endif   /*ICANON*/
}


int
sys_mkstemp(char *temp)
{
#if defined(HAVE_MKSTEMP) || \
        defined(linux) || defined(sun) || defined(_AIX)
    return mkstemp(temp);

#else
    temp = mktemp(temp);
    if (temp && temp[0]) {
        return open(temp, O_CREAT|O_EXCL|O_RDWR, 0600);
    }
    return -1;
#endif
}


/*  Function:           sys_getpid
 *      Retrieve the current process identifier.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Current process identifier;
 */
int
sys_getpid(void)
{
    return getpid();
}


int
sys_getuid(void)
{
    return getuid();
}


int
sys_geteuid(void)
{
    return geteuid();
}


/*  Function:           sys_core
 *      Generate a core/system stack for the current process.
 *
 *  Parameters:
 *      msg -               Optional user message.
 *      path -              Optional output path.
 *      fname -             Optional file-name prefix.
 *
 *  Returns:
 *      Returns 0 on success, otherwise -1;
 */
#if defined(HAVE_PRCTL) && defined(PR_SET_PTRACER)
#define HAVE_PRCTL_SYSTEM
static int prctl_system(const char *command);   /*see prctl() for more details*/
#endif

int
sys_core(const char *msg, const char *path, const char *fname)
{
#if defined(__CYGWIN__)
    const DWORD pid = GetCurrentProcessId();
    FILE *out = NULL;
#else
    const int pid = (int) getpid();
#endif
    char outbase[512], cmd[1024];
#if defined(PR_SET_PTRACER)
    int child, status;
#endif
    int cmdlen;

    outbase[0] = 0;
    strxcpy(outbase, (path ? path : "."), sizeof(outbase));
    if (outbase[0]) strxcat(outbase, "/", sizeof(outbase));
    if (fname) {
        strxcat(outbase, fname, sizeof(outbase));
        strxcat(outbase, ".", sizeof(outbase));
    }

#if defined(__CYGWIN__)
    /*
     *  stackdump (if available)
     *  /usr/bin/dumper [OPTION] <filename> <win32pid>
     *
     *      gr.<pid>.stackdump
     *      gr.<pid>.core
     *
     *  Note: It turns out that dumper will work with a Windows process, but only 32-bit processes.
     */
    sxprintf(cmd, sizeof(cmd), "gr.%u.stackdump", (unsigned)pid);
    if (NULL != (out = fopen(cmd, "w"))) {
        edbt_stackdump(out, 2);
        fclose(out);
    }
    cmdlen = sxprintf(cmd, sizeof(cmd), "/usr/bin/dumper --quiet %sgr.%u %u\n",
                outbase, (unsigned)pid, (unsigned)pid);

#elif defined(_AIX)
    /*
     *  procstack <pid>
     *  gencore <pid> <outfile>
     */
    cmdlen = sxprintf(cmd, sizeof(cmd), "procstack %d > %spstack.%d 2>&1; gencore %d %score.%d\n",
                pid, outbase, pid, pid, outbase, pid);

#else   /*BSD/Sun/HPUx/APPLE */
    /*
     *  [sleep 4]
     *  pstack <pid> > <file> 2>&1
     *  gcore -o <file> <pid>
     *
     *      gr.<pid>.stackdump
     *      gr.<pid>.core
     *
     *  Note, generally all optmisation should be disabled for pstack and gcore to
     *      correctly report the active callback trace.
     */
#if defined(HAVE_PRCTL_SYSTEM)
    cmdlen = sxprintf(cmd, sizeof(cmd), "sleep 4; pstack %d > %sgr.pstack.%d 2>&1; gcore -o %sgr.core %d\n",
                pid, outbase, pid, outbase, pid);
#else
    cmdlen = sxprintf(cmd, sizeof(cmd), "pstack %d > %sgr.pstack.%d 2>&1; gcore -o %sgr.core %d\n",
                pid, outbase, pid, outbase, pid);
#endif
#endif

    if (msg) {
        const int errout = fileno(stderr);
        if (errout >= 0) {
            char alert[256];
            int ret;

            ret = write(errout, alert, sxprintf(alert, sizeof(alert), "%s, pid:%d, running: ", msg, pid));
            ret = write(errout, cmd, cmdlen);
            __CUNUSED(ret)
        }
    }
    cmd[cmdlen - 1] = 0;                        /* remove \n */

#if defined(HAVE_PRCTL_SYSTEM)
    return prctl_system(cmd);
#else
    return system(cmd);
#endif
}


#if defined(HAVE_PRCTL_SYSTEM)
static int
prctl_system(const char *command)
{
    char * newenviron[] = {NULL};
    char * argp[] = {"sh", "-c", NULL, NULL};
    int dumpable;
    sig_t intsave, quitsave;
    sigset_t mask, omask;
    pid_t childpid;
    int pstat;

    if (NULL == (argp[2] = (char*)command)) {
        return 1;
    }
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    dumpable = prctl(PR_GET_DUMPABLE, 0, 0, 0, 0);
    if (!dumpable) prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
#if defined(HAVE_WORKING_VFORK)
    switch (childpid = vfork()) {
#else
    switch (childpid = fork()) {
#endif
    case -1:            /* error */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        return -1;
    case 0:             /* child */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        execve("/bin/sh", argp, newenviron);
        _exit(127);
    }
    prctl(PR_SET_PTRACER, childpid, 0, 0, 0);
    intsave = signal(SIGINT, SIG_IGN);
    quitsave = signal(SIGQUIT, SIG_IGN);
    childpid = waitpid(childpid, (int *)&pstat, 0);
    sigprocmask(SIG_SETMASK, &omask, NULL);
    (void)signal(SIGINT, intsave);
    (void)signal(SIGQUIT, quitsave);
    if (!dumpable) prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
    prctl(PR_SET_PTRACER, getppid(), 0, 0, 0);
    return (-1 == childpid ? -1 : pstat);
}
#endif  /*HAVE_PRCTL_SYSTEM*/


void
sys_abort(void)
{
    signal(SIGABRT, SIG_DFL);
    abort();
    /*NOTREACHED*/
}


/*  Function:           sys_signal
 *      Signal handler installation.
 *
 *  Parameters:
 *      XXX -
 *
 *  Returns:
 *      Returns 0 on success, otherwise -1;
 */
signal_handler_t
sys_signal(int sig, signal_handler_t func)
{
    return signal(sig, func);
}


/*  Function:           sys_running
 *      Determine if given process is still running.
 *
 *  Parameters:
 *      pid -               Process identifier.
 *
 *  Returns:
 *      Returns 1 if the processing is still running, 0 if not otherwise -1 on an error condition.
 */
int
sys_running(int pid)
{
    if (-1 == kill(pid, 0)) {
        if (errno == ESRCH)
            return 0;                           /* no */
        return -1;
    }
    return 1;
}



#if defined(HAVE_MOUSE)
/*
 *  sys_mouseinit ---
 *      Initialise the mouse interface
 */
int
sys_mouseinit(const char *dev)
{
    const char *bmouse = NULL;
    int xterm = 0;

    /*
     *  Select the mouse device.
     */
    if (NULL != (bmouse = ggetenv("BMOUSE")) && !*bmouse) {
        bmouse = NULL;                          /* override */
    }

    if (NULL == bmouse && x_pt.pt_mouse[0]) {
        bmouse = x_pt.pt_mouse;                 /* term feature */
    }

    if (NULL == dev || '\0' == *dev) {
        if (NULL == (dev = bmouse))  {          /* BMOUSE or auto-configure */
            static const struct {
                unsigned    len;
                const char *desc;
            } xtermlike[] = {
#define XTERMLIKE(x)            { sizeof(x)-1, x }
                XTERMLIKE("xterm"),
                XTERMLIKE("rxvt"),
                XTERMLIKE("Eterm"),
                XTERMLIKE("dtterm"),
                XTERMLIKE("cygwin")
                };
#undef XTERMLIKE

            const char *term = ggetenv("TERM");

            if (term) {
                unsigned i;
                                                /* e.g. xterm-color */
                for (i = 0; i < (sizeof(xtermlike)/sizeof(xtermlike[0])); ++i)
                    if (strncmp(term, xtermlike[i].desc, xtermlike[i].len) == 0 &&
                            ('\0' == term[xtermlike[i].len] || '-' == term[xtermlike[i].len])) {
                        xterm = 1;              /* known xterm, force xterm */
                        break;
                    }
            }
        }
    }

    /*
     *  Open the device.
     */
    if (dev && 0 == strcmp(dev, "gpm")) {
        /*
         *  GPM driver
         */
#if defined(HAVE_LIBGPM) && defined(HAVE_GPM_H)
        Gpm_Connect conn;

        (void) memset(&conn, 0, sizeof(conn));
        conn.eventMask = ~0;                    /* ~GPM_MOVE */
        conn.defaultMask = 0;                   /* GPM_MOVE */
        conn.minMod = 0;
        conn.maxMod = 0;

        if ((mouse_fd = Gpm_Open(&conn, 0)) >= 0) {
            io_device_add(gpm_fd);
            mouse_type = MOUSE_GPM;

        } else if (-2 == mouse_fd) {            /* running under xterm */
         /* ttpush("\033[?1000h"); */
            key_define_key_seq(MOUSE_KEY, "\x1b[M");
            mouse_type = MOUSE_XTERM;
            mouse_fd = -1;
        }
#endif  /*HAVE_LIBGPM*/

    } else if (xterm || (dev && 0 == strcmp(dev, "xterm"))) {
        /*
         *  xterm
         *    o 1001 not supported under Cygwin, causes terminal state corruption.
         */
#if !defined(__CYGWIN__)
        ttpush("\033[?1001s");                  /* save old highlight mouse tracking */
#endif
        ttpush("\033[?1000h");                  /* enable mouse tracking */
        ttflush();

        key_define_key_seq(MOUSE_KEY, "\x1b[M");
        mouse_type = MOUSE_XTERM;
        dev = "xterm";

    } else {                                    /* X11 */
        /*
         *  X11 mouse --
         *      if open fails, then no mouse support is available.
         *
         *      Fail quietly so we can use same aliases with or without mouse support.
         */
        TERMIO mouse_term;

        dev = (bmouse ? bmouse : "/dev/tty00"); /* /dev/tty00 as default */
        mouse_fd = open(dev, O_RDONLY | O_EXCL);
        if (mouse_fd != -1) {
            /* Make mouse device into raw mode, etc */
#if defined(HAVE_TERMIOS_H)
            tcgetattr(mouse_fd, &mouse_term);
#else
            ioctl(mouse_fd, TCGETA, &mouse_term);
#endif
            mouse_term.c_lflag &= ~(ICANON | ECHO);
            mouse_term.c_iflag &= ~ICRNL;
            mouse_term.c_oflag = 0;
#if defined(CBAUD)
            mouse_term.c_cflag &= ~(PARENB | CBAUD);
            mouse_term.c_cflag |= (CS8 | B1200);
#else
            mouse_term.c_cflag &= ~PARENB;
            mouse_term.c_cflag |= CS8;
            mouse_term.c_ispeed = mouse_term.c_ospeed = B1200;
#endif
            mouse_term.c_cc[VMIN] = 0;
            mouse_term.c_cc[VTIME] = 1;
#if defined(HAVE_TERMIOS_H)
            tcsetattr(mouse_fd, TCSANOW, &mouse_term);
#else
            ioctl(mouse_fd, TCSETA, &mouse_term);
#endif
            io_device_add(mouse_fd);
            mouse_type = MOUSE_X11;
        }
    }

    mouse_dev = dev;
    trace_ilog("mouse_init(%s) : %d (%d)\n", (dev ? dev : "n/a"),
        (mouse_type == MOUSE_NONE ? -1 : 0), mouse_fd);

    if (mouse_type != MOUSE_NONE) {
        return (TRUE);
    }
    return (FALSE);
}


/*
 *  sys_mouseclose ---
 *      Close the mouse interface.
 */
void
sys_mouseclose(void)
{
    switch (mouse_type) {
    case MOUSE_GPM:
#if defined(HAVE_LIBGPM) && defined(HAVE_GPM_H)
        if (mouse_fd != -1) {
            io_device_remove(gpm_fd);
            Gpm_Close();
            mouse_fd = -1;
        }
#endif
        break;

    case MOUSE_X11:
        if (mouse_fd != -1) {
            io_device_remove(mouse_fd);
            close(mouse_fd);
            mouse_fd = -1;
        }
        break;

    case MOUSE_XTERM:
        /*
         *  xterm
         *    o 1001 not supported under Cygwin, causing terminal state corruption.
         */
        ttpush("\033[?1000l");                  /* disable mouse tracking */
#if !defined(__CYGWIN__)
        ttpush("\033[?1001r");                  /* restore old highlight mouse tracking */
#endif
        ttflush();
        break;
    }
    mouse_type = MOUSE_NONE;
}


/*
 *  sys_mousepoll ---
 *      Poll for any mouse events and decode if available.
 */
int
sys_mousepoll(fd_set *fds, struct MouseEvent *m)
{
    switch (mouse_type) {
#if defined(HAVE_LIBGPM) && defined(HAVE_GPM_H)
    case MOUSE_GPM: {
            if (gpm_flag && FD_ISSET(gpm_fd, fds)) {
                Gpm_Event ev;                   /* Mouse event */

                Gpm_GetEvent(&ev);
                Gpm_FitEvent(&ev);
                GPM_DRAWPOINTER(&ev);
                                                /* decode position and buttons */
                if (ev.type & (GPM_DRAG|GPM_DOWN)) {
                    m->x  = ev.x;
                    m->y  = ev.y;
                    m->b1 = (ev.buttons & GPM_B_LEFT) ? 1 : 0;
                    m->b2 = (ev.buttons & GPM_B_RIGHT) ? 1 : 0;
                    m->b3 = (ev.buttons & GPM_B_MIDDLE) ? 1 : 0;

                    if (ev.type & GPM_DOUBLE) {
                        m->multi = 1;
                    } else if (ev.type & GPM_TRIPLE) {
                        m->multi = 2;
                    } else  {
                        m->multi = 0;
                    }

                    trace_ilog("mouse_event(%0x[%d,%d])\n", ev.type, ev.x, ev.y);
                    return TRUE;
                }
            }
        }
        break;
#endif  /*HAVE_GPM*/

    case MOUSE_XTERM:
        break;

    case MOUSE_X11: {
            static int x_pitch, y_pitch;
            static int max_x, max_y;
            unsigned char mbuf[3];
            int n;

            if (mouse_fd < 0 || !FD_ISSET(mouse_fd, fds)) {
                break;
            }

            if (x_pitch == 0) {
                x_pitch = MAX_X / ttcols();
                y_pitch = MAX_Y / ttrows();
                max_x = x_pitch * (ttcols() - 1);
                max_y = y_pitch * (ttrows() - 1);
            }

            /*
             *  Make sure we get 3 bytes from the mouse and make sure
             *  that we haven't lost sync
             */
            n = read(mouse_fd, mbuf, 1);
            if (n != 1 || (mbuf[0] & 0xc0) != 0xc0)
                return FALSE;

            if (read(mouse_fd, mbuf + 1, 1) != 1)
                return FALSE;

            if (read(mouse_fd, mbuf + 2, 1) != 1)
                return FALSE;

            if (mbuf[1] != 0x80) {
                int d = mbuf[1] & 0x3f;
                if (mbuf[0] & 0x03) {
                    m->x -= 0x40 - d;
                } else {
                    m->x += d;
                }
            }

            if (mbuf[2] != 0x80) {
                int d = mbuf[2] & 0x3f;
                if (mbuf[0] & 0x0c) {
                    m->y -= 0x40 - d;
                } else {
                    m->y += d;
                }
            }
            m->b1 = (mbuf[0] & 0x20) != 0;
            m->b2 = (mbuf[0] & 0x10) != 0;
            m->b3 = 0;
            m->multi = 0;

            /*
             *  Check againt dim, limit bottom-right usage to
             *  remove possible scrolling issues.
             */
            if (m->y < 0)       m->y = 0;
            if (m->x < 0)       m->x = 0;
            if (m->y >= max_y)  m->y = max_y - 1;
            if (m->x > max_x)   m->x = max_x;

            if (m->x == max_x - 1 && m->y == max_y - 1) {
                --m->x;
            }

            if (m->y != mouse_oldy || m->x != mouse_oldx) {
                vtmouseicon(m->y / y_pitch, m->x / x_pitch, mouse_oldy / y_pitch, mouse_oldx / x_pitch);
                mouse_oldy = m->y;
                mouse_oldx = m->x;
            }
            return TRUE;
        }
        break;
    }
    return FALSE;
}


void
sys_mousepointer(int on)
{
    (void) on;
}
#endif  /*HAVE_MOUSE*/

#endif  /*!_VMS && !__OS2__ && !__MSDOS__*/
