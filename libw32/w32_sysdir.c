#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sysdir_c,"$Id: w32_sysdir.c,v 1.13 2021/06/10 06:13:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 interface support
 *
 * Copyright (c) 2007, 2012 - 2021 Adam Young.
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


LIBW32_API int
w32_getsysdir(int id, char *buf, int maxlen)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t wpath[WIN32_PATH_MAX];

        if (w32_getsysdirW(id, wpath, _countof(wpath)) > 0) {
            return w32_wc2utf(wpath, buf, maxlen);
        }
        return -1;
    }
#endif

    return w32_getsysdirA(id, buf, maxlen);
}


LIBW32_API int
w32_getsysdirA(int id, char *buf, int maxlen)
{
    char t_path[ MAX_PATH ], *path = buf;
    HRESULT hres;
    int len;

    if (NULL == buf || maxlen < 4) {
        return -1;
    }

    switch (id) {
    case SYSDIR_TEMP:
        id = CSIDL_INTERNET_CACHE;
        break;
    default:
        return -1;
    }

    if (maxlen < MAX_PATH) path = t_path;
    hres = SHGetSpecialFolderPathA(NULL, path, id, FALSE);
    if (SUCCEEDED(hres)) {
        len = (int)strlen(path);
        if (path == buf) {                      // direct
            return len;                        
        } else if (len < maxlen) {              // indirect
            (void) strcpy(buf, (const char *)t_path);
            return len;
        }
    }
    return -1;
}


LIBW32_API int
w32_getsysdirW(int id, wchar_t *buf, int maxlen)
{
    wchar_t t_path[ MAX_PATH ], *path = buf;
    HRESULT hres;
    int len;

    if (NULL == buf || maxlen < 4) {
        return -1;
    }

    switch (id) {
    case SYSDIR_TEMP:
        id = CSIDL_INTERNET_CACHE;
        break;
    default:
        return -1;
    }

    if (maxlen < MAX_PATH) path = t_path;
    hres = SHGetSpecialFolderPathW(NULL, path, id, FALSE);
    if (SUCCEEDED(hres)) {
        len = (int)wcslen(path);
        if (path == buf) {                      // direct
            return len;                        
        } else if (len < maxlen) {              // indirect
            (void) wcscpy(buf, (const wchar_t *)t_path);
            return len;
        }
    }
    return -1;
}


LIBW32_API const char *
w32_selectfolder(const char *message, char *szBuffer)
{
    char const * Result = NULL;
    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pList;

    /* Throw display dialog */
    memset(&BrowseInfo, 0, sizeof(BrowseInfo));
    BrowseInfo.hwndOwner = NULL;                /* XXX */
    BrowseInfo.pszDisplayName = szBuffer;
    BrowseInfo.lpszTitle = message;
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

/*end*/
