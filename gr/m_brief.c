#include <edidentifier.h>
__CIDENT_RCSID(gr_m_brief_c,"$Id: m_brief.c,v 1.11 2024/12/05 19:00:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_brief.c,v 1.11 2024/12/05 19:00:11 cvsuser Exp $
 * BRIEF compatibility.
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

#include "m_brief.h"

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */
#include "main.h"                               /* ms_cnt */


/*<<GRIEF>>
    Macro: inq_brief_level - Retrieve the editor nesting level.

        int
        inq_brief_level()

    Macro Description:
        The 'inq_brief_level()' primitive retrieves the number of
        copies of GRIEF running within the current session.

        The original implementation returned the total number of
        instances in memory, whereas this emulation only reports the
        number of instances visible with the associated terminal.

        This function is provided for compatibility using the
        'getenv' interface; see the <getenv> primitive and the
        'brief' macro module for details.

    Macro Parameters:
        none

    Macro Returns:
        The current number of active editor sessions.

    Macro Portability:
        Provided for BRIEF compatibility, retrieving the current
        *GRLEVEL* environment variable level.

    Macro See Also:
        inq_environment
 */
void
inq_brief_level(void)
{
    //see: replacement brief.cr
    acc_assign_int(-1);
}


/*<<GRIEF>>
    Macro: set_mouse_action - Set keyboard mouse handler.

        int
        set_mouse_action(string mouse_handler)

    Macro Description:
        The 'set_mouse_button()' primitive is reserved for future BRIEF
        compatibility.

        The 'set_mouse_action()' primitive sets the function mouse_handler
        as the mouse action handler within the current keyboard.

>           void
>           mouse_handler(
>                   int event,      // event code; see below
>                   int modifier,   // modifier keys
>                   int parm2,      // either line or scrollbar position
>                   int parm3       // either column or thumb position
>                   )
>           {
>               switch(event) {
>               case BTN1_MOVE:
>                   break;
>               case BTN2_MOVE:
>                   break;
>               case BTN3_MOVE:
>                   break;
>
>               case BTN1_DOWN:
>                   break;
>               case BTN2_DOWN:
>                   break;
>               case BTN3_DOWN:
>                   break;
>
>               case BTN1_UP:
>                   break;
>               case BTN2_UP:
>                   break;
>               case BTN3_UP:
>                   break;
>
>               case BTN1_CLICK:
>                   break;
>               case BTN2_CLICK:
>                   break;
>               case BTN3_CLICK:
>                   break;
>
>               case BTN1_DBLCLK:
>                   break;
>               case BTN2_DBLCLK:
>                   break;
>               case BTN3_DBLCLK:
>                   break;
>               }
>           }

    Macro Parameters:
        mouse_handler - A string containing the name of the function to be
            associated with the current keyboard.

    Macro Returns:
        Returns the previous mouse action.

    Macro Portability:
        n/a

    Macro See Also:
        inq_mouse_action, inq_keyboard
 */
void
do_set_mouse_action(void)       /* string (string) */
{
    //TODO
    acc_assign_str("", 0);
}


/*<<GRIEF>>
    Macro: inq_mouse_action - Retrieve the keyboard mouse handler.

        string inq_mouse_action()

    Macro Description:
        The 'inq_mouse_button()' primitive is reserved for future
        BRIEF compatibility.

        The 'inq_mouse_action()' primitive retrieves the name of the
        current mouse action handler. This function can be used to
        save the current mouse handle prior to pushing a new keyboard.

    Macro Parameters:
        none

    Macro Returns:
        Returns the current mouse action.

    Macro Portability:
        n/a

    Macro See Also:
        set_mouse_action
 */
void
inq_mouse_action(void)          /* string () */
{
    //TODO
    acc_assign_str("", 0);
}



/*<<GRIEF>>
    Macro: set_mouse_type - Sets the mouse type.

        int
        set_mouse_type()

    Macro Description:
        The 'set_mouse_type()' primitive sets the mouse type.

    Macro Parameters:
        type - Integer stating the current mouse type.

(start table,format=nd)
            [Value  [Description                ]
          ! 0       No mouse.
          ! 1       One-button mouse.
          ! 2       Two-button mouse.
          ! 3       Three-button mouse.
(end table)

        button1 - Option integer indicates which button shall be
            treated as the first button. A value of zero indicates
            the left-most, with a value of one indicates the
            right-most button. By default the left-most button is the
            mouse button one.

    Macro Returns:
        none

    Macro Portability:
        n/a

    Macro See Also:
        inq_mouse_type
 */
void
do_set_mouse_type(void)         /* void ([int type], [int button1]) */
{
    //TODO
}


/*<<GRIEF>>
    Macro: inq_mouse_type - Retrieve the button type.

        int
        inq_mouse_type()

    Macro Description:
        The 'inq_mouse_type()' primitive retrieves the current mouse type.

    Macro Parameters:
        none

    Macro Returns:
        Returns the current mouse type.

(start table,format=nd)
            [Value  [Description                ]
          ! 0       No mouse.
          ! 1       One-button mouse.
          ! 2       Two-button mouse.
          ! 3       Three-button mouse.
(end table)

    Macro Portability:
        n/a

    Macro See Also:
        inq_mouse_type
 */
void
inq_mouse_type(void)            /* int () */
{
    //TODO
    acc_assign_int(2);
}


/*<<GRIEF>>
    Macro: inq_btn2_action - Retrieve the second button action.

        int
        inq_btn2_action()

    Macro Description:
        The 'inq_btn2_action()' primitive is reserved for future
        BRIEF compatibility.

        The 'inq_btn2_action()' primitive retrieves the current
        action status of the second mouse button. The function can be
        used to assign a mouse action handler into a newly pushed
        keyboard, which shall be automatically removed upon the
        keyboard destruction.

    Macro Parameters:
        none

    Macro Returns:
        Returns the current action button status.

    Macro Portability:
        n/a
 */
void
inq_btn2_action(void)           /* string () */
{
    //TODO
    acc_assign_int(0);
}


/*<<GRIEF>>
    Macro: set_btn2_action - Set the second button action.

        int
        set_btn2_action([int action])

    Macro Description:
        The 'set_btn2_action()' primitive is reserved for future
        BRIEF compatibility.

        The 'set_btn2_action()' primitive sets or toggles the default
        action for the second mouse button. If 'action' is zero, the
        Quick-Edit action will be the default. If 'action' is
        non-zero, the Quick-Menu action will be the default. If
        'action' is omitted, the current setting shall be toggled.

    Macro Parameters:
        action - Optional integer, the new Quick-Menu button action.

    Macro Returns:
        Returns the previous action button status.

    Macro Portability:
        n/a
 */
void
do_set_btn2_action(void)        /* int ([int action]) */
{
    //TODO
}


/*<<GRIEF>> [env]
    Macro: inq_environment - Retrieve an environment variable.

        string
        inq_environment(string name)

    Macro Description:
        The 'inq_environment()' primitive shall search the
        environment of the calling process for the environment
        variable name if it exists and return the value of the
        environment variable. If the specified environment variable
        cannot be found, an empty string shall be returned.

        This function is provided for compatibility using the
        'getenv' interface; see the <getenv> primitive and the
        'brief' macro module for details.

    Macro Parameters:
        name - String containing the name of the environment variable.

    Macro Returns:
        The 'inq_environment()' primitive returns the value of the
        corresponding environment variable, otherwise an empty string.

    Macro Portability:
        n/a

    Macro See Also:
        getenv
 */
//  void
//  inq_environment(void)       /* string (string name) */
//  {
//      // see: brief.cr
//      do_getenv();
//  }


/*<<GRIEF>> [file]
    Macro: del - Delete a file.

        int
        del(string name)

    Macro Description:
        The 'del()' primitive deletes the specified file 'name'.

        This function is provided for compatibility using the
        'remove' interface; see the <remove> primitive and the
        'brief' macro module for details.

    Macro Parameters:
        name - String containing the file to be removed.

    Macro Returns:
        remove, delete_buffer
 */
//  void
//  do_del(void)                /* int (string name) */
//  {
//      // see: brief.cr
//      do_remove();
//  }


/*<<GRIEF>> [proc]
    Macro: dos - Create a sub-shell.

        int
        dos([string cmd], [int use_shell], [string callback])

    Macro Description:
        The 'dos()' primitive executes the specified command 'cmd',
        if omitted starts an interactive command shell.

        This function is provided for compatibility using the <shell>
        interface; see the <shell> and the 'brief' macro module for
        details.

    Macro Parameters:
        cmd - String containing the command to be executed.

        shell - Optional integer stating whether a shell should be
            utilised, if non-zero use of a shell shall be omitted if
            feasible.

        callback - Optional string containing the name of a macro to be
            executed on the completion of the task. If stated the
            command is intended to run as a background task,
            otherwise in the foreground.

    Macro Returns:
        The 'dos()' primitive returns the completion value of the
        corresponding command.

        A negative value denotes an execution failure; on failure
        errno contains a value indicating the type of error that has
        been detected including.

(start table,format=nd)
        [Constant       [Description                                          ]

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

      ! ENOENT          The command or one of its components does not exist.

      ! ENOEXEC         An executable is not in a recognized format, is
                        for the wrong architecture, or has some other
                        format error that means it cannot be executed.

      ! ENOMEM          Insufficient kernel memory was available.
(end table)

    Macro Portability:
        n/a

    Macro See Also:
        shell
 */
//  void
//  do_dos(void)                /* int ([string cmd], [int use_shell], [string macro]) */
//  {
//      // see: brief.cr
//  }

/*end*/
