#include <edidentifier.h>
__CIDENT_RCSID(gr_m_pty_c,"$Id: m_pty.c,v 1.28 2024/12/06 15:46:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_pty.c,v 1.28 2024/12/06 15:46:06 cvsuser Exp $
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

/*#define ED_LEVEL 2*/

#include <editor.h>
#include <edfileio.h>
#include <edtermio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#include "accum.h"                              /* acc_...() */
#include "asciidefs.h"                          /* ASCII.. */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"                            /* get_...() */
#include "debug.h"                              /* trace_...() */
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "getkey.h"
#include "line.h"
#include "lisp.h"
#include "m_pty.h"
#include "mac1.h"
#include "main.h"
#include "map.h"
#include "procspawn.h"
#include "search.h"
#include "symbol.h"                             /* sym_...() */
#include "tty.h"
#include "window.h"
#include "word.h"

#if defined(linux)
#define COLORTERM       "linux"
#define MONOTERM        "linux-m"
#else
#define COLORTERM       "ansi"
#define MONOTERM        "ansi-m"
#endif

enum {
    ESCAPE_CSI = 1,
    ESCAPE_OSC
};

static DISPLAY_t *      p_create(int x, int y);
static void             p_flagset(DISPLAY_t *dp, uint32_t flags);
static int              p_waitfor(accint_t tmo, const struct regopts *regopts, const LIST *lp, const char *str);
static void             p_update(BUFFER_t *bp, const char *buf);
static void             p_push(BUFFER_t *bp, const char *str);

static int              p_escape_type(int leading);
static int              p_escape_term(int type, int ch);
static void             p_escape_decode(DISPLAY_t *dp);
static void             p_newline(DISPLAY_t *dp);
static char *           p_flush(const char *start, const char *end);

static int              p_sgrlength(const LINECHAR *cp, const LINECHAR *end, char command);

static void             p_term_save(char *buf, int mono);
static void             p_term_reset(const char *buf);

static int              x_ptynum = 0;
static int              x_ptyident = 12000;


/*  Function:           do_connect
 *      connect primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: connect - Attach a process to a process.

        int
        connect(int mode, string shell = NULL, string cwd = NULL)

    Macro Description:
        The 'connect()' primitive creates a sub-process and attaches
        it to the current buffer. The buffer behaviour is similar to
        that of a virtual terminal with the sub-process standard
        input and output streams redirected to the buffer. The
        virtual terminal implementation is system dependent yet most
        utilise pseudo-devices otherwise pipes.

(start ditaa -S)

       User Space

              /------------\    Fork/Exec      /-------------\
              |   Editor   | ................> | Sub-Process |
              \------------/                   \-------------/
                      ^ |                            ^  |
                 read | | write                stdin |  | stdout
                      | |                            |  |
         ..........   | |   ...................      |  |  ..............
                      | |                            |  |
       Kernel Space   | |                            |  |
                      | |                            |  |
                      | V                            |  V
                   /-----------------\     /-----------------\
                   | Psuedo Terminal |<--->| Slave /dev/tty/ |
                   \-----------------/     \-----------------/

(end ditaa)

        All text inserted into the buffer is automatically forwarded
        to the underlying sub-process, insertion methods include
        <self_insert>, <insert> and <paste>. In addition the
        primitive <insert_process> allows for explicit text to be
        forwarded without echo.

        Likewise output from the underlying sub-process is sent to
        the buffer. All output from the sub-process is automatically
        inserted into the buffer, at the process insertion position;
        see <inq_process_position> and <set_process_position>.

        The 'mode' argument specifies the control flags the behaviour
        of the connection, if omitted defaults to 'PF_ECHO'.

        By default, the process created is a shell process, and the
        shell is got from the *SHELL* environment variable. If
        'shell' is specified, then it is taken as the pathname of a
        shell to execute.

        Once attached connect() may be called to change the 'mode'
        flag.

    *Modes*

        Control flags

(start table,format=nd)
    [Constant           [Value      [Description                            ]

  ! PF_ECHO             0x0001      Reserved for macro use, denotes all
                                        typed key strokes are redirected to
                                        the process using a 'REG_TYPE' signal
                                        handler (see register_macro).

  ! PF_NOINSERT         0x0002      Reserved for macro compatibility,
                                        disables automatic insertion of
                                        sub-process output into the 
                                        associated buffer; not implemented.

  ! PF_NONINTERACTIVE   0x0004      Non-interactive mode command shell.

  ! PF_LOCALECHO        0x1000      Local echo mode, characters written to
                                    the buffer using either <insert> or
                                    <self_insert> shall be automatically
                                    sent to the process.

  ! PF_OVERWRITE        0x4000      Overwrite process input (CR/LF
                                    conversion). Indicates whether output
                                    from a process overwrites text in the
                                    buffer or inserts and shifts text
                                    over as it does so. Needed for
                                    effectively allow type-ahead to not
                                    be destroyed but allow escape
                                    sequences to cause data in the buffer
                                    to be overwritten when running
                                    termcap oriented programs.

  ! PF_WAITING          0x8000      Waiting for text. Normally when a
                                    buffer is created, the output from the
                                    subprocess is inserted directly into
                                    the buffer. Setting this bit causes the
                                    output from the process to be held onto,
                                    until the calling macro issues a <wait>
                                    or <wait_for>.
(end table)

    Macro Parameters:
        mode - Optional integer control flags.

        shell - Optional string containing the SHELL
            specification is be utilised, if omitted the
            environment value of SHELL is used.

        cwd - Optional string containing current working directory.

    Macro Returns:
        The 'connect()' primitive returns the positive IP
        identifier associated identifier with the created
        connection, 0 if the buffer is already connected,
        otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        insert_process, inq_connection, disconnect, wait, shell, dos
 */
void
do_connect(void)                /* int (int mode = PF_ECHO, string shell = NULL, string cwd = NULL) */
{
    const uint32_t flags = (uint32_t) get_xinteger(1, PF_ECHO);
    const char *sh    = get_xstr(2);
    const char *shell = (sh && sh[0]) ? sh : proc_shell_get();
    const char *cwd   = get_xstr(3);            /* extension */
    DISPLAY_t *dp;
    char myterm[64];                            /* MAGIC */
    int ret;

    ED_TRACE(("connect(flags:0x%x,sh:%s)\n", flags, sh))

    if (NULL != (dp = curbp->b_display)) {      /* already connected */
        p_flagset(dp, flags);
        ED_TRACE(("=> already connected\n"))
        ret = 0;

    } else {
        dp = p_create(ttcols() - (xf_borders * 2), ttrows() - ((xf_borders * 2) + 1));

        if (NULL == dp) {
            ewprintf("Couldn't allocate memory for connection.");
            ret = -1;

        } else {
            p_flagset(dp, flags);
            p_term_save(myterm, FALSE);

            if (-1 == (ret = pty_connect(dp, shell, cwd /*(PF_NONINTERACTIVE  & flags ?  FALSE : TRUE)*/))) {
                pty_free(dp);

            } else if (1 == ret) {
                proc_add(dp->d_pid, NULL, NULL, NULL, 0);
                curbp->b_display = dp;
                if (dp->d_pid >= 0) {
                    infof("Buffer connected.");
                }
                ED_TRACE(("==> connected (pid:%d)\n", dp->d_pid))
                io_pty_state(TRUE);
                if (dp->d_pipe_in >= 0) {
                    io_device_add(dp->d_pipe_in);
                }
                ++x_ptynum;
                pty_poll();
            }
            p_term_reset(myterm);
        }
    }

    ED_TRACE(("==> ret:%d\n", ret))
    acc_assign_int(ret);
}


/*  Function:           do_disconnect
 *      discount primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>> [proc]
    Macro: inq_connection - Connection information.

        int
        inq_connection(int cid, int &flags, int &pid)

    Macro Description:
        The 'inq_connection()' primitive retrieves information
        regarding the connection 'cid'.

    Macro Parameters:
        cid - Integer connection identifier returned from connect.
        flags - Integer variable to be populated with the
            connection flags.
        pid - Integer variable to be populated with the
            connection process identifier.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        connect, disconnect
 */
void
inq_connection(void)            /* int (int cid, int &flags, int &pid) */
{
    DISPLAY_t *dp;
    int ret = -1;

    if (NULL != (dp = pty_argument(1))) {
        argv_assign_int(2, (accint_t) dp->d_flags);
        argv_assign_int(3, (accint_t) dp->d_pid);
        ret = dp->d_ident;
    }
    acc_assign_int(ret);
}


/*  Function:           do_disconnect
 *      discount primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: disconnect - Disconnect a buffer from a process.

        int
        disconnect()

    Macro Description:
        The 'disconnect()' primitive disconnects the current buffer
        from any attached process, if any, terminating the
        subprocess.

        Note!:
        Under Unix the terminated subprocess is sent a SIGTERM
        followed by a SIGKILL signal.

    Macro Parameters:
        none

    Macro Returns:
        Returns 1 on success otherwise 0.

    Macro Portability:
        n/a

    Macro See Also:
        connect, disconnect, send_signal
 */
void
do_disconnect(void)             /* int () */
{
    DISPLAY_t *dp = (curbp ? curbp->b_display : NULL);
    int ret = 0;

    ED_TRACE(("discount()\n"))
    if (dp) {
        pty_cleanup(curbp);
        pty_poll();
        ret = 1;
    }
    acc_assign_int(ret);
}


/*  Function:           do_send_signal
 *      send_signal primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: send_signal - Send signal to a process buffer.

        int
        send_signal(int signal)

    Macro Description:
        The 'send_signal()' primitive send a signal to a process or
        a group of processes attached to the current buffer. The
        signal to be sent is specified by 'signo' and is either
        one from the list given in <Signals> or 0.

        If 'signo' is 0 or omitted, then the null signal is sent,
        error checking is performed but no signal is actually sent.

        See the unix 'kill' system function is more details.

    Macro Parameters:
        signo - Optional integer signal number, if omitted
            defaults to the 'null' signal.

    Macro Returns:
        The 'send_signal()' primitive upon successful completion
        returns 0 and 1 when no buffer is attached. Otherwise -1
        shall be returned and <errno> set to indicate the error.

    Macro Portability:
        n/a

    Macro See Also:
        connect, disconnect, send_signal
 */
void
do_send_signal(void)            /* int (int signo = 0) */
{
    DISPLAY_t *dp = (curbp ? curbp->b_display : NULL);
    int ret = 1;

    if (dp) {                                   /* Send signal to process group */
        ret = pty_send_signal(dp->d_pid, get_xinteger(1, 0));
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           inq_process_position
 *      inq_process_position primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_process_position - Get position of process buffer.

        int
        inq_process_position([int &line], [int &column])

    Macro Description:
        The 'inq_process_position()' primitive retrieves the
        current cursor position for the underlying process, this
        primitive is similar to <inq_position>.

        The process position is used for output from the process;
        rather than inserting the output from the process where
        the users cursor is, a separate cursor is maintained
        instead.

        This permits the user to move around the buffer whilst
        the process is generating output without the process
        output being sprinkled through the buffer.

    Macro Parameters:
        line - Optional integer variable to be populated with the
            cursor line.

        column - Optional integer variable to be populated with
            the cursor row.

    Macro Returns:
        The 'inq_process_position()' primitive returns 0 on success,
        otherwise -1 if the current buffer is not attached to a
        process.

    Macro Portability:
        n/a

    Macro See Also:
        set_process_position, connect
 */
void
inq_process_position(void)      /* int ([int &line], [int &column]) */
{
    DISPLAY_t *dp = (curbp ? curbp->b_display : NULL);
    int ret = -1;

    if (dp) {
        argv_assign_int(1, (accint_t) dp->d_curline);
        argv_assign_int(2, (accint_t) dp->d_curcol);
        ret = 0;
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           set_process_position
 *      set_process_position primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_process_position - Set process insertion position.

        int
        set_process_position([int line], [int column])

    Macro Description:
        The 'set_process_position()' primitive sets the line and/or
        column associated with the input from a subprocess.

        Processes maintain their own independent input in the
        buffer so that it is easier to write macros which
        manipulate subprocesses.

    Macro Parameters:
        line - Optional integer specifying the line number,
            if positive the cursor is set to the specified line.

        column - Optional integer specifying the column number,
            if positive the cursor is set to the specified column.

    Macro Returns:
        The 'set_process_position()' primitive returns 0 on success,
        otherwise -1 if the current buffer is not attached to a
        process.

    Macro Portability:
        n/a

    Macro See Also:
        inq_process_position, connect
 */
void
set_process_position(void)      /* ([int line], [int column]) */
{
    DISPLAY_t *dp = (curbp ? curbp->b_display : NULL);
    int ret = -1;

    if (dp) {
        if (isa_integer(1)) {
            const accint_t line = get_xinteger(1, 1);

            dp->d_curline =
                (line <= 1 ? 1 : (line < curbp->b_numlines ? line : curbp->b_numlines));
            dp->d_curcol = 1;
        }

        if (isa_integer(2)) {
            const accint_t col = get_xinteger(2, 1);

            dp->d_curcol = (col <= 0  ? 1 : col);
        }

        ret = 0;
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_wait
 *      wait() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: wait - Wait for attached process to terminate.

        int
        wait([int &status])

    Macro Description:
        The 'wait()' primitive suspends execution until for the
        process attached to the current buffer terminates,
        returning status information for the terminated child, or
        until delivery of a signal whose action is either to
        execute a signal-catching function or to terminate the
        process.

        This primitive may be aborted by pressing a space.

    Macro Parameters:
        status - Optional integer variable populated with the
            process status.

    Macro Returns:
        The 'wait()' primitive returns 0 if the process has
        completed, -2 if the user aborted, otherwise -1 if no
        process was attached.

    Macro Portability:
        n/a

    Macro See Also:
        wait_for, inq_process_position, connect
 */
void
do_wait(void)                   /* int (int &status) */
{
    int ret = -1;

    if (curbp->b_display) {
        ret = 0;
        while (curbp->b_display) {
            if (' ' == io_get_key(1000)) {
                /*
                 *  Wait for process to disappear.
                 */
                ret = -2;
                break;
            }
        }

        if (0 == ret) {
            argv_assign_int(1, (accint_t) curbp->b_wstat);
        }
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_wait_for
 *      wait_for() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: wait_for - Wait for process output.

        int
        wait_for([int timeout],
            list|string pattern, [int flags = 0])

    Macro Description:
        The 'wait_for()' primitive waits for a specific string of
        characters to be output by the current buffers attached
        sub-process for up to the specified time 'timeout'.

        The character sequence is in the form of regular
        expression 'pattern' using the search flags 'flags'. The
        pattern is either a single string expression or a list of
        string expressions. When a list is given, then a parallel
        match is performed as each character is read from the
        buffer.

    Macro Parameters:
        timeout - Optional integer timeout in seconds, if omitted
            or zero the primitive blocks indefinitely until the
            sequence is encountered or the sub-process terminates.

        pattern - Character sequences to be matched, as either a
            single string containing a regular expression or a
            list of string expressions.

        flags - Optional integer stating the search flags to be
            applied (see re_search).

    Macro Returns:
        The 'wait_for()' primitive returns a non-negative return on
        a success match, otherwise -1 if there is no process
        currently attached to the current buffer, or the user
        interrupted.

        On a single string expression 'wait_for' returns 1,
        otherwise upon a list of string expressions returns the
        matching index.

    Macro Portability:
        n/a

    Macro See Also:
        wait, inq_process_position, connect
 */
void
do_wait_for(void)               /* int ([int timeout], list|string expr, [int search-flags = 0]) */
{
    const accint_t tm = get_xaccint(1, 0);
    const int flags = (isa_undef(3) ? 0 : get_xinteger(3, 0));
    struct regopts regopts;
    int ret = -1;

    search_options(&regopts, TRUE, flags);

    if (isa_list(2)) {
        ret = p_waitfor(tm, &regopts, get_list(2), NULL);

    } else if (isa_string(2)) {
        ret = p_waitfor(tm, &regopts, NULL, get_str(2));
    }

    acc_assign_int(ret);
}


static DISPLAY_t *
p_create(int x, int y)
{
    DISPLAY_t *dp;

    if (NULL == (dp = chk_calloc(sizeof(DISPLAY_t), 1))) {
        return NULL;
    }
    dp->d_magic    = DISPLAY_MAGIC;
    dp->d_ident    = ++x_ptyident;
    dp->d_escptr   = NULL;
    dp->d_escmax   = dp->d_esclen = 0;
    dp->d_flags    = 0;
    dp->d_rows     = y;
    dp->d_cols     = x;
    dp->d_color    = PTY_FG_SET(WHITE);
    dp->d_lastchar = 0;
    dp->d_attr     = 0;
    dp->d_wlen     = 0;
    dp->d_waitfor  = NULL;
    dp->d_curline  = 1;
    dp->d_curcol   = 1;
    dp->d_attrline = -1;
    dp->d_attrcol  = -1;
    dp->d_pipe_in  = -1;
    dp->d_pipe_out = -1;
    dp->d_cleanup  = NULL;
    return dp;
}


DISPLAY_t *
pty_argument(int argi)
{
    if (isa_integer(argi)) {
        const int cid = get_xinteger(argi, 0);

        if (cid > 0) {
            register BUFFER_t *bp;
            DISPLAY_t *dp;

            for (bp = buf_first(); bp; bp = buf_next((BUFFER_t *)bp)) {
                if (NULL != (dp = bp->b_display)) {
                    assert(DISPLAY_MAGIC == dp->d_magic);
                    if (cid == dp->d_ident) {
                        return dp;
                    }
                }
            }
        }

    } else if (isa_undef(argi)) {
        DISPLAY_t *dp;

        if (curbp && NULL != (dp = curbp->b_display)) {
            return dp;
        }
    }
    return NULL;
}


void
pty_free(DISPLAY_t *dp)
{
    assert(DISPLAY_MAGIC == dp->d_magic);
    dp->d_magic = 0xDEADBEEF;
    chk_free(dp);
}


static void
p_flagset(DISPLAY_t *dp, uint32_t flags)
{
    dp->d_flags = flags;
    if (PF_OVERWRITE & flags) {
        dp->d_flags |= PF_NOCR | PF_NOLF;       /* enable <CR>/<LF> logic */
    }
}


static int
p_waitfor(accint_t tmo, const struct regopts *regopts, const LIST *lp, const char *str)
{
#define QUEUE_SIZE 64
    REGEXP *prog = NULL;
    size_t qlen = 0;
    char qbuf[QUEUE_SIZE + 1] = {0};
    char buf[2] = {0};
    int ret = 0;

    if (NULL == curbp->b_display) {
        ewprintf("cannot wait on a normal buffer.");
        return 0;
    }

    if (NULL == lp) {                           /* pattern-list or pattern */
        if (NULL == str || NULL == (prog = regexp_comp(regopts, str))) {
            return -4;                          /* bad expression */
        }
    }

    ED_TRACE(("waitfor(tmo:%d,pattern:%s)\n", (int)tmo, (lp ? "LIST" : str)))

    second_passed();
    curbp->b_display->d_flags |= PF_WAITING;

    while (curbp->b_display && !io_typeahead()) {
        int cnt = -1;

        if (pty_died(curbp)) {
            ret = -3;                           /* died */
            break;
        }

        if ((cnt = pty_read(curbp, buf, 1)) < 1) {
            if (' ' == io_get_key(1000)) {
                ret = -2;                       /* aborted */
                break;
            }

            if (TRUE == second_passed()) {
                if (--tmo < 0) {
                    ret = -1;                   /* timeout */
                    break;
                }
            }
            vtupdate();
            continue;
        }

        p_update(curbp, buf);
        if (qlen >= QUEUE_SIZE) {
            memmove(qbuf, qbuf + 1, QUEUE_SIZE - 1);
            qbuf[QUEUE_SIZE - 1] = buf[0];
        } else {
            qbuf[qlen++] = buf[0];
        }
        ED_TRACE(("\t%d,<%s>\n", qlen, qbuf))

        if (lp) {
            const LIST *nextlp, *lp1;
            int idx;

            for (idx = 0, lp1 = lp; (nextlp = atom_next(lp1)) != lp1; ++idx) {
                if (NULL != (str = atom_xstr(lp1))) {
                    if (NULL == (prog = regexp_comp(regopts, str))) {
                        ret = -4;               /* bad expression */
                        goto done;
                    }

                    if (regexp_exec(prog, qbuf, qlen, 0) > 0) {
                        ret = idx;              /* 0 ... idx */
                        goto done;
                    }
                }
                lp1 = nextlp;
            }
        } else {
            if (regexp_exec(prog, qbuf, qlen, 0) > 0) {
                ret = 1;                        /* success */
                break;
            }
            continue;
        }
    }

done:;
    vtupdate();
    if (curbp->b_display) {
        curbp->b_display->d_flags &= ~PF_WAITING;
    }
    ED_TRACE(("==> %d\n", ret))
    regexp_free(prog);
    return ret;
}


static void
p_update(BUFFER_t *bp, const char *buf)
{
    DISPLAY_t *dp = bp->b_display;
    WINDOW_t *wp;

    ED_TRACE(("p_update(bp:%p,buf:%s,dp:%p,flags:0x%x)\n", bp, buf, dp, dp->d_flags))

    p_push(bp, buf);

    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_bufp == bp) {                 /* XXX - generic update func? */
            wp->w_status |= WFHARD;
            wp->w_line = dp->d_curline;
        }
}


/*
 *  p_push ---
 *      push the specified string into the given buffer.
 */
static void
p_push(BUFFER_t *bp, const char *str)
{
    BUFFER_t *saved_bp = curbp;
    const int saved_no_undo = BFTST(curbp, BF_NO_UNDO);
    DISPLAY_t *dp = bp->b_display;
    const char *start = NULL;

    set_curbp(bp);
    BFSET(curbp, BF_NO_UNDO);

    if ((*cur_line = dp->d_curline) < 1) {
        *cur_line = 1;
    }

    if ((*cur_col = dp->d_curcol) < 1) {
        *cur_col = 1;
    }

    for (; *str; ++str) {
        const int ch = *str;

        /*
         *  buffering ESC?
         */
        if (dp->d_escptr) {
            if (0 == dp->d_esclen &&
                    -1 == (dp->d_esctyp = p_escape_type(ch))) {

                ED_TRACE(("p_push: ESC unknown (%d/0x%x)\n", ch, ch))
                dp->d_escptr = NULL;            /* unknown */

            } else {
                if (dp->d_esclen < dp->d_escmax) {
                    *dp->d_escptr++ = (char)ch;
                    ++dp->d_esclen;
                }
                                                /* terminator? */
                if (p_escape_term(dp->d_esctyp, ch)) {
                    p_escape_decode(dp);
                    dp->d_escptr = NULL;        /* done */
                }
                continue;
            }
        }

        /*
         *  others
         */
        switch (ch) {
        case ASCIIDEF_ESC:      /* ESC <cmd> */
            start = p_flush(start, str);
            dp->d_escptr = dp->d_escbuf;
            dp->d_escmax = sizeof(dp->d_escbuf)-1;
            dp->d_esctyp = -1;
            dp->d_esclen = 0;
            break;

        case ASCIIDEF_BEL:      /* bell */
            start = p_flush(start, str);
            ttbeep();
            break;

        case ASCIIDEF_CR:       /* carriage-return */
            start = p_flush(start, str);
            *cur_col = 1;

            if (XF_TEST(2)) {
                dp->d_flags &= ~PF_NOCR;        /* <CR> encountered */
                if (ASCIIDEF_LF == str[1]) {
                    *cur_line += 1;             /* optimise CR/LF */
                    dp->d_flags &= ~PF_NOLF;
                    ++str;

                } else if (PF_NOLF & dp->d_flags) {
                    *cur_line += 1;             /* implied new-line */
                }
            }
            break;

            /*
             *  if (ASCIIDEF_LF != str[1]) {
             *      start = p_flush(start, str);
             *      *cur_col = 1;
             *      break;
             *  }
             *  end = str++;
             *  - FALLTHRU -
             */

        case ASCIIDEF_LF:       /* new-line */
            start = p_flush(start, str);
            *cur_line += 1;

            if (XF_TEST(2)) {
                dp->d_flags &= ~PF_NOLF;        /* <LF> encountered */
                if (PF_NOCR & dp->d_flags) {
                    *cur_col = 1;               /* implied carriage-return */
                }
            }
            break;

        case ASCIIDEF_BS:       /* backspace */
            start = p_flush(start, str);
            if (*cur_col > 1) {
                *cur_col -= 1;
            }
            break;

        case ASCIIDEF_FF:       /* form-feed */
            ED_TRACE(("p_push: form-feed\n"))
            start = p_flush(start, str);
            break;

        case ASCIIDEF_HT:       /* tab */
        case ASCIIDEF_DEL:      /* delete */
        default:                /* others */
            if (NULL == start) {
                start = str;
            }
            break;
        }
    }

    p_flush(start, str);

    dp->d_curline = *cur_line;
    dp->d_curcol = *cur_col;

    if (curwp && curwp->w_bufp == curbp) {
        set_buffer_parms(bp, curwp);
    }
    set_curbp(saved_bp);
    if (! saved_no_undo) {
        BFCLR(curbp, BF_NO_UNDO);
    }
}


/*
 *  p_escape_type ---
 *      determine the escape sequence type.
 */
static int
p_escape_type(int leading)
{
    switch (leading) {
    case '[':                   /* CSI <cmd> */
        return ESCAPE_CSI;
    case ']':                   /* ESC ] <cmd> */
        return ESCAPE_OSC;
    }
    return -1;
}


/*
 *  p_escape_term ---
 *      escape type specific terminator check.
 */
static int
p_escape_term(int type, int ch)
{
    switch (type) {
    case ESCAPE_CSI:    /* CSI [<num>[[;[<num>]] ... [;] <cmd> */
        switch (ch) {
        case '@':
        case '`':
            return TRUE;
        default:                                /* a-zA-Z */
            return isalpha(ch);
        }
        return FALSE;

    case ESCAPE_OSC:    /* OSC <cmd> [;<text>] <ST or BEL> */
        return (0x9 == ch || 0x7 == ch);
    }
    return TRUE;
}


/*
 *  p_escape_decode ---
 *      escape sequence decoder (xterm and ANSI)
 */
static void
p_escape_decode(DISPLAY_t *dp)
{
    const char *cp = dp->d_escbuf;
    int type;

    if (-1 == (type = p_escape_type(*cp++))) {
        return;
    }

    *dp->d_escptr = '\0';                       /* terminate buffer */

#define ARGVAL(_idx,_def)   (args[_idx-1] > 0 ? args[_idx-1] : _def)

    if (ESCAPE_CSI == type) {
        /*
         *  CSI <cmd>
         */
        int args[DISPLAY_ESCAPELEN] = {0};
        int argno = 0;

        ED_TRACE(("p_escape: CSI <%s>\n", cp))
        while (*cp) {
            const char ch = *cp;

            if (';' == ch) {                    /* empty parameter = 0 */
                ++cp, ++argno;

            } else if (isdigit(ch)) {           /* parameter numeric value */
                char *cursor;
                int val = (int) strtoul(cp, &cursor, 10);

                if (argno < DISPLAY_ESCAPELEN) {
                    args[argno++] = val;
                }
                cp = (*cursor == ';' ? cursor + 1 : cursor);

            } else if (isalpha(ch)) {           /* command */
                break;

            } else {
                return;
            }
        }

        switch (*cp) {
        case '@': {     /* CSI Ps @         Insert Ps (Blank) Character(s) (default = 1) (ICH) */
                int cnt = ARGVAL(1, 1);

                while (cnt-- > 0) {
                    if (*cur_col > dp->d_cols) {
                        p_newline(dp);          /* obey margins */
                    }
                    linsertc(' ');
                }
            }
            break;

        case 'A':       /* CSI Ps A         Cursor Up Ps Times (default = 1) (CUU) */
            *cur_line -= ARGVAL(1, 1);
            break;

        case 'B':       /* CSI Ps B         Cursor Down Ps Times (default = 1) (CUD) */
            *cur_line += ARGVAL(1, 1);
            break;

        case 'C':       /* CSI Ps C         Cursor Forward Ps Times (default = 1) (CUF) */
            *cur_col += ARGVAL(1, 1);
            break;

        case 'D':       /* CSI Ps D         Cursor Backward Ps Times (default = 1) (CUB) */
            *cur_col -= ARGVAL(1, 1);
            break;

        case 'E':       /* CSI Ps E         Cursor Next Line Ps Times (default = 1) (CNL) */
            *cur_line += ARGVAL(1, 1);
            break;

        case 'F':       /* CSI Ps F         Cursor Preceding Line Ps Times (default = 1) (CPL) */
            *cur_line -= ARGVAL(1, 1);
            break;

        case 'G':       /* CSI Ps G         Cursor Character Absolute [column] (default = [row,1]) */
            *cur_col = ARGVAL(1, 1);
            break;

        case 'H':       /* CSI Ps ; Ps H    Cursor Position [row;column] (default = [1,1]) (CUP) */
        case 'f': {     /* CSI Ps ; Ps f    Horizontal and Vertical Position [row;column] (default = [1,1]) (HVP) */
                const LINENO line = ARGVAL(1, 1), col = ARGVAL(2, 1);

                *cur_line = curwp->w_top_line + line;
                *cur_col = col;
                llinepad();
            }
            break;

        case 'J': {     /* CSI Ps J         Erase in Display (ED) */
                /*
                 *  Ps = 0  -> Erase Below (default).
                 *  Ps = 1  -> Erase Above.
                 *  Ps = 2  -> Erase All.
                 *  Ps = 3  -> Erase Saved Lines (xterm).
                 */
                if (2 == args[0]) {
                    *cur_line = curwp->w_top_line;
                }

                while (*cur_line <= curbp->b_numlines) {    /* NEWLINE */
                    lremove(curbp, *cur_line);
                }
            }
            break;

        case 'K': {     /* CSI Ps K         Erase in Line (EL) */
                /*
                 *  Ps = 0  -> Erase to Right (default).
                 *  Ps = 1  -> Erase to Left.
                 *  Ps = 2  -> Erase All.
                 */
                const int mode = ARGVAL(1, 0);

                if (0 == mode) {
                    do_delete_to_eol();

                } else if (1 == mode) {
                    const int ccol = *cur_col;

                    *cur_col = 1;
                    while (*cur_col < ccol) {
                        lwritec(' ');
                    }
                    *cur_col = ccol;

                } else if (2 == mode) {
                    do_delete_line();
                    lnewline();
                }
            }
            break;

        case 'M': {     /* CSI Ps M         Delete Ps Line(s) (default = 1) (DL) */
                int cnt = ARGVAL(1, 1);

                while (cnt-- > 0) {
                    do_delete_line();
                }
            }
            break;

        case 'S':       /* CSI Ps S         Scroll up Ps lines (default = 1) (SU) */
            break;

        case 'T':       /* CSI Ps T         Scroll down Ps lines (default = 1) (SD) */
            break;

        case 'X':       /* CSI Ps X         Erase Ps Character(s) (default = 1) (ECH) */
            ldeletec(ARGVAL(1, 1));
            break;

        case '`':       /* CSI Pm `         Character Position Absolute [column] (default = [row,1]) */
            *cur_col = ARGVAL(1, 1);
            break;

        case 'b': {     /* CSI Ps b         Repeat the preceding graphic character Ps times (REP) */
                int lastchar, cnt = ARGVAL(1, 1);

                if ((lastchar = dp->d_lastchar) > 0) {
                    while (cnt-- > 0) {
                        if (*cur_col > dp->d_cols) {
                            p_newline(dp);      /* obey margins */
                        }
                        linsertc(lastchar);
                    }
                }
            }
            break;

        case 'd':       /* CSI Pm d         Line Position Absolute [row] (default = [1,column]) */
            *cur_line =
                curwp->w_top_line + ARGVAL(1, 1);
            break;

        case 'm': {     /* CSI Pm m         Character Attributes (SGR) */
                const LINENO cline = *cur_line, ccol = *cur_col;
                unsigned cnt = 0;
                int idx;

                /* decode escapes */
                for (idx = 0; idx < argno; ++idx) {
                    int val = args[idx];

                    ++cnt;
                    switch (val) {
                    case 0:         /* Ps = 0   normal */
                    case 1:         /* Ps = 1   bold */
                    case 2:         /*          faint */
                    case 3:         /*          italic */
                    case 4:         /* Ps = 4   underlined */
                    case 5:         /* Ps = 5   bright */
                    case 6:         /*          blink */
                    case 7:         /* Ps = 7   reverse */
                    case 8:         /* Ps = 8   invisible/conceal */
                    case 9:         /*          crossed-out */
                                    /*          fonts */
                    case 10: case 11: case 12: case 13: case 14:
                    case 15: case 16: case 17: case 18: case 19:
                        break;

                    case 22:        /* Ps = 22  clear bold */
                    case 23:        /*          clear italic */
                    case 24:        /* Ps = 24  clear underline */
                    case 25:        /* Ps = 25  clear blink */
                    case 27:        /* Ps = 27  clear reverse */
                    case 28:        /* Ps = 28  visible/conceal */
                    case 29:        /*          clear crossed-out */
                        break;
                                    /* Ps = 30+ foreground color */
                    case 30: case 31: case 32: case 33:
                    case 34: case 35: case 36: case 37:
                        break;

                                    /* Ps = 40+ background color */
                    case 40: case 41: case 42: case 43:
                    case 44: case 45: case 46: case 47:
                        break;

                    case 38:        /* 38;5;<color> - foreground color 256 color mode */
                        if ((idx + 2) < argno && 5 == args[idx + 1]) {
                            idx += 2;
                        }
                        break;

                    case 48:        /* 38;5;<color> - background color 256 color mode */
                        if ((idx + 2) < argno && 5 == args[idx + 1]) {
                            idx += 2;
                        }
                        break;

                    case 39:        /* set forergound to default */
                    case 49:        /* set background to default */
                        break;

                    case 51:        /* framed */
                    case 52:        /* encircled */
                    case 53:        /* overlined */
                    case 54:        /* not framed or encircled */
                    case 55:        /* not overlined */
                        break;

                    case 60:        /* ideogram underline or right side line */
                    case 61:        /* ideogram double underline or double line on the right side */
                    case 62:        /* ideogram overline or left side line */
                    case 63:        /* ideogram double overline or double line on the left side */
                    case 64:        /* ideogram stress marking */
                        break;
                                    /* set foreground color, high intensity (aixterm) */
                    case 90:  case 91:  case 92:  case 93:  case 94:
                    case 95:  case 96:  case 97:  case 98:  case 99:
                        break;
                                    /* set background color, high intensity (aixterm) */
                    case 100: case 101: case 102: case 103: case 104:
                    case 105: case 106: case 107: case 108: case 109:
                        break;

                    default:
                        args[idx] = -1;
                        --cnt;
                        break;
                    }
                }

                /* update buffer */
                if (cnt) {
                    const char *prefix = "\033[";
                    char *attrbuf = dp->d_attrbuf;
                    int attrlen = 0, dellen = 0;
                    LINENO length = 0;
                    LINE_t *lp;
                    int dot = 0;

                    for (idx = 0; idx < argno; ++idx) {
                        if (args[idx] >= 0) {
                            attrlen += sprintf(attrbuf + attrlen, "%s%d", prefix, args[idx]);
                            prefix = ";";
                        }
                    }
                    attrlen += sprintf(attrbuf + attrlen, "m");
                    assert(attrlen > 1 && attrlen < (int)(sizeof(dp->d_attrbuf) - 1));

                    if (dp->d_attrline != cline || dp->d_attrcol != ccol) {
                        /*
                         *  new cursor position
                         *      size any existing escape specification to be removed
                         */
                        if (NULL != (lp = linepx(curbp, cline))) {
                            if ((length = llength(lp)) > 0) {
                                dot = line_offset_const(lp, cline, ccol, LOFFSET_FIRSTBYTE);
                                dellen = p_sgrlength(ltext(lp) + dot, ltext(lp) + length, 'm');
                            }
                            vm_unlock(cline);
                        }
                    }

                    if (dellen > 0) {
                        lreplacedot(attrbuf, attrlen, dellen, dot, NULL);
                    } else {
                        linserts(attrbuf, attrlen);
                    }

                    dp->d_attrline = cline;
                    dp->d_attrcol  = ccol;
                    dp->d_attrlen  = attrlen;
                }
            }
            break;

        case 'g':       /* CSI Ps g         Tab Clear (TBC) */
            break;

        case 'l':       /* CSI Pm l         Reset Mode (RM) */
            break;

        case 'r':       /* CSI Ps ; Ps r    Set Scrolling Region [top;bottom] (default = full size of window) */
            break;

        default:
            ED_TRACE(("p_escape: CSI unknown ('%c'/%d/0x%x)\n", \
                (isprint(*cp) ? *cp : '.'), *cp, *cp))
            break;
        }

    } else if (ESCAPE_OSC == type) {
        /*
         *  Operating System Command
         */
        ED_TRACE(("p_escape: OSC <%s>\n", cp))
        return;
    }

    if (*cur_col < 1) {
        *cur_col = 1;
    }

    if (*cur_line < 1) {
        *cur_line = 1;
    }
}


static void
p_newline(DISPLAY_t *dp)
{
    *cur_line += 1;
    *cur_col = 1;
    linserts(dp->d_attrbuf, dp->d_attrlen);
}


static char *
p_flush(const char *start, const char *end)
{
    DISPLAY_t *dp = curbp->b_display;
    int size = end - start;

    assert(end >= start);
    if (size <= 0 || NULL == start) {
        return NULL;
    }

    ED_TRACE(("p_flush(line:%d,col:%d,start:%p,end:%p,size:%d,buf:%*.*s)\n", \
        *cur_line, *cur_col, start, end, size, size, size, start))

    while (size-- > 0) {
        if (*cur_col > dp->d_cols) {
            p_newline(dp);                      /* obey margins */
        }
        linsertc(*start++);
    }
    dp->d_lastchar = start[-1];

    return NULL;
}


static int
p_sgrlength(const LINECHAR *cp, const LINECHAR *end, char command)
{
    const LINECHAR *start = cp;

    if (0x1b == *cp++ && '[' == *cp++) {
        unsigned argno = 0;

        while (cp < end) {
            const char ch = *cp++;

            if (';' == ch) {                    /* empty parameter = 0 */
                ++argno;

            } else if (isdigit(ch)) {           /* parameter numeric value */
                while (cp < end && isdigit(*cp)) {
                    ++cp;
                }
                if (';' == *cp) {
                    ++cp;
                }
            } else if  ('m' == command) {
                return (cp - start);
            }
        }
    }
    return 0;
}


static void
p_term_save(char *buf, int mono)
{
    const char *cp = getenv("TERM");

    if (NULL == cp) {
        *buf = '\0';
    } else {
        strcpy(buf, cp);
    }
    p_term_reset(mono ? MONOTERM : COLORTERM);
}


static void
p_term_reset(const char *buf)
{
    if (*buf) {
        gputenv2("TERM", buf);
    }
}


void
pty_cleanup(BUFFER_t *bp)
{
    DISPLAY_t *dp = bp->b_display;

    ED_TRACE(("pty_cleanup(dp:%p)\n", dp))

    if (NULL == dp) {
        return;
    }

    bp->b_display = NULL;

    if (dp->d_cleanup) {
        (*dp->d_cleanup)(dp);                   /* creator cleanup */
    }

    if (dp->d_pipe_in >= 0) {
        io_device_remove(dp->d_pipe_in);
        fileio_close(dp->d_pipe_in);
        dp->d_pipe_in = -1;
    }

    if (dp->d_pipe_out >= 0) {
        fileio_close(dp->d_pipe_out);
        dp->d_pipe_out = -1;
    }

    if (dp->d_pid) {
        pty_send_term(dp->d_pid);
        dp->d_pid = 0;
    }

    if (dp->d_waitfor) {
        chk_free(dp->d_waitfor);
        dp->d_waitfor = NULL;
    }

    pty_free(dp);

#if (XXX_PTY_DIED)
    if (issignalled()) {
        if (iscored())
        }
    } else {
        return-code
    }
#endif

    infof("%s disconnected.", bp->b_fname);
    if (0 == x_ptynum || 0 == --x_ptynum) {
        io_pty_state(FALSE);
    }
}


/*
 *  Function to poll the process buffers to see if we have any
 *  input to update on the screen
 */
void
pty_poll(void)
{
    static int entry = 0;
    BUFFER_t *ocurbp = curbp;
    int updated;
    char buf[1024+1];

    ED_TRACE(("pty_poll()\n"))

    if (entry++ || x_ptynum <= 0) {             /* protect this code from being re-entered */
        --entry;
        x_ptynum = 0;
        io_pty_state(FALSE);
        ED_TRACE(("==> re-entered\n"))
        return;
    }

start_again:
    do {
        DISPLAY_t *dp;
        BUFFER_t *bp;

        updated = FALSE;
        for (bp = buf_first(); bp; bp = buf_next(bp)) {
            while (NULL != (dp = bp->b_display) && 0 == (dp->d_flags & PF_WAITING)) {
                int cnt;

                errno = 0;
                if ((cnt = pty_read(bp, buf, sizeof(buf)-1)) <= 0) {
                    /*
                     *  If process died, start loop again,
                     *  as this buffer entry will have been modified.
                     */
                    ED_TRACE(("==> dp:%p,cnt:%d,errno:%d\n", dp, cnt, errno))
                    if (pty_died(bp)) {
                        goto start_again;
                    }
                    break;
                }
                buf[cnt] = '\0';
                ED_TRACE(("\tlength:%d,buffer:\"%s\"\n", cnt, buf))
                p_update(bp, buf);
                updated = TRUE;
            }
        }

        /*
         *  Update screen if necessary and make sure that the current buffer is restored
         *  so that cursor goes at right place and in case we exit this loop
         */
        set_curbp(ocurbp);
        if (updated) {
            vtupdate();
        }

    } while (updated && !io_typeahead());

    --entry;
}


/*
 *  Process insertx() implementation
 */
int
pty_inserts(const char *buf, int len)
{
    const DISPLAY_t *dp = curbp->b_display;

    if (dp) {
        const uint32_t flags = dp->d_flags;

        if (PF_LOCALECHO & flags) {
            pty_write(buf, len);
            return ((PF_NOINSERT & flags) ? 0 : 1);
        }
        return 1;
    }
    return -1;                                  /* not attached */
}


/*
 *  Process self_insert() implementation
 */
int
pty_insertc(int ch)
{
    const DISPLAY_t *dp = curbp->b_display;

    if (dp) {
        const uint32_t flags = dp->d_flags;

        if (PF_LOCALECHO & flags) {
            unsigned char buf[1];
            buf[0] = (unsigned char)ch;         /* FIXME/MCHAR??? */
            pty_write((const char *)buf, 1);
            return ((PF_NOINSERT & flags) ? 0 : 1);
        }
        return 1;
    }
    return -1;                                  /* not attached */
}


/*
 *  Process insert_process() implementation
 */
void
pty_write(const char *buf, int len)
{
    DISPLAY_t *dp = curbp->b_display;

    ED_TRACE(("pty_write(dp:%p,len:%d,buf:%s)\n", dp, len, buf))
    if (dp) {                                   /* FIXME/MCHAR??? */
        int n;

        errno = 0;
        do {                                    /* EINTR safe */
            if ((n = fileio_write(dp->d_pipe_out, buf, len)) >= 0) {
                if ((len -= n) <= 0) {
                    break;
                }
                buf += n;
            }
        } while (n >= 0 || (n < 0 && errno == EINTR));
    }
}

/*end*/
