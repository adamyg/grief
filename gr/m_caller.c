#include <edidentifier.h>
__CIDENT_RCSID(gr_m_caller_c,"$Id: m_caller.c,v 1.16 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_caller.c,v 1.16 2025/02/07 03:03:21 cvsuser Exp $
 * Caller primitives.
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

#include "m_caller.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* mac_... */
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */

typedef TAILQ_HEAD(CallerList, Caller)
                        CallerList_t;           /* caller list */

typedef struct Caller {
    MAGIC_t             c_magic;                /* structure magic */
#define CALLER_MAGIC        MKMAGIC('C', 'a', 'L', 'l')
    TAILQ_ENTRY(Caller) c_node;                 /* list node */
    int                 c_level;                /* macro level/depth */
    int                 c_hits;                 /* cache hits */
    int                 c_length;               /* name length, in bytes; excluding terminator */
    char                c_name[1];              /* name buffer */
} Caller_t;

static int                      caller_release(int level);

static int                      x_callers = -1;
static CallerList_t             x_callerlist;


/*  Function:           do_inq_called
 *      inq_called primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_called - Get the name of the calling macro.

        string 
        inq_called()

    Macro Description:
        The 'inq_called()' primitive returns the callers name, 
        allowing macros to differentiate between being called
        directly from the command prompt, or from another macro.

        This primitive is provided to support <replacement> macros
        and to allow macros to determine whether user prompts
        <get_parm> are suitable and <message> output may be required.

    Macro Returns:
        The 'inq_calling()' primitive returns a string containing the
        name of the macro which called the current macro, otherwise
        an empty string where invoked from the keyboard.

        The <set_calling_name> can be used to modify the value
        returned in-turn controlling the behaviour of the macros which
        are then invoked.

    Macro See Also:
        set_calling_name, replacement
 */
void
inq_called(void)                /* (void) */
{
    const struct mac_stack *stk2 = (mac_sd >= 2 ? mac_stack + (mac_sd - 2) : NULL);
    const char *caller = (stk2 ? (stk2->caller ? stk2->caller : stk2->name) : NULL);

    acc_assign_str(caller ? caller : "");
}


/*  Function:           do_set_calling_name
 *      set_calling_name primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_calling_name - Set the name of the calling macro.

        void 
        set_calling_name(string name = NULL)

    Macro Description:
        The 'set_calling_name()' primitive sets the calling name of
        the current macro, which is the name that would be returned
        by 'inq_name()' within any macros called. If omitted then the
        name is cleared and returned to the original name.

        'set_calling_name' is used to modify the value returned
        in-turn controlling the behaviour of the macros which are then
        invoked.

        Another common use is within replacement macros, to forward
        the original callers name onto the next macro.

>           set_calling_name(inq_called());

    Macro Returns:
        Nothing

    Macro See Also:
        inq_called, replacement
 */
void
do_set_calling_name(void)       /* (string name = NULL) */
{
    caller_release(mac_sd);                     /* release resources */

    if (mac_sd >= 1) {
        struct mac_stack *stk1 = mac_sp;
        const char *caller = get_xstr(1);

        if (caller) {                           /* set_calling_name(NULL) -- extension */

            if (stk1->caller && 0 == strcmp(caller, stk1->caller)) {
                return;                         /* no change */
            }

            if (0 == *caller) {                 /* set_calling_name("") */
                caller = "";
                                                /* set_calling_name(<self>) */
            } else if (0 == strcmp(caller, stk1->name)) {
                caller = stk1->name;
                                                /* set_calling_name(inq_called()) */
            } else if (mac_sd >= 2 && 0 == strcmp(caller, stk1[-1].name)) {
                caller = stk1[-1].name;

            } else {                            /* set_calling_name(<name>) */
                const int length = (int)strlen(caller);
                CallerList_t *callerlist = &x_callerlist;
                Caller_t *cp = NULL;

                TAILQ_FOREACH(cp, callerlist, c_node) {
                    assert(CALLER_MAGIC == cp->c_magic);
                    if (length == cp->c_length &&
                            0 == memcmp(cp->c_name, caller, length)) {
                        caller = cp->c_name;
                        ++cp->c_hits;
                        break;
                    }
                }

                if (NULL == cp &&               /* new name */
                        NULL != (cp = chk_calloc(sizeof(Caller_t) + length, 1))) {
                    TAILQ_INSERT_TAIL(callerlist, cp, c_node);
                    cp->c_magic = CALLER_MAGIC;
                    memcpy(cp->c_name, caller, length + 1);
                    cp->c_length = length;
                    cp->c_level = mac_sd;
                    cp->c_hits = 0;
                    caller = cp->c_name;
                    ++x_callers;
                }
            }
        }

        stk1->caller = caller;                  /* associate/clear */
    }
}


static int
caller_release(int level)
{
    CallerList_t *callerlist = &x_callerlist;

    if (-1 == x_callers) {
        if (mac_sd >= 1) {
            TAILQ_INIT(callerlist);
            x_callers = 0;
        }
    } else {
        Caller_t *cp, *cpnext;

        if (NULL != (cp = TAILQ_FIRST(callerlist)))
            do {                                /* TODO - only age low usage items */
                assert(CALLER_MAGIC == cp->c_magic);
                cpnext = TAILQ_NEXT(cp, c_node);
                if (cp->c_level > level) {
                    cp->c_magic = ~CALLER_MAGIC;
                    TAILQ_REMOVE(callerlist, cp, c_node);
                    chk_free((void *)cp);
                    --x_callers;
                }
            } while (NULL != (cp = cpnext));
    }
    return 0;
}

/*end*/
