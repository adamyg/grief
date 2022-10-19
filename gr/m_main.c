#include <edidentifier.h>
__CIDENT_RCSID(gr_m_main_c,"$Id: m_main.c,v 1.13 2022/07/10 13:13:08 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_main.c,v 1.13 2022/07/10 13:13:08 cvsuser Exp $
 * Initialisation and primary processing loop.
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

#include "main.h"                               /* public header */

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "display.h"
#include "eval.h"                               /* get_int/str.. */
#include "getkey.h"
#include "keyboard.h"
#include "register.h"                           /* trigger() */
#include "symbol.h"
#include "window.h"


/*  Function:           main_loop
 *      main macro processing loop, exiting upon an exit() request.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
main_loop(void)
{
    while (x_plevel >= 0) {
        x_plevel = 0;
        x_msglevel = 0;
        do_process();
        check_exit();
    }
}


void
check_exit(void)
{
    if (buf_anycb() == FALSE) {
        gr_exit(EXIT_SUCCESS);
    }
}


/*  Function:           do_procss
 *      process primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: process - Invoke a Grief engine

        void
        process()

    Macro Description:
        The 'process()' primitive invokes an instance of the Grief
        command-loop recursively by accepting keystrokes and calling
        the functions assigned thru the associated key bindings 
        (see assign_to_key).

        'process' is usually invoked after building the required
        environment including buffers and/or windows with an
        associated keyboard. Process nesting may occur to any depth, 
        within the bounds of Griefs maximum nesting level. The

        current command loop executes until the session is
        terminated by an <exit> command.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        exit, suspend
 */
void
do_process(void)                /* () */
{
    int last_plevel;

    x_panycb = 0;
    ++x_plevel;
    vtupdate_cursor();
    vtupdate2(TRUE);                            /* ignore typehead() */
    while (1) {
        last_plevel = x_plevel;
        (void) key_execute(io_next(0));
        if (x_plevel < last_plevel) {
            return;
        }
        vtupdate();                             /* refresh */
    }
}


/*  Function:           do_exit
 *      exit primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: exit - Exit current process level.

        void
        exit([string y_or_w])

    Macro Description:
        The 'exit()' primitive signals to leave the current process
        loop, which if the top level loop causes Grief to
        terminate to the operating system.

        Upon leaving the top level prior to exiting when modified
        buffers are present the user shall be prompted as to
        whether or not the buffers should be saved, as follows;

>           1 buffer has not been saved. Exit [ynw]?

        The user may have all modified buffers saved prior to
        terminating by replying with either "W" or "w", otherwise
        terminate without saving any buffers using "Y" or "y".
        Alternatively the user may reject the exit signal
        altogether with a reply of "N" or "n".

    Macro Parameters:
        y_or_w - Optional string that is applied to the answer
            regarding the action to occur when modified buffers
            are detected; 'Y' or 'y' Grief shall exit without
            saving any modifiers buffers, whereas 'W' or 'w' all
            modified buffers are written. 
            
            Any other values other than "YyWW" shall cause the
            user to be prompted. In addition, the parameter is
            ignored if 'exit' is being applied to a secondary
            process loop, which would not cause Grief to terminate.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        abort, process
 */
void
do_exit(void)                   /* ([string yw], [TODO/int kill_subprocesses]) */
{
    if (0 == --x_plevel) {                      /* top level */
        const char *yw = get_xstr(1);

        x_panycb = 0;
        if (yw) {
            switch (*yw) {                      /* [YyWw] */
            case 'Y': case 'y':
                x_panycb = 'y';
                break;
            case 'W': case 'w':
                x_panycb = 'w';
                break;
            }
        }
    }
}


/*  Function:           do_abort
 *      abort primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: abort - Abnormal process

        void 
        abort()

    Macro Description:
        The 'abort()' primitive immediately exits Grief.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        exit, process
 */
void
do_abort(void)                  /* void () */
{
    trigger(REG_ABORT);
    vtclose(FALSE);
    exit(1);
}


/*  Function:           do_suspend
 *      Suspend primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: suspend - Suspend current process.

        void
        suspend()

    Macro Description:
        The 'suspend()' primitive pretends user typed 'ctrl-z' by
        sending a *SIGTSTP* to the controlled processing,
        effectively suspending Grief by placing it in the
        background.

        Note!:
        This primitive shall only function on systems which
        support job control, for example 'unix'.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        process, exit
 */
void
do_suspend(void)                /* int () */
{
    int ret = -1;

#if defined(SIGTSTP)
    ret = kill(getpid(), SIGTSTP);
#elif defined(SIGSTOP)
    ret = kill(getpid(), SIGSTOP);
#endif
    acc_assign_int(ret);
}

/*end*/
