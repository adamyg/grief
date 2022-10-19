#include <edidentifier.h>
__CIDENT_RCSID(gr_signals_c,"$Id: signals.c,v 1.19 2022/08/10 15:44:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: signals.c,v 1.19 2022/08/10 15:44:57 cvsuser Exp $
 * Signal handling.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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
#include <edstacktrace.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "signals.h"			        /* public interface */

#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"                               /* eclear() */
#include "main.h"
#include "procspawn.h"
#include "system.h"                             /* sys_... () */
#include "tty.h"                                /* tt..() */

static int              x_fatallevel = 0;       /* Fatal exception level */

static void             signal_mode0(void);
static void             signal_mode1(void);
static const char *     signame(int sig, int unknown);
static int              sigfatal(int sig, const char **name);

extern void             sighandler_int(int sig);
extern void             sighandler_tstp(int sig);
extern void             sighandler_ttin(int sig);
extern void             sighandler_sys1(int sig);
extern void             sighandler_sys2(int sig);
extern void             sighandler_usr1(int sig);
extern void             sighandler_usr2(int sig);
extern void             sighandler_term(int sig);
extern void             sighandler_abrt(int sig);
extern void             sighandler_pipe(int sig);
extern void             sighandler_hup(int sig);
extern void             sighandler_danger(int sig);


void
signals_init(int mode)
{
    switch (mode) {
    case 0:
        signal_mode0();
        break;
    case 1:
        signal_mode1();
        break;
    }
}


void
signals_shutdown(void)
{
}


static void
signal_mode0(void)
{
    signal(SIGINT, sighandler_int);

#if defined(linux) || defined(unix) || defined(_AIX) || defined(__APPLE__)
    if (xf_sigtrap) {
#if defined(SIGBUS)
        signal(SIGBUS,  sighandler_sys1);       /* bus error */
#endif
#if defined(SIGSEGV)
        signal(SIGSEGV, sighandler_sys1);       /* segmentation violation (ANSI) */
#endif
#if defined(SIGILL)
        signal(SIGILL,  sighandler_sys1);       /* illegal */
#endif
#if defined(SIGFPE)
        signal(SIGFPE,  sighandler_sys1);       /* floating point */
#endif
#if defined(SIGIOT)
        signal(SIGIOT,  sighandler_sys1);
#endif
#if defined(SIGABRT)
        signal(SIGABRT, sighandler_abrt);
#endif
    }
#endif  /*linux || unix || _AIX || __APPLE__ */

#if defined(SIGPIPE)
    sys_signal(SIGPIPE, sighandler_pipe);
#endif
}


static void
signal_mode1(void)
{
    /*
     *  Signal handlers.
     */
    if (xf_sigtrap) {
#if defined(linux) || defined(unix) || defined(_AIX) || defined(__APPLE__)
#if defined(SIGBUS)
        signal(SIGBUS, sighandler_sys2);        /* bus error */
#endif
#if defined(SIGSEGV)
        signal(SIGSEGV, sighandler_sys2);       /* segmentation violation (ANSI) */
#endif
#if defined(SIGILL)
        signal(SIGILL, sighandler_sys2);        /* illegal */
#endif
#if defined(SIGFPE)
        signal(SIGFPE, sighandler_sys2);        /* floating point */
#endif
#endif  /*linux || unix || _AIX || __APPLE__ */

#if defined(SIGTERM)
        signal(SIGTERM, sighandler_term);       /* termination */
#endif
#if defined(SIGHUP)
        signal(SIGHUP, sighandler_hup);         /* hangup */
#endif
    }

#if defined(SIGUSR1)
    signal(SIGUSR1, sighandler_usr1);
#endif
#if defined(SIGUSR2)
    signal(SIGUSR2, sighandler_usr2);
#endif
#if defined(SIGDANGER)
    signal(SIGDANGER, sighandler_danger);       /* virtual memory low */
#endif

    /*
     *  Job control
     */
#if defined(SIGTSTP)
    if (signal(SIGTSTP, SIG_IGN) == SIG_DFL) {  /* running under shell? */
#if defined(HAVE_SIGACTION)
        struct sigaction handler = {0};

        handler.sa_handler = sighandler_tstp;
        handler.sa_flags = 0;
        sigaction(SIGTSTP, &handler, NULL);

#if defined(SIGTTIN)
        handler.sa_handler = sighandler_ttin;
        handler.sa_flags = 0;
        sigaction(SIGTTIN, &handler, NULL);
#endif
#else
        signal(SIGTSTP, sighandler_tstp);
#endif  /*SIGACTION*/
    }
#endif  /*SIGTSTP*/
}


/*
 *  sighandler_int ---
 *      SIGINT handler.
 */
void
sighandler_int(int sig)
{
    __CUNUSED(sig)
    execute_event_ctrlc();
    signal(SIGINT, sighandler_int);
}


#if defined(SIGTSTP)
/*
 *  sighandler_tstp ---
 *      Function called to handle JOB control stop signal.
 */
void
sighandler_tstp(int sig)
{
#if defined(HAVE_SIGACTION)
    sigset_t mask;
#endif
    sig=sig;

    proc_prep_stop(TRUE);

#if defined(SIGTTOU)
    signal(SIGTTOU, SIG_DFL);                   /* ignore ISGTTOU whilst we move the cursor */
#endif
#if defined(SIGTTOU)
    signal(SIGTTOU, SIG_DFL);
#endif

#if defined(HAVE_SIGACTION)
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
#else   /*defined(HAVE_SIGSETMASK)*/
    sigsetmask(0);
#endif

    signal(SIGTSTP, SIG_DFL);
    kill(getpid(), SIGTSTP);                    /* sleep */
    signal(SIGTSTP, sighandler_tstp);

    proc_prep_start();
    vtupdate();
}
#endif  /*SIGTSTP*/


#if defined(SIGTSTP) && defined(SIGTTIN)
/*
 *  sighandler_ttin ---
 *      SIGTTIN handler.
 */
void
sighandler_ttin(int sig)
{
    sig=sig;
    signal(SIGTTIN, sighandler_ttin);
}
#endif  /*SIGTSTP && SIGTTIN*/


static const char *
signame(int sig, int unknown)
{
    const char *name = (unknown ? "unknown" : NULL);

    switch (sig) {
#if defined(SIGBUS)
    case SIGBUS:
        name = "SIGBUS";
        break;
#endif
#if defined(SIGSEGV)
    case SIGSEGV:
        name = "SIGSEGV";
        break;
#endif
#if defined(SIGILL)
    case SIGILL:
        name = "SIGILL";
        break;
#endif
#if defined(SIGFPE)
    case SIGFPE:
        name = "SIGFPE";
        break;
#endif
#if defined(SIGIOT) && (SIGIOT != SIGABRT)
    case SIGIOT:
        name = "SIGIOT";
        break;
#endif
#if defined(SIGTERM)
    case SIGTERM:
        name = "SIGTERM";
        break;
#endif
    case SIGABRT:
        name = "SIGABRT";
        break;
    default:
        break;
    }
    return name;
}


/*
 *  sigfatal ---
 *      Common fatal handler.
 */
static __CINLINE int
sigfatal(int sig, const char **name)
{
    const char *t_name = signame(sig, TRUE);
    const int level = ++x_fatallevel;

    if (1 == level) {
        char buf[100];

        signal(sig, SIG_DFL);                   /* reset handler */
        sxprintf(buf, sizeof(buf), "_fatal_error %d \"%s\"", sig, t_name);
        execute_str(buf);
    }
    *name = t_name;
    return level;
}


/*
 *  sighandler_sys1 ---
 *      SIGBUS/SIGSEGV/SIGIOT fatal handler.
 */
void
sighandler_sys1(int sig)
{
    const char *name = signame(sig, TRUE);

    ++x_fatallevel;
    vtclose(FALSE);
    fprintf(stderr,
        "\n\nA program error signal %s (%d) occured during startup, exiting.\n", name, sig);
    fflush(stderr);
    xf_dumpcore = 2;                            /* dump core on exit */
    gr_exit(1);
}


#if defined(linux) || defined(unix) || defined(_AIX) || defined(__APPLE__)
/*
 *  sighandler_sys2 ---
 *      SIGBUS/SIGSEGV/SIGIOT fatal handler, single level handler.
 *
 *<<GRIEF>> [callback]
    Macro: _fatal_error - Fatal condition callback.

        void
        _fatal_error(int signo, string desc)

    Macro Description:
        The '_fatal_error' callback is executed by Grief upon a fatal
        signal condition being thrown by the operating system; these
        generally represent editor bugs or possible macro usage
        violations.

        The list of trapped condition is very system dependent, yet the
        following are generally included.

(start table,format=nd)
            [Constant       [Description            ]

        !   SIGBUS          Bus error
        !   SIGSEGV         Segmentation violation.
        !   SIGILL          Illegal instruction.
        !   SIGFPE          Floating point error.
(end table)

        Note!:
        The macro implementation should utilise as few Grief facilities
        as possible to avoid re-triggering the same condition.

        Refer to the 'core' macro for an example.

    Macro Parameters:
        signo - Integer signal number.

        desc - String containing the signal description.

    Macro Returns:
        The '_start_complete' should return nothing

    Macro Portability:
        n/a

    Macro See Also:
        Callbacks
 */
void
sighandler_sys2(int sig)
{
    const char *name = signame(sig, TRUE);
    const int level = sigfatal(sig, &name);

    xf_dumpcore = 2;                            /* dump core on exit */

    if (level <= 2) {
        eclear();
        ttflush();
        ttmove(ttrows() - 1, 0);
    }
    printf("ALERT - Fatal signal detected %s (%d) - level (%d).\r\n", name, sig, level);

    if (level > 2) {
        _exit(3);
    }

    gr_exit(1);
}
#endif  /*unix || _AIX || __APPLE__ */


#if defined(SIGUSR1)
/*
 *  sighandler_usr1 ---
 *      SIGUSR1 handler, toggle trace.
 */
void
sighandler_usr1(int sig)
{
    const int flags = trace_flags();

    __CUNUSED(sig)
    if (flags) {
        trace_flagsset(0);
    } else {
        trace_flagsset(DB_TRACE);
    }
    execute_event_usr1();
}
#endif  /*SIGUSR1*/


#if defined(SIGUSR2)
/*
 *  sighandler_usr2 ---
 *      SIGUSR2 handler, toggle flush.
 */
void
sighandler_usr2(int sig)
{
    const int flags = trace_flags();

    __CUNUSED(sig)
    if (flags) {
        if (DB_FLUSH & flags) {
            trace_flagsset(flags & ~DB_FLUSH);
        } else {
            trace_flagsset(flags | DB_FLUSH);
        }
    }
    execute_event_usr2();
}
#endif  /*SIGUSR2*/


#if defined(SIGTERM)
/*
 *  sighandler_term ---
 *      SIGTERM handler.
 */
void
sighandler_term(int sig)
{
    __CUNUSED(sig)
    signal(SIGTERM, SIG_DFL);
    sigfatal(SIGTERM, NULL);
    vtclose(FALSE);
    fprintf(stderr, "\n\nA program termination signalled, exiting.\n");
    fflush(stderr);
    gr_exit(1);
}
#endif  /*SIGTERM*/


/*
 *  sighandler_abrt ---
 *      SIGABRT handler.
 */
void
sighandler_abrt(int sig)
{
    __CUNUSED(sig)
    signal(SIGABRT, SIG_DFL);
    sigfatal(SIGABRT, NULL);
    vtclose(FALSE);
    fprintf(stderr, "\n\nA program abort occurred, exiting.\n");
    fflush(stderr);
    xf_dumpcore = 2;                            /* dump core on exit */
    gr_exit(1);
}


#if defined(SIGPIPE)
/*
 *  sighandler_abrt ---
 *      SIGPIPE handler.
 */
void
sighandler_pipe(int sig)
{
    __CUNUSED(sig)
    fflush(stderr);
    vtclose(FALSE);
    fprintf(stderr, "\n\nBroken pipe, exiting.\n");
    fflush(stderr);
    gr_exit(1);
}
#endif  /*SIGPIPE*/


#if defined(SIGHUP)
/*
 *  sighandler_abrt ---
 *      SIGHUP handler.
 */
void
sighandler_hup(int sig)
{
    __CUNUSED(sig)
    vtclose(FALSE);
    fprintf(stderr, "\n\nBroken pipe, exiting.\n");
    fflush(stderr);
    gr_exit(1);
}
#endif  /*SIGHUP*/


#if defined(SIGDANGER)
/*
 *  sighandler_danger ---
 *      SIGDANGER handler, low memory condition.
 */
void
sighandler_danger(int sig)
{
    __CUNUSED(sig)
    signal(SIGDANGER, sighandler_danger);
//TODO - autosave/trigger
    ewprintf("WARNING - Low virtual memory");
}
#endif  /*SIGDANGER*/

/*end*/
