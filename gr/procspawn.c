#include <edidentifier.h>
__CIDENT_RCSID(gr_procspawn_c,"$Id: procspawn.c,v 1.28 2025/07/02 13:29:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: procspawn.c,v 1.28 2025/07/02 13:29:59 cvsuser Exp $
 * Process spawn primitive and management.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"                               /* file_...() */
#include "getkey.h"
#include "mouse.h"
#include "procspawn.h"
#include "system.h"                             /* sys_...() */
#include "tty.h"

#if defined(SIGCHLD)
#define SIGCHILD        SIGCHLD                 /* posix */
#elif defined(SIGCLD)
#define SICHILD         SIGCLD                  /* old school */
#endif

#define PROCESSSTATUSSZ 16

typedef TAILQ_HEAD(_ProcessList, _process)
                        PROCESSLIST_t;

typedef struct _process {
    MAGIC_t             p_magic;                /* structure magic */
#define PROCESS_MAGIC       MKMAGIC('P','r','O','c')
    TAILQ_ENTRY(_process)
                        p_node;                 /* list node */
    IDENTIFIER_t        p_ident;                /* internal process identifier */
    int                 p_pid;                  /* process identifier */
    unsigned            p_usize;                /* sizeof user area */
    char *              p_macro;                /* completion macro */
    void              (*p_cleanup)(void *);     /* cleanup interface */

      /*
       *  ... user arena of 'usize' in bytes.
       */
} process_t;

static IDENTIFIER_t     x_process_ident;        /* internal processing identifier */

static PROCESSLIST_t    x_process_list;         /* active process list */

static const char *     x_sh_cache = NULL;      /* Shell name cache */

static int              x_child_signal = FALSE; /* TRUE when child signal triggered */

#if defined(HAVE_MOUSE)
static int              x_mouse_active = 0;
#endif

int                     x_background = FALSE;   /* background process ? */


/*  Function:           do_shell
 *      shell primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: shell - Spawn a sub-shell process.

        int
        shell([string cmd], [int use_shell],
            [string completion],
            [string stdin], [string stdout], [string stderr],
            [int mode], [string spec])

    Macro Description:
        The 'shell' primitive performs executes a command specified
        in 'cmd' by creating a system dependent shell, and returns
        after the command has been completed.

        The 'shell' primitive can be used to create a subshell
        without exiting the current Grief session; placing Grief into
        the background. Without any arguments an interactive shell is
        created. The user terminates the sub-shell by typing the
        usual ^D or exit.

>           shell()

        Alternatively the 'shell' primitive can be used to execute an
        explicit command, as in the following examples.

        The following example performs a 'uname(1)' command
        redirecting into a output in a temporary working file for use
        on completion; the 'TRUE' informs Grief not to bother
        repainting the screen.

>           shell("uname 2>&1 >/tmp/uname.tmp", TRUE);

        The following example performs a 'gmake' command redirecting
        output plus registers the macro 'gmake_complete' to be
        executed on completion.

>           shell("gmake 2>&1 >/tmp/gmake.tmp", TRUE, "gmake_completion");

        associated completion macro.

>           void
>           gmake_completion(int status)
>           {
>               message("gmake done, status = %d", status);
>           }

    Meta Characters::

        As the command is passed on to a command processor care
        should be taken with special characters (including '$', '?',
        '*', '[' and ';') since the command wild-card and environment
        expansion will occur.

        Although there is no definite syntax for wildcard operations,
        common features include:

(start table)
            [Wildcard       [Description                                ]
         !  ?               Matches any single character.

         !  *               Matches none or more characters.

         !  [seq]           Matches any one of the characters within the
                            sequence. Sequences generally support ranges;
                            two characters separated by '-' denote
                            a range. (e.g. '[A-Fa-f0-9]' is equivalent
                            to '[ABCDEFabcdef0123456789]'.)

         !  [!seq]          Matches any one of the characters not
                            contained within the sequence.
(end table)

    Redirection::

        In addition input/output redirection is system dependent
        and/or shell command interpreter specific. Despite this fact
        the following are normally supported across all supported
        Grief targets.

(start table)
            [Redirection    [Description                                ]

          ! >               Writes the command output to a file or a
                            device, such as a printer, instead of the
                            Command Prompt window.

          ! <               Reads the command input from a file,
                            instead of reading input from the keyboard.

          ! >>              Appends the command output to the end of
                            a file without deleting the information
                            that is already in the file.

          ! >&              Writes the output from one handle to the
                            input of another handle; the standard handle
                            assignments are '0=stdin', '1=stdout'
                            and '2=stderr'.

          ! <&              Reads the input from one handle and
                            writes it to the output of another handle.
(end table)

        Note!:
        If there is any doubt regarding portability of redirection
        operators it is advised to use the explicit 'stdin', 'stdout'
        and 'stderr' arguments.

    Macro Parameters:
        cmd - Optional string containing the command to be passed to
            the host environment to be executed by a command
            processor in an implementation-dependent manner. If
            omitted a interactive sub-shell is created.

        use_shell - Optional boolean value, if *true* forces the
            original display and terminal settings to the restored,
            and than initialised on completion of the sub-process.

        completion - Optional string containing the name of macro
            to be executed on the termination of the sub-process.

            The completion routine is called with the first parameter
            set to the return status from the underlying process. Any
            other positional parameters are shifted up one.

        stdin - Option string, specifies the name of the file/device
            from which standard input shall be source. If omitted the
            sub-shell standard input remains unchanged.

        stdout - Option string, specifies the name of the file/device
            to which standard output shall be redirected. If omitted
            the sub-shell standard output remains unchanged.

        stderr - Option string, specifies the name of the file/device
            to which standard error shall be redirected. If omitted
            the sub-shell standard error remains unchanged.

        mode - Optional mode flags, specifies the creation mode to be
           utilised during stream creation. If omitted '0644' is
           applied.

        spec - Optional string, reserved for future use.

    Macro Returns:
        The 'shell' primitive returns the exit status from the
        executed command (0 .. 256), otherwise -1 on error and sets
        <errno> to indicate the error.

(start table,format=nd)
        [Constant       [Description                                    ]

      ! E2BIG           Combined Size of environment and argument list
                        is too large.

      ! EACCES          Search permission is denied on a component of
                        the path prefix of filename or the name of a
                        script interpreter.

      ! EACCES          The file or a script interpreter is not a
                        regular file.

      ! EACCES          Execute permission is denied.

      ! EIO             An I/O error occurred.

      ! ENAMETOOLONG    Path is too long.

      ! ENOENT          The command or one of its components does
                        not exist.

      ! ENOEXEC         An executable is not in a recognized format,
                        is for the wrong architecture, or has some
                        other format error that means it cannot be
                        executed.

      ! ENOMEM          Insufficient kernel memory was available.
(end table)

    Macro Portability:
        A arguments differ from the original implementation.

    Macro See Also:
        dos, connect
 */
void
do_shell(void)                  /* int ([string cmd], [int use_shell], [string completion],
                                            [string stdin], [string stdout], [string stderr], [int mode], [string spec]) */
{
    const char *cmd     = get_xstr(1);
    const int  repaint  = get_xinteger(2, TRUE);
    const char *macro   = get_xstr(3);          /* completion macro */
    const char *fstdin  = get_xstr(4);
    const char *fstdout = get_xstr(5);
    const char *fstderr = get_xstr(6);          /* extension, stderr redirection */
    const int  mode     = get_xinteger(7,0644); /* extension, creation mode */
    const char *spec    = get_xstr(8);          /* extension, pipe options (pipe,pty) */
    int status;

#define SNULL(s)        (s && s[0] ? s : NULL)

    __CUNUSED(spec)
    proc_prep_stop(repaint);
    x_background = TRUE;
    status = sys_shell(SNULL(cmd), SNULL(macro), SNULL(fstdin), SNULL(fstdout), SNULL(fstderr), mode);
    x_background = FALSE;
    proc_prep_start();
    eredraw();
    acc_assign_int((accint_t) status);
}


#if defined(SIGCHILD) || defined(SIGPIPE)
/*
 *  signal_child( int sig ) ---
 *      SIGCHILD and SIGPIPE handler
 */
static void
sigxxx_child(int sig)
{
    (void)sig;
    x_child_signal = TRUE;
}
#endif  /*SIGCHILD || SIGPIPE*/


/*
 *  proc_add ---
 *      Add the specified process to the process list.
 */
void *
proc_add(int pid, const char *macro,
    void (*cleanup)(void *), const void *udata, int usize)
{
    PROCESSLIST_t *pl = &x_process_list;
    process_t *pp;

    assert(pid > 0);
    assert(usize >= 0);

    if (0 == x_process_ident++) {
#if defined(SIGCHILD)
        sys_signal(SIGCHILD, sigxxx_child);
#endif
#if defined(SIGPIPE)
        sys_signal(SIGPIPE, sigxxx_child);
#endif
#if defined(SIGCHILD)
#if defined(HAVE_SIGACTION)
        {
            struct sigaction act;
            (void)sigaction(SIGCHILD, NULL, &act);
            act.sa_flags &= ~SA_RESTART; /*flag=1*/
            sigaction(SIGCHILD, &act, NULL);
        }
#elif defined(HAVE_SIGINTERRUPT)
        siginterrupt(SIGCHILD, 1);
#endif
#endif
        TAILQ_INIT(pl);
    }

    if (NULL == (pp = chk_alloc(sizeof(process_t) + usize))) {
        return NULL;
    }
    memset(pp, 0, sizeof(process_t) + usize);
    if (usize && udata) {                       /* copy user data */
        memcpy((void *)(pp + 1), (const void *)udata, usize);
    }

    pp->p_magic   = PROCESS_MAGIC;
    pp->p_pid     = pid;
    pp->p_ident   = x_process_ident;
    if (macro && *macro) {
        const size_t mlen = strlen(macro) + 1;

        if (NULL != (pp->p_macro = chk_alloc(mlen + PROCESSSTATUSSZ))) {
            memcpy(pp->p_macro, macro, mlen);
        } else {
            chk_free((void *)pp);
            return NULL;
        }
    }
    pp->p_cleanup = cleanup;
    pp->p_usize   = usize;

    TAILQ_INSERT_HEAD(pl, pp, p_node);
    return (pp + 1);
}


/*
 *  proc_check ---
 *      Process any events from child processes.
 */
void
proc_check(void)
{
#if defined(SIGCHILD) || defined(SIGPIPE)
    if (! x_child_signal)
        return;
#endif
    proc_wait(-1);
}


int
proc_enum(int (*callback)(int pid, void *udata), void *udata)
{
    int ret = FALSE;

    if (callback && x_process_ident) {
        PROCESSLIST_t *pl = &x_process_list;
        process_t *pp;

        TAILQ_FOREACH(pp, pl, p_node) {
            assert(PROCESS_MAGIC == pp->p_magic);
            if ((ret = (*callback)(pp->p_pid, udata)) == TRUE) {
                break;
            }
        }
    }
    return ret;
}


int
proc_find(int pid, void **udata)
{
    if (pid >= 0 && x_process_ident) {
        PROCESSLIST_t *pl = &x_process_list;
        process_t *pp;

        TAILQ_FOREACH(pp, pl, p_node) {
            assert(PROCESS_MAGIC == pp->p_magic);
            if (pp->p_pid == pid) {
                if (udata) {
                    *udata = (pp->p_usize > 0 ? (void *)(pp + 1) : NULL);
                }
                return pp->p_usize;
            }
        }
    }
    return -1;
}


/*
 *  proc_wait ---
 *      wait for a process to die, and return the exit status of the process.
 *
 *      If pid is -1 then wait for any process to die, otherwise wait for a
 *      specific process to die.
 */
int
proc_wait(int pid)
{
    PROCESSLIST_t *pl = &x_process_list;
    int ret, status;
    process_t *pp;
    BUFFER_t *bp;

    errno = 0;
    while (1) {
        while (1) {
#if defined(HAVE_SIGACTION)
            {
                struct sigaction act;
                (void)sigaction(SIGINT, NULL, &act);
                act.sa_flags &= ~SA_RESTART; /* flag=1 */
                sigaction(SIGINT, &act, NULL);
            }
#elif defined(HAVE_SIGINTERRUPT)
            siginterrupt(SIGINT, 1);
#endif
            ret = sys_waitpid(pid, &status, pid < 0 ? WNOHANG : 0);
#if defined(SIGCHILD)
            sys_signal(SIGCHILD, sigxxx_child);
#endif
            if (ret < 0) {
                if (EINTR == errno) {           /* ptrace, ignore */
                    continue;
                }
                if (pid < 0) {
                    x_child_signal = FALSE;
                    return status;
                }
            }
            if (ret >= 0) {
                break;
            }
        }

        /*
         *  locate associated buffer (if any)
         */
        for (bp = buf_first(); bp; bp = buf_next(bp)) {
            if (bp->b_display && bp->b_display->d_pid == ret) {
                bp->b_wstat = status;
                break;
            }
        }

        /*
         *  process completion callbacks
         */
        TAILQ_FOREACH(pp, pl, p_node) {
            assert(PROCESS_MAGIC == pp->p_magic);
            if (pp->p_pid == ret) {
                if (pp->p_cleanup) {            /* system specific cleanup */
                    (*pp->p_cleanup)(pp + 1);
                }

                if (pp->p_macro) {              /* user specific */
                    char t_status[PROCESSSTATUSSZ], *args;
                    int slen;

                    /*
                     *  <macro> <status> [<args>]
                     */
                    for (args = pp->p_macro; *args && ' ' != *args;) {
                        ++args;
                    }
                    slen = sxprintf(t_status, sizeof(t_status), " %d%s", status, (*args ? " " : ""));
                    memmove(args + slen, (const char *)args, strlen(args) + 1);
                    memcpy(args, (const char *)t_status, slen);
                    execute_str(pp->p_macro);
                    chk_free((void *)pp->p_macro);
                }

                TAILQ_REMOVE(pl, pp, p_node);
                pp->p_magic = 0;
                chk_free((void *)pp);
                break;
            }
        }

        if (NULL == TAILQ_FIRST(pl) || pid < 0 || pid == ret) {
            break;
        }
    }

    x_child_signal = FALSE;
    if (! x_background) {
        vtupdate();
    }
    return status;
}


const char *
proc_shell_get(void)
{
    if (NULL == x_sh_cache) {                       /* cached ? */
        x_sh_cache = sys_getshell();                /* .. system specific */
    }
    return x_sh_cache;
}


static int
cmdis(const char *shell, size_t slen, const char *cmd)
{
    const size_t clen = strlen(cmd);
    const char *p = (shell + slen) - clen;

    if (slen == clen || (slen > clen && (p[-1] == '\\' || p[-1] == '/'))) {
        if (str_icmp(p, cmd) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}


int
proc_shell_iscmd(const char *shell)
{
    const size_t slen = strlen(shell);

    if (cmdis(shell, slen, "cmd") ||
            cmdis(shell, slen, "cmd.exe") ||
            cmdis(shell, slen, "command") ||
            cmdis(shell, slen, "command.exe")) {
        return TRUE;
    }
    return FALSE;
}


/*
 *  proc_prep_stop ---
 *      Prepare for subsell spawn or SIGTSTP processing.
 */
void
proc_prep_stop(int repaint)
{
#if defined(HAVE_MOUSE)
    x_mouse_active = mouse_active();
    mouse_close();
#endif
    if (x_scrfn.scr_control) {
        (*x_scrfn.scr_control)(SCR_CTRL_SAVE, repaint);
    }
    if (repaint) {
        ttclose();
    }
}


/*
 *  proc_prep_start ---
 *      Prepare for sub-shell spawn or SIGTSTP return.
 */
void
proc_prep_start(void)
{
    sys_initialise();
    ttopen();
    if (x_scrfn.scr_control) {
        (*x_scrfn.scr_control)(SCR_CTRL_RESTORE, 0);
    }
    io_reset_timers();
#if defined(HAVE_MOUSE)
    if (x_mouse_active) {
        mouse_init("");
    }
#endif
    file_chdir(NULL);                           /* clear directory cache */
    vtgarbled();
    vtupdate();
}

/*end*/
