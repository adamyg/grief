#include <edidentifier.h>
__CIDENT_RCSID(gr_m_signal_c,"$Id: m_signal.c,v 1.9 2014/10/26 22:13:11 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_signal.c,v 1.9 2014/10/26 22:13:11 ayoung Exp $
 * Signal symbol primitives.
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

#include <sys/stat.h>
#ifndef  _GNU_SOURCE
#define  _GNU_SOURCE
#endif

#include <editor.h>
#include <errno.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_signal.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "echo.h"
#include "eval.h"                               /* get_...()/isa_...() */
#include "prntf.h"
#include "symbol.h"                             /* sym_...() */

#ifndef SIGHUP
#define SIGHUP              -1
#endif
#ifndef SIGINT
#define SIGINT              -1
#endif
#ifndef SIGQUIT
#define SIGQUIT             -1
#endif
#ifndef SIGILL
#define SIGILL              -1
#endif
#ifndef SIGTRAP
#define SIGTRAP             -1
#endif
#ifndef SIGABRT
#define SIGABRT             -1
#endif
#ifndef SIGIOT
#define SIGIOT              -1
#endif
#ifndef SIGBUS
#define SIGBUS              -1
#endif
#ifndef SIGFPE
#define SIGFPE              -1
#endif
#ifndef SIGKILL
#define SIGKILL             -1
#endif
#ifndef SIGUSR1
#define SIGUSR1             -1
#endif
#ifndef SIGSEGV
#define SIGSEGV             -1
#endif
#ifndef SIGUSR2
#define SIGUSR2             -1
#endif
#ifndef SIGPIPE
#define SIGPIPE             -1
#endif
#ifndef SIGALRM
#define SIGALRM             -1
#endif
#ifndef SIGTERM
#define SIGTERM             -1
#endif
#ifndef SIGSTKFLT
#define SIGSTKFLT           -1
#endif
#ifndef SIGCLD
#define SIGCLD              -1
#endif
#ifndef SIGCHLD
#define SIGCHLD             -1
#endif
#ifndef SIGCONT
#define SIGCONT             -1
#endif
#ifndef SIGSTOP
#define SIGSTOP             -1
#endif
#ifndef SIGTSTP
#define SIGTSTP             -1
#endif
#ifndef SIGTTIN
#define SIGTTIN             -1
#endif
#ifndef SIGTTOU
#define SIGTTOU             -1
#endif
#ifndef SIGURG
#define SIGURG              -1
#endif
#ifndef SIGXCPU
#define SIGXCPU             -1
#endif
#ifndef SIGXFSZ
#define SIGXFSZ             -1
#endif
#ifndef SIGVTALRM
#define SIGVTALRM           -1
#endif
#ifndef SIGPROF
#define SIGPROF             -1
#endif
#ifndef SIGWINCH
#define SIGWINCH            -1
#endif
#ifndef SIGPOLL
#define SIGPOLL             -1
#endif
#ifndef SIGIO
#define SIGIO               -1
#endif
#ifndef SIGPWR
#define SIGPWR              -1
#endif
#ifndef SIGSYS
#define SIGSYS              -1
#endif
#ifndef SIGUNUSED
#define SIGUNUSED           -1
#endif

#ifndef SIGRTMIN
#define NO_SIGRTMIN
#define SIGRTMIN            -1
#endif
#ifndef SIGRTMAX
#define SIGRTMAX            -1
#endif

#define SIGUNKNOWN          99

static const char *         str_signal(int signo);

static const struct {
    const char *    tag;
    int             value;
#if !defined(HAVE_STRSIGNAL)
    const char *    desc;
#define __S(__x,__d)        #__x, __x, __d          /* No   Description */
#else
#define __S(__x,__d)        #__x, __x               /* No   Description */
#endif
} x_signal_consts[] = {
    __S(SIGHUP,     "hangup"                    ),  /* 1    Hangup (POSIX). */
    __S(SIGINT,     "interrupt"                 ),  /* 2    Interrupt (ANSI). */
    __S(SIGQUIT,    "quit (POSIX)"              ),  /* 3    Quit (POSIX). */
    __S(SIGILL,     "illegal instruction"       ),  /* 4    Illegal instruction (ANSI). */
    __S(SIGTRAP,    "trace trap"                ),  /* 5    Trace trap (POSIX). */
    __S(SIGABRT,    "abort"                     ),  /* 6    Abort (ANSI). */
    __S(SIGIOT,     "iOT trap"                  ),  /* 6    IOT trap (4.2 BSD). */
    __S(SIGBUS,     "bUS error"                 ),  /* 7    BUS error (4.2 BSD). */
    __S(SIGFPE,     "floating-point exception"  ),  /* 8    Floating-point exception (ANSI). */
    __S(SIGKILL,    "kill, unblockable"         ),  /* 9    Kill, unblockable (POSIX). */
    __S(SIGUSR1,    "user-defined signal 1"     ),  /* 10   User-defined signal 1 (POSIX). */
    __S(SIGSEGV,    "segmentation violation"    ),  /* 11   Segmentation violation (ANSI). */
    __S(SIGUSR2,    "user-defined signal 2"     ),  /* 12   User-defined signal 2 (POSIX). */
    __S(SIGPIPE,    "broken pipe"               ),  /* 13   Broken pipe (POSIX). */
    __S(SIGALRM,    "alarm clock"               ),  /* 14   Alarm clock (POSIX). */
    __S(SIGTERM,    "termination"               ),  /* 15   Termination (ANSI). */
    __S(SIGSTKFLT,  "stack fault"               ),  /* 16   Stack fault. */
    __S(SIGCHLD,    "child status has changed"  ),  /* 17   Child status has changed (POSIX). */
    __S(SIGCLD,     "same as SIGCHLD"           ),  /*      Same as SIGCHLD (System V). */
    __S(SIGCONT,    "continue"                  ),  /* 18   Continue (POSIX). */
    __S(SIGSTOP,    "stop, unblockable"         ),  /* 19   Stop, unblockable (POSIX). */
    __S(SIGTSTP,    "keyboard stop"             ),  /* 20   Keyboard stop (POSIX). */
    __S(SIGTTIN,    "background read from tty"  ),  /* 21   Background read from tty (POSIX). */
    __S(SIGTTOU,    "background write to tty"   ),  /* 22   Background write to tty (POSIX). */
    __S(SIGURG,     "urgent condition on socket"),  /* 23   Urgent condition on socket (4.2 BSD). */
    __S(SIGXCPU,    "cPU limit exceeded"        ),  /* 24   CPU limit exceeded (4.2 BSD). */
    __S(SIGXFSZ,    "file size limit exceeded"  ),  /* 25   File size limit exceeded (4.2 BSD). */
    __S(SIGVTALRM,  "virtual alarm clock"       ),  /* 26   Virtual alarm clock (4.2 BSD). */
    __S(SIGPROF,    "profiling alarm clock"     ),  /* 27   Profiling alarm clock (4.2 BSD). */
    __S(SIGWINCH,   "window size change"        ),  /* 28   Window size change (4.3 BSD, Sun). */
    __S(SIGPOLL,    "pollable event occurred"   ),  /*      Pollable event occurred (System V). */
    __S(SIGIO,      "i/O now possible"          ),  /* 29   I/O now possible (4.2 BSD). */
    __S(SIGPWR,     "power failure restart"     ),  /* 30   Power failure restart (System V). */
    __S(SIGSYS,     "bad system call"           ),  /* 31   Bad system call. */
    __S(SIGUNUSED,  "unused systeem call"       ),  /*      Synonymous with SIGSYS. */
    __S(SIGUNKNOWN, "unknown signal"            )   /* 99   Unknown error */
#undef  __S
    };


/*  Function:           sym_signal_constants
 *      Create and initialise the well-known global symbols.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
sym_signal_constants(void)
{
    SYMBOL *sp;
    unsigned i;

    for (i = 0; i < (sizeof(x_signal_consts)/sizeof(x_signal_consts[0])); ++i) {
        sp = sym_push(1, x_signal_consts[i].tag, F_INT, SF_CONSTANT|SF_SYSTEM);
        sym_assign_int(sp, x_signal_consts[i].value);
    }
    sp = sym_push(1, "SIGRTMIN", F_INT, SF_CONSTANT|SF_SYSTEM);
    sym_assign_int(sp, SIGRTMIN);
    sp = sym_push(1, "SIGRTMAX", F_INT, SF_CONSTANT|SF_SYSTEM);
    sym_assign_int(sp, SIGRTMAX);
}


/*  Function:           do_strsignal
 *      strsignal primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strsignal - Return string describing signal

        int
        strsignal(int signo,
            [string &manifest], [int multi = FALSE])

    Macro Description:
        The 'strsignal()' primitive returns a string describing the
        signal number passed in the argument 'signo'.

    Macro Parameters:
        signo - Integer value of the signal number to be decoded.

        manifest - Optional string variable which is specified shall be
            populated with the signal manifest.

    Macro Returns:
        The 'strsignal()' primitive returns the appropriate description
        string, or an unknown signal message if the signal number is
        invalid.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        strerror

 *<<GRIEF>> [proc]
    Topic: Signals

        Signals are a limited form of inter-process communication
        used in Unix, Unix-like, and other POSIX compliant operating
        systems. A signal is an asynchronous notification sent to a
        process or to a specific thread within the same process in
        order to notify it of an event that occurred.

        The following manifest constants are the general set of
        signals which may be supported representing their positive
        signal number, which if not supported by the underlying
        operating system shall have an assigned value of -1.

(start table,format=nd)
        [Constant           [Description                        ]
      ! SIGHUP              Hangup (POSIX).
      ! SIGINT              Interrupt (ANSI).
      ! SIGQUIT             Quit (POSIX).
      ! SIGILL              Illegal instruction (ANSI).
      ! SIGTRAP             Trace trap (POSIX).
      ! SIGABRT             Abort (ANSI).
      ! SIGIOT              IOT trap (4.2 BSD).
      ! SIGBUS              BUS error (4.2 BSD).
      ! SIGFPE              Floating-point exception (ANSI).
      ! SIGKILL             Kill, unblockable (POSIX).
      ! SIGUSR1             User-defined signal 1 (POSIX).
      ! SIGSEGV             Segmentation violation (ANSI).
      ! SIGUSR2             User-defined signal 2 (POSIX).
      ! SIGPIPE             Broken pipe (POSIX).
      ! SIGALRM             Alarm clock (POSIX).
      ! SIGTERM             Termination (ANSI).
      ! SIGSTKFLT           Stack fault.
      ! SIGCHLD             Child status has changed (POSIX).
      ! SIGCLD              Same as SIGCHLD (System V).
      ! SIGCONT             Continue (POSIX).
      ! SIGSTOP             Stop, unblockable (POSIX).
      ! SIGTSTP             Keyboard stop (POSIX).
      ! SIGTTIN             Background read from tty (POSIX).
      ! SIGTTOU             Background write to tty (POSIX).
      ! SIGURG              Urgent condition on socket (4.2 BSD).
      ! SIGXCPU             CPU limit exceeded (4.2 BSD).
      ! SIGXFSZ             File size limit exceeded (4.2 BSD).
      ! SIGVTALRM           Virtual alarm clock (4.2 BSD).
      ! SIGPROF             Profiling alarm clock (4.2 BSD).
      ! SIGWINCH            Window size change (4.3 BSD, Sun).
      ! SIGPOLL             Pollable event occurred (System V).
      ! SIGIO               I/O now possible (4.2 BSD).
      ! SIGPWR              Power failure restart (System V).
      ! SIGSYS              Bad system call.
      ! SIGUNKNOWN          Unknown error
(end table)

 */
void
do_strsignal(void)              /* (int signo, [string &manifest], [int multi = FALSE]) */
{
    const int xsignal = get_xinteger(1, -1);

    if (!isa_undef(2)) {                        /* optional, constant name */
#if !defined(NO_SIGRTMIN)
        char t_signame[32] = {0};
#endif
        const int multi = get_xinteger(3, FALSE);
        const char *tag = NULL;
        char *buf = NULL;

        if (xsignal > 0) {

#if !defined(NO_SIGRTMIN)                       /* run-time signals, dynamic range */
            if (xsignal >= SIGRTMIN && xsignal <= SIGRTMAX) {
                sxprintf(t_signame, sizeof(t_signame), "SIGRT%d", xsignal - SIGRTMIN);
                tag = t_signame;
            }
#endif

            if (NULL == tag) {                  /* well defined values */
                size_t taglen, buflen = 0;
                unsigned i;

                for (i = 0; i < (sizeof(x_signal_consts)/sizeof(x_signal_consts[0])); ++i) {
                                                /* could map, but only expect low usage */
                    if (xsignal == x_signal_consts[i].value) {
                        tag = x_signal_consts[i].tag;
                        if (! multi) {
                            break;
                        }
                        taglen = strlen(tag);
                        if (buf) buf[buflen++] = ',';
                        if (NULL == (buf = chk_realloc(buf, buflen + taglen + 1))) {
                            break;
                        }
                        strcpy(buf + buflen, tag);
                        buflen += taglen;
                        tag = buf;
                    }
                }
            }
        }
        argv_assign_str(2, tag ? tag : "SIGUNKNOWN");
        chk_free(buf);
    }
    acc_assign_str(str_signal(xsignal), -1);
}


static const char *
str_signal(int signo)
{
#if !defined(HAVE_STRSIGNAL)
    static char buf[32];                        /* TODO */
    size_t i;

    if (signo >= 0) {
        for (i = 0; i < (sizeof(x_signal_consts)/sizeof(x_signal_consts[0])); ++i) {
            if (signo == x_signal_consts[i].value) {
                return x_signal_consts[i].desc;
            }
        }
    }
    (void) sprintf(buf, "signal %d", signo);
    return buf;

#else
    return strsignal(signo);
#endif
}
/*end*/
