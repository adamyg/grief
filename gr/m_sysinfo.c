#include <edidentifier.h>
__CIDENT_RCSID(gr_m_sysinfo_c,"$Id: m_sysinfo.c,v 1.14 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_sysinfo.c,v 1.14 2025/02/07 03:03:21 cvsuser Exp $
 * System information primitives.
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

#if defined(unix) || defined(__unix__) || defined(_AIX) || defined(linux) || \
        defined(WIN32) || defined(__APPLE__)
#define HAVE_UTSNAME
#include <sys/utsname.h>
#endif

#include "m_sysinfo.h"

#include "accum.h"
#include "debug.h"
#include "symbol.h"
#include "sysinfo.h"


/*  Function:           inq_home
 *      inq_home primitive.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_home - Retrieve the user home directory.

        string
        inq_home()

    Macro Description:
        The 'inq_home()' primitive retrieves the current users home
        directory.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_home()' primitive returns a string containing the
        users home directory.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_username, getenv
 */
void
inq_home(void)                  /* string () */
{
    acc_assign_str(sysinfo_homedir(NULL, -1));
}


/*  Function:           inq_tmpdir
 *      inq_tmpdir primitive.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_tmpdir - Get the temporary-file directory.

        string
        inq_tmpdir()

    Macro Description:
        The 'inq_tmpdir()' primitive retrieves the temporary file
        directory. The directory is determined by the current
        environment, which is host specific, for example on UNIX
        systems the default value of this property is typically "/tmp"
        or "/var/tmp"; on Microsoft Windows systems it is typically
        "c:\temp".

     Windows::

        On XP and greater the system folder 'local application data'
        directory is queried.

        If this fails, the function checks for the existence of
        environment variables in the following order and uses the
        first path found:

         o The path specified by the *TMP* environment variable.
         o The path specified by the *TEMP* environment variable.
         o The path specified by the *USERPROFILE* environment variable.
         o The Windows directory.

     UNIX like Systems::

        *TMPDIR* is the canonical Unix environment variable that
        should be used to specify a temporary directory for scratch
        space.

        Other forms accepted are *TEMP*, *TEMPDIR* and *TMP*, but
        these alternatives are used more commonly by non-POSIX
        operating systems or non-conformant programs.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_tmpdir()' primitive returns a string containing the
        temporary file directory.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        getenv
 */
void
inq_tmpdir(void)                /* string () */
{
    acc_assign_str(sysinfo_tmpdir());
}


/*  Function:           inq_username
 *      inq_username primitive.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_username - Retrieve the user name.

        string
        inq_username()

    Macro Description:
        The 'inq_username()' primitive retrieves the current user name.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_username()' primitive returns a string containing
        the current user name.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_hostname, getenv
 */
void
inq_username(void)              /* string () */
{
    acc_assign_str(sysinfo_username(NULL, -1));
}


/*  Function:           inq_hostname
 *      inq_hostname primitive.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_hostname - Retrieve the local host name.

        string
        inq_hostname()

    Macro Description:
        The 'inq_hostname()' primitive retrieves the name of the
        local host.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_hostname()' primitive returns a string containing
        the local host name.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_username, getenv
 */
void
inq_hostname(void)              /* string () */
{
    acc_assign_str(sysinfo_hostname(NULL, -1));
}


/*  Function:           do_uname
 *      uname primitive.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: uname - Retrieve system information.

        int
        uname([string &sysname], [string &nodename],
                [string &version], [string &release],
                [string &machine])

    Macro Description:
        The 'uname()' primitive retrieves the strings corresponding to the
        result of a uname system call. On non-POSIX systems, these
        strings maybe blank unless there is an equivalent value available.

    Macro Parameters:
        sysname  - System name.
        nodename - Network node name.
        version  - Kernel version
        release  - Kernel release.
        machine  - Machine type.

    Macro Returns:
        The 'uname()' primitive returns on 0 otherwise, -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        version
*/
void
do_uname(void)                  /* int ([string &sysname], [string &nodename], [string &version],
                                            [string &release], [string &machine]) */
{
#if defined(HAVE_UTSNAME)
#if defined(unix) || defined(__unix__) || defined(_AIX) || defined(linux) || \
        defined(WIN32) || defined(__APPLE__)
    struct utsname u = {0};

    if (uname(&u) != -1) {
        argv_assign_str(1, u.sysname);
        argv_assign_str(2, u.nodename[0] ? u.nodename : sysinfo_hostname(NULL, -1));
        argv_assign_str(3, u.version);
        argv_assign_str(4, u.release);
        argv_assign_str(5, u.machine);
        acc_assign_int(0);
        return;
    }
#endif
#endif /*HAVE_UTSNAME*/
    acc_assign_int(-1);
}

/*end*/
