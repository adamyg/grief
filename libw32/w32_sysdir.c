#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sysdir_c,"$Id: w32_sysdir.c,v 1.7 2015/02/19 00:17:33 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 1998 - 2015, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 */

#include "win32_internal.h"
#include "win32_misc.h"

#include <shlobj.h>                             /* SHXxxxx functions */
#include <shlguid.h>
#include <unistd.h>


int
w32_getsysdir(
    int id, char *buf, int maxlen)
{
    char szPath[ MAX_PATH ];
    HRESULT hres;
    int len;

    switch (id) {
    case SYSDIR_TEMP:
        id = CSIDL_INTERNET_CACHE;
        break;
    default:
        return -1;
    }

    hres = SHGetSpecialFolderPath(NULL, szPath, id, FALSE);
    if (SUCCEEDED(hres) &&
            (len = (int)strlen(szPath)) <= maxlen) {
        (void) strcpy(buf, (const char *)szPath);
        return len;
    }
    return (-1);
}


const char *
w32_selectfolder(
    const char *strMessage, char *szBuffer)
{
    char const * Result = NULL;
    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pList;

    /* Throw display dialog */
    memset(&BrowseInfo, 0, sizeof(BrowseInfo));
    BrowseInfo.hwndOwner = NULL;                /* XXX */
    BrowseInfo.pszDisplayName = szBuffer;
    BrowseInfo.lpszTitle = strMessage;
    BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
    pList = SHBrowseForFolder(&BrowseInfo);

    /* Convert from MIDLISt to real string path */
    if (pList != NULL) {
        SHGetPathFromIDList(pList, szBuffer);
        CoTaskMemFree(pList);
        Result = szBuffer;
    }
    return (Result);
}
