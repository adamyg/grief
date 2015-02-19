#include <edidentifier.h>
__CIDENT_RCSID(gr_sysfile_c,"$Id: sysfile.c,v 1.7 2014/10/22 02:33:21 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sysfile.c,v 1.7 2014/10/22 02:33:21 ayoung Exp $
 * System level file support.
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
#include <errno.h>
#include <edfileio.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "system.h"                             /* sys_...() */


/*  Function:           sys_basename
 *      sys_basename() strips off the leading part of a path name, leaving only
 *      the final component of the name, which is assumed to be the file name.
 *
 *  Parameters:
 *      name - Filename.
 *
 *  Returns:
 *      Basename component of the filename.
 */
const char *
sys_basename(const char *name)
{
    const char *cp1 = strrchr(name, '/');

#if defined(DOSISH)         /* X: */
    const char *cp2 = strrchr(name, '\\');
    if (cp2 > cp1) {
        return cp2 + 1;
    }
    if (!cp1) {
        const unsigned char ch0 = *((unsigned char *)name);

        if (ch0 && ':' == name[1] && isalpha(ch0)) {
            return name + 2;
        }
    }
#endif

    if (cp1) {
        return cp1 + 1;
    }

#if defined(_VMS)
    if ((cp1 = strrchr(name, ']')) != NULL) {
        return cp1 + 1;
    }
    if ((cp1 = strrchr(name, ':')) != NULL) {
        return cp1 + 1;
    }
#endif
    return name;
}


/*  Function:           sys_pathend
 *      Retrieve the address of the last character of the directory component
 *      within the specified path.
 *
 *  Parameters:
 *      path - File path.
 *
 *  Returns:
 *      Address of the last character within the directory (ie the delimiter),
 *      otherwise NULL.
 */
const char *
sys_pathend(const char *path)
{
    const char *end = path;

    while (*path) {
        const int ch = *path;
#if defined(DOSISH)
        if (ch == '/' || ch == '\\') {
#else
        if (ch == '/') {
#endif
            end = path;
        }
        ++path;
    }
    return end;
}



int
sys_isabspath(const char *path)
{
    if (path) {
        const unsigned char ch0 = *((unsigned char *)path);

#if (defined(DOSISH) && !defined(__CYGWIN__)) || defined(WIN32)
        if (ch0 && ':' == path[1] && isalpha(ch0) &&
                ('/' == path[2] || '\\' == path[2])) {
#else
        if (FILEIO_PATHSEP == ch0) {            /* absolute path */
#endif
            return TRUE;
        }
    }
    return FALSE;
}


const char *
sys_pathdelimiter(void)
{
    static const char delim[2] = {FILEIO_DIRDELIM, 0};
    return delim;
}


const char *
sys_pathseparator(void)
{
    static const char sep[2] = {FILEIO_PATHSEP, 0};
    return sep;
}
/*end*/
