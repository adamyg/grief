#include <edidentifier.h>
__CIDENT_RCSID(gr_m_system_c,"$Id: m_system.c,v 1.21 2023/09/10 16:35:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_system.c,v 1.21 2023/09/10 16:35:52 cvsuser Exp $
 * Basic system primitives.
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

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if defined(HAVE_TIME_H)
#if !defined(HAVE_SYS_TIME_H) || defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif
#endif

#include "m_system.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "eval.h"                               /* get_str() ... */
#include "symbol.h"
#include "system.h"                             /* sys_...() */


/*  Function:       do_getenv
 *      getenv() primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: getenv - Retrieve an environment variable.

        string
        getenv(string name)

    Macro Description:
        The 'getenv()' primitive searches the environment of the calling
        process for the environment variable name if it exists and
        returns the value of that environment variable. If the
        specified environment variable cannot be found, an empty string
        shall be returned.


    Macro Parameters:
        name - String containing the name of the environment variable.

    Macro Returns:
        The 'getenv()' primitive returns the value of the corresponding
        environment variable, otherwise an empty string.

    Macro Portability:
        n/a

    Macro See Also:
        putenv, inq_environment
 */
void
do_getenv(void)                 /* string (string name) */
{
    const char *cp;

    cp = ggetenv(get_str(1));
    acc_assign_str(cp ? cp : "", -1);
}


/*  Function:       do_putenv
 *      putenv() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: putenv - Set an environment variable.

        void
        putenv(string name, [string value])

    Macro Description:
        The 'putenv()' primitive shall use the string arguments to set
        the value of an environment variable.

        The arguments must construct a string of the form

>           "name=value"

        This construction may be stated by one or two means. Firstly by
        the explicit string arguments 'name' and 'value', or
        alternatively a single argument 'name' already correctly formed
        containing both components separated by an embedded '=' within
        the string.

    Macro Parameters:
        name - String containing the name of the environment variable
            and if 'value' is omitted should also contain the
            associated value separated by an equals '='.

        value - Optional value to be assigned.

    Macro Returns:
        Upon successful completion, putenv shall return one on success,
        otherwise zero and set 'errno' to indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        getenv
 */
void
do_putenv(void)                 /* int (string name, [string val]) */
{
    const char *name = get_str(1);
    const char *value = get_xstr(2);
    int ret;

    errno = 0;
    if (value) {
        ret = gputenv2(name, value);
    } else {
        ret = gputenv(name);
    }
    system_call(ret);
    acc_assign_int(ret ? 0 : 1);                /* success(1) otherwise (0) */
}


/*  Function:       do_getpid
 *      getpid() primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: getpid - Retrieve the process identifier.

        int
        getpid()

    Macro Description:
        The 'getpid()' primitive shall return the process ID of the
        calling process.

    Macro Parameters:
        none

    Macro Returns:
        The 'getpid()' primitive shall always be successful returning
        the current process identifier.

    Macro Portability:
        n/a

    Macro See Also:
        getuid
 */
void
do_getpid(void)                 /* int (void) */
{
    acc_assign_int((accint_t) sys_getpid());
}


/*  Function:       do_getuid
 *      getuid() primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: getuid - Retrieve the user identifier.

        int
        getuid()

    Macro Description:
        The 'getuid()' primitive shall return the real user ID of the
        calling process.

    Macro Parameters:
        none

    Macro Returns:
        The 'getuid()' primitive shall always be successful returning
        the current user identifier.

    Macro Portability:
        n/a

    Macro See Also:
        getpid, geteuid
 */
void
do_getuid(void)                 /* int (void) */
{
    acc_assign_int((accint_t) sys_getuid());
}



/*  Function:       do_geteuid
 *      geteuid() primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: geteuid - Retrieve the effective user identifier.

        int
        geteuid()

    Macro Description:
        The 'geteuid()' primitive shall return the effective user ID
        of the calling process.

    Macro Parameters:
        none

    Macro Returns:
        The 'geteuid()' primitive shall always be successful
        returning the current effective user identifier.

    Macro Portability:
        n/a

    Macro See Also:
        getpid, getuid
 */
void
do_geteuid(void)                /* int (void) */
{
    acc_assign_int((accint_t) sys_geteuid());
}


/*  Function:       inq_clock
 *      inq_clock() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_clock - Retrieve the user identifier.

        int
        inq_clock()

    Macro Description:
        The 'inq_clock()' primitive retrieves an approximation of the
        current CPU time used by the current edit session.

        This primitive is an interface to the system clock() function,
        yet normalises the returned value to microseconds; to determine
        the number of seconds used, divide by the system constant
        *CLOCKS_PER_SEC*.

        Note loops every 72 minutes.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_clock' returns the time in microseconds since start of
        the current Grief instance.

        If the processor time used is not available or its value cannot
        be represented, the function returns the value -1.

    Macro Portability:
        n/a

    Macro See Also:
        time, sleep
 */
void
inq_clock(void)                 /* (void) */
{
#if defined(__MINGW32__)                        /* CLOCKS_PER_SEC issues */
#undef  CLOCKS_PER_SEC
#define CLOCKS_PER_SEC          1000
#endif
    accint_t value = 0;

#if defined(__MAKEDEPEND__)
#undef  CLOCKS_PER_SEC
#define CLOCKS_PER_SEC          1000
#endif

#if !defined(CLOCKS_PER_SEC)
#error inq_clock: CLOCK_PER_SEC is undefined ...
    
#else
    if (1000000 == CLOCKS_PER_SEC) {            /* POSIX standard */
        value = clock();

    } else if (1000 == CLOCKS_PER_SEC) {        /* WIN32/DOS */
        value = (clock() * 1000);

    } else {
        // POSIX requires that CLOCKS_PER_SEC equals 1000000 independent of the actual resolution.
        value = clock();
    }
#endif

    acc_assign_int(value);
}
/*end*/
