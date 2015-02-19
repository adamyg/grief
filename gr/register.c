#include <edidentifier.h>
__CIDENT_RCSID(gr_register_c,"$Id: register.c,v 1.29 2014/10/22 02:33:17 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: register.c,v 1.29 2014/10/22 02:33:17 ayoung Exp $
 * Event handlers.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "register.h"

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* x_command_name */
#include "debug.h"                              /* trace_...() */
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "macros.h"
#include "macros.h"                             /* macro_lookup */
#include "main.h"
#include "tty.h"

#define MAX_REGISTER    (REG_MAX + 1)

typedef struct _registration {
    MAGIC_t             r_magic;
#define REGISTER_MAGIC      MKMAGIC('R','e','G',' ')
    TAILQ_ENTRY(_registration)
                        r_node;
    accint_t            r_ident;
    const char *        r_name;
} registration_t;

static int              register_ident(void);
static registration_t * register_find(REGISTERLIST_t *rq, const char *macro);
static int              register_remove(REGISTERLIST_t *rq, const char *macro);

static REGISTERLIST_t   x_reglst[MAX_REGISTER]; /* trigger lists */

accint_t                xf_interval = 0;

time_t                  x_time_last_key = 0;

int                     x_trigger_level = 0;    /* trigger nesting */


/*  Function:           register_init
 *      runtime initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
register_init(void)
{
    REGISTERLIST_t *rq;
    unsigned type;

    for (type = 0, rq = x_reglst; type < MAX_REGISTER; ++type, ++rq) {
        TAILQ_INIT(rq);
    }
}


/*  Function:           register_shutdown
 *      runtime subsystem shutdown.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
register_shutdown(void)
{
    REGISTERLIST_t *rq;
    registration_t *r;
    unsigned type;

    for (type = 0, rq = x_reglst; type < MAX_REGISTER; ++type, ++rq) {
        while (NULL != (r = TAILQ_FIRST(rq))) {
            assert(REGISTER_MAGIC == r->r_magic);
            TAILQ_REMOVE(rq, r, r_node);
            chk_free((void *)r);
        }
    }
}


/*  Function:           do_call_registered_macro
 *      call_registered_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: call_registered_macro - Invoke registered macro callbacks.

        int
        call_registered_macro(int type)

    Macro Description:
        The 'call_registered_macro()' primitive invokes all of
        functions which have register against the particular event
        type 'type'.

        See <register_macro> for the particulars on the different
        registered event types.

    Macro Parameters:
        type - Event type to be invoked.

    Macro Returns:
        The 'call_registered_macro' returns nothing.

    Macro Portability:
        The set of available events differ between systems.

    Macro See Also:
        register_macro
 */
void
do_call_registered_macro(void)  /* (int num) */
{
    const int type = register_ident();
    int ret = -1;

    if (type >= 0) {
        ret = trigger(type);
    }
    acc_assign_int(ret);
}


/*  Function:           do_register_macro
 *       register_macro and reregister_macro primitives.
 *
 *  Parameters:
 *      unique - *TRUE* if reregister_macro(), otherwise *FALSE*.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: register_macro - Register a callback procedure.

        int
        register_macro(int type,
            string macro, [int local = FALSE])

    Macro Description:
        The 'registered_macro()' primitive registers a function to be
        invoked upon the trigger of the event type 'type'. Multiple
        macros may be associated against any particular event type,
        in which case upon execution they are called in *FIFO* order.

        The argument 'macro' is the name of a macro to be invoked
        upon the event being triggered. If 'local' is specified and
        is non-zero, then the macro is only registered against the
        current buffer, otherwise the event may be triggered even
        when the current buffer is not selected.

        Registered macros may be invoked using
        <call_registered_macro> and removed by using the
        <unregister_macro> primitive.

(start table,format=basic)
        [Constant           [Description                                ]
        REG_TYPED           Character typed.
        REG_EDIT            Different file edited.
        REG_ALT_H           ALT-H pressed in response to a prompt.
        REG_UNASSIGNED      Unassigned key pressed.
        REG_IDLE            Idle time expired.
        REG_EXIT            About to exit.
        REG_NEW             New file edited and readin.
        REG_CTRLC           CTRL-C (SIGINT) pressed during macro.
        REG_INVALID         Invalid key pressed during response input.
        REG_INTERNAL        Internal error.
        REG_MOUSE           Mouse callback.
        REG_PROC_INPUT      Process input available.
        REG_KEYBOARD        Keyboard buffer empty.
        REG_STARTUP         Startup complete.
        REG_BOOKMARK        Bookmark dropped/deleted.
        REG_INSERT_MODE     Insert mode has changed.
        REG_BUFFER_MOD      Buffer has been modified.
        REG_BUFFER_WRITE    Buffer write operation.
        REG_BUFFER_RENAME   Buffer rename operation.
        REG_BUFFER_DELETE   buffer delete operation.
        REG_FILE_SAVE       File write request.
        REG_FILE_WRITTEN    File write completion.
        REG_FILE_CHANGE     File external change.
        REG_SIGUSR1         SIGUSR1 signal trap.
        REG_SIGUSR2         SIGUSR2 signal trap.
(end table)

    Macro Parameters:
        type - Event type against which to register.

        name - Name of the macro to be registered.

        local - Optional int, Whether the trigger is of local or
            global scope. Note currently local is only effective on
            REG_TYPED.

    Macro Returns:
        The 'registered_macro()' primitive returns 1 if the macro was
        successfully registered, 0 if already registered, otherwise -1
        on error.

    Macro Portability:
        The set of available events differ between systems.

    Macro See Also:
        register_macro

 *<<GRIEF>>
    Macro: reregister_macro - Register a unique callback procedure.

        int
        reregister_macro(int type,
                string macro, [int local = FALSE])

    Macro Description:
        The 'reregistered_macro()' primitive registers a unique
        function to be invoked upon the trigger of the event type
        'type'. Similar to <register_macro> yet only permits a single
        instance of the given function tp be registered.

        This primitive allows macros to unconditionally register
        handlers without need to know whether a previous instance has
        been installed, unlike <register_macro> which shall permit
        multiple instances to exist.

        See <register_macro> for the particulars on the different
        registered event types.

    Macro Parameters:
        type - Event type against which to register.

        name - Name of the macro to be registered.

        local - Optional int, Whether the trigger is of local or
            global scope. Note currently local is only effective on
            REG_TYPED.

    Macro Returns:
        The 'reregistered_macro()' primitive returns 1 if the macro was
        uniquely registered, 0 if already registered, otherwise -1 on
        error.

    Macro Portability:
        The set of available events differ between systems.

    Macro See Also:
        register_macro
 */
void
do_register_macro(int unique)   /* (int num, string macro-name, [int local = FALSE]) */
{
    const int type = register_ident();
    int ret = -1;

    if (type >= 0) {
        const char *macro = get_str(2);
        const accint_t local = (REG_TYPED == type ? get_xinteger(3, FALSE) : FALSE);
        const char *resolved = macro_resolve(macro);
        REGISTERLIST_t *rq =
            (local ? (curbp ? &curbp->b_register : NULL) : x_reglst + type);

        if (resolved && rq) {
            ret = 0;
            if (FALSE == unique || NULL == register_find(rq, resolved)) {
                const size_t len = strlen(resolved) + 1;
                registration_t *r;

                if (NULL != (r = chk_alloc(sizeof(registration_t) + len))) {
                    r->r_magic = REGISTER_MAGIC;
                    r->r_name = (const char *)(r + 1);
                    memcpy(r + 1, resolved, len);
                    TAILQ_INSERT_HEAD(rq, r, r_node);
                }
                ret = 1;
            }
        }
    }
    acc_assign_int(ret);
}


void
register_attach(BUFFER_t *bp)
{
    TAILQ_INIT(&bp->b_register);
}


void
register_detach(BUFFER_t *bp)
{
    REGISTERLIST_t *rq = &bp->b_register;
    registration_t *r;

    while (NULL != (r = TAILQ_FIRST(rq))) {
        assert(REGISTER_MAGIC == r->r_magic);
        TAILQ_REMOVE(rq, r, r_node);
        chk_free((void *)r);
    }
}


/*  Function:           do_unregister_macro
 *      unregister_macro primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: unregister_macro - Remove a registered macro.

        int
        unregister_macro(int type,
            string macro, [int local = FALSE])

    Macro Description:
        The 'unregistered_macro()' primitive removes a previously
        registered macro.

        If a particular macro has been registered multiple times than
        for each successful registration a corresponding unregister
        must occur to remove all instances; unregister_macro may be
        called in a loop until all instances are removed.

    Macro Parameters:
        type - Event type against which to unregister.

        name - Name of the macro to be unregistered.

        local - Optional int, Whether the trigger is of local or
            global scope. Note currently local is only effective on
            REG_TYPED.

    Macro Returns:
        The 'unregistered_macro()' primitive returns 1 if macro was
        registered and has now been unregistered, otherwise 0.

    Macro Portability:
        The set of available events differ between systems.

    Macro See Also:
        register_macro
 */
void
do_unregister_macro(void)       /* (int num, string macro-name, [int local = FALSE]) */
{
    const int type = register_ident();
    int ret = -1;

    if (type >= 0) {
        const char *macro = get_str(2);
        const accint_t local = (REG_TYPED == type ? get_xinteger(3, FALSE) : FALSE);
        const char *resolved = macro_resolve(macro);
        REGISTERLIST_t *rq =
            (local ? (curbp ? &curbp->b_register : NULL) : x_reglst + type);

        if (resolved && rq) {
            ret = register_remove(rq, resolved);
        }
    }
    acc_assign_int(ret);
}


/*  Function:           register_ident
 *      validate the register macro parameters.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Register index, otherwise -1.
 */
static int
register_ident(void)
{
    const accint_t type = get_xinteger(1, -1);

    if (type < 0 || type >= MAX_REGISTER) {
        errorf("%s: invalid parameters.", x_command_name);
        return -1;
    }
    return type;
}


/*  Function:           register_find
 *      Find the specified macro.
 *
 *  Parameters:
 *      rq - Queue head
 *      macro - Macro name
 *
 *  Returns:
 *      Node if found, otherwise NULL.
 */
static registration_t *
register_find(REGISTERLIST_t *rq, const char *macro)
{
    registration_t *r;

    TAILQ_FOREACH(r, rq, r_node) {
        assert(REGISTER_MAGIC == r->r_magic);
        if (0 == strcmp(r->r_name, macro)) {
            return r;
        }
    }
    return NULL;
}


/*  Function:           register_remove
 *      Remove the specified macro.
 *
 *  Parameters:
 *      rq - Queue head
 *      macro - Macro name
 *
 *  Returns:
 *      nothing
 */
static int
register_remove(REGISTERLIST_t *rq, const char *macro)
{
    registration_t *r;

    TAILQ_FOREACH(r, rq, r_node) {
        assert(REGISTER_MAGIC == r->r_magic);
        if (0 == strcmp(r->r_name, macro)) {
            TAILQ_REMOVE(rq, r, r_node);
            chk_free((void *)r);
            return 1;
        }
    }
    return 0;
}



/*  Function:           trigger
 *      Call the registered macros of a specific type. If no macro registered then
 *      return 0. Otherwise return value of the last macro called. Must be an integer).
 *
 *  Parameters:
 *      type -  Event identifier/type.
 *
 *  Returns:
 *      int - return value of the trigger'ed function(s). If the event in no
 *          registered handler then -1 is returned.
 */
int
trigger(int type)
{
    return triggerx(type, NULL);
}


/*  Function:           triggerx
 *      Call the registered macros of a specific type. If no macro
 *      registered then return 0. Otherwise return value of the last
 *      macro called. Must be an integer).
 *
 *  Parameters:
 *      type - Event identifier/type.
 *      fmt - Trigger argument format specification.
 *      ... - Optional parameters.
 *
 *   Returns:
 *      int - return value of the trigger'ed function(s). If the
 *      event in no registered hanlders then -1 is returned.
 *
 *<<GRIEF>> [callback]
    Macro: _invalid_key - Invalid key event.

        void
        _invalid_key()

    Macro Description:
        The '_invalid_key' callback is a legacy BRIEF interface.

        This interface is provided for BRIEF compatibility, if the macro
        '_invalid_key' exists, it shall be called instead of any
        registered macros. When invoked the keystroke awaiting is the
        invalid key.

    Macro Parameters:
        none

    Macro Returns:
        The '_invalid_key' callback should return an integer value.

    Macro Portability:
        BRIEF compatibility.

    Macro See Also:
        register_macro
 */
int
triggerx(int type, const char *fmt, ...)
{
    const int omsglevel = x_msglevel;
    char t_buf[MAX_CMDLINE], buf[MAX_CMDLINE];
    REGISTERLIST_t *grq, *brq = NULL;
    const registration_t *r;

    if (type < 0 || type >= MAX_REGISTER) {
        return -1;
    }

    grq = x_reglst + type;

    if (REG_UNASSIGNED == type) {
        static const char _invalid_key[] = "_invalid_key";

        if (macro_lookup(_invalid_key)) {
            /*
            //  BRIEF compatibility,
            //      if the macro '_invalid_key' exists, it shall be
            //      called instead of any registered macros. When
            //      invoked the keystroke awaiting is the invalid key.
            */
            execute_str(_invalid_key);
            return acc_get_ival();
        }

    } else if (REG_TYPED == type) {
        if (curbp) {
            brq = &curbp->b_register;
        }
    }

    if (NULL == TAILQ_FIRST(grq) &&
            (NULL == brq || NULL == TAILQ_FIRST(brq))) {
        return -1;
    }

    if (fmt) {
        va_list ap;                             /* with (additional) arguments */

        buf[0] = ' ';                           /* delimiter */
        va_start(ap, fmt);
        vsxprintf(buf + 1, sizeof(buf) - 1, fmt, ap);
        trace_trigger(type, buf + 1);
        va_end(ap);

    } else {
        trace_trigger(type, NULL);
    }

    x_msglevel = 0;
    ++x_trigger_level;
    acc_assign_int(0);                          /* default return */

    if (brq) {
        TAILQ_FOREACH(r, brq, r_node) {         /* buffer local */
            assert(REGISTER_MAGIC == r->r_magic);
            if (NULL == fmt) {
                execute_str(r->r_name);

            } else {
                strxcpy(t_buf, r->r_name, sizeof(t_buf));
                strxcat(t_buf, buf, sizeof(t_buf));
                execute_str(t_buf);
            }
        }
    }

    TAILQ_FOREACH(r, grq, r_node) {             /* global local */
        assert(REGISTER_MAGIC == r->r_magic);
        if (NULL == fmt) {
            execute_str(r->r_name);

        } else {
            strxcpy(t_buf, r->r_name, sizeof(t_buf));
            strxcat(t_buf, buf, sizeof(t_buf));
            execute_str(t_buf);
        }
    }

    --x_trigger_level;
    x_msglevel = omsglevel;

    if (REG_CTRLC == type && acc_get_ival()) {
        check_exit();
    }

    return acc_get_ival();
}


/*  Function:           trigger_idle
 *      idle event trigger.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
trigger_idle(void)
{
    int old_col, old_row;

    if (x_plevel < 2 && !x_prompting) {
        ttposget(&old_row, &old_col);
        trigger(REG_IDLE);
        vtupdate();
        ttmove(old_row, old_col);
        ttflush();
    }
}


/*  Function:           inq_idle_default
 *      inq_idle_default primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_idle_default - Retrieve idle interval.

        int
        inq_idle_default()

    Macro Description:
        The 'inq_idle_default()' primitives retrieves the current
        keyboard idle interval, see <set_idle_default>.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_idle_default()' primitives returns the current
        value of the interval timer.

    Macro Portability:
        n/a

    Macro See Also:
        inq_idle_default, set_idle_default
 */
void
inq_idle_default(void)          /* int () */
{
    acc_assign_int((accint_t) xf_interval);
}


/*  Function:           do_set_idle_default
 *      set_idle_default primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_idle_default - Set idle interval.

        int
        set_idle_default(int internal = 0)

    Macro Description:
        The 'set_idle_default()' primitives set the keyboard idle
        interval as a measure of seconds betwen the last keystroke
        and when the *REG_IDLE* event is generated, see
        <register_macro> for details.

    Macro Parameters:
        interval - Integer idle interval in seconds, if omitted the
            system default shall be utilised.

    Macro Returns:
        The 'set_idle_default()' primitives returns the previous
        value of the interval timer.

    Macro Portability:
        n/a

    Macro See Also:
        inq_idle_default, set_idle_default
 */
void
do_set_idle_default(void)       /* int (int internal) */
{
    acc_assign_int((accint_t) xf_interval);
    if ((xf_interval = get_xinteger(1, 0)) < 0) {
        xf_interval = 0;
    }
}


/*  Function:           inq_idle_time
 *      inq_idle_time primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_idle_time - Retrieve keyboard idle time.

        int
        inq_idle_time()

    Macro Description:
        The 'inq_idle_time()' primitives retrieves the number of
        seconds since the user last pressed a key, representing the
        time keyboard input has been idle.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_idle_time()' primitive returns the idle timer in
        seconds.

    Macro Portability:
        n/a

    Macro See Also:
        inq_idle_default, set_idle_default
 */
void
inq_idle_time(void)             /* int () */
{
    if (0 == x_time_last_key) {
        acc_assign_int((accint_t) 0);
    } else {
        acc_assign_int((accint_t) (time(NULL) - x_time_last_key));
    }
}
/*end*/
