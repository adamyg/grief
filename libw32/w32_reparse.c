#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_reparse_c,"$Id: w32_reparse.c,v 1.6 2020/04/20 23:17:16 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 reparse support
 *
 * Copyright (c) 2007, 2012 - 2019 Adam Young.
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
 * Sourced originally from a public domain implementation and highly.
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501
#endif

#include "win32_internal.h"
#include "win32_ioctl.h"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static void
replace_dir(char *buf, int maxlen, const char *original, const char *replacement)
{
    assert(buf && maxlen > 0);

    if (maxlen-- > 0) {                         /* reserve nul */
        const char *d1 = strrchr(original, '/'), *d2 = strrchr(original, '\\'),
            *d = (d1 > d2 ? d1 : d2);           /* last delimitor; if any */

                                                /* note: wont deal with parent directory references */
        if (d++) {                              /* include delimitor within result */
            const int dirlen =
                ((d - original) < maxlen ? (d - original) : maxlen);

            memcpy(buf, original, dirlen);
            strncpy(buf + dirlen, replacement, maxlen - dirlen);

        } else {
            strncpy(buf, replacement, maxlen);
        }
        buf[maxlen] = 0;                        /* nul terminate */
    }
}


static void
memxcpy(char *dst, const char *src, int len, int maxlen)
{
    if (len >= maxlen) len = maxlen - 1;        /* limit to upper limit; plus nul terminator */
    (void) memcpy(dst, src, len);
    dst[len]=0;
}


LIBW32_API int
w32_reparse_read(const char *name, char *buf, int maxlen)
{
    BYTE reparseBuffer[MAX_REPARSE_SIZE];       /* XXX: warning: owc crash if = {0} under full optimisation */
    PREPARSE_DATA_BUFFER rdb = (PREPARSE_DATA_BUFFER)reparseBuffer;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    DWORD returnedLength = 0;
    int ret = -1;

    assert(24 == sizeof(REPARSE_DATA_BUFFER));
    memset(reparseBuffer, 0, sizeof(reparseBuffer));

    if ((fileHandle = CreateFile(name, 0,       /* open the file image */
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /*
     *  retrieve reparse details
     */
    if (DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT,
            NULL, 0, rdb, sizeof(reparseBuffer), &returnedLength, NULL)) {
        if (IsReparseTagMicrosoft(rdb->ReparseTag)) {
            char resolved[1024] = { 0 };        /* resolved name */
            int length;

            switch (rdb->ReparseTag) {
            case IO_REPARSE_TAG_SYMLINK:
                //
                //  Symbolic links.
                //
                if ((length = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength) >= 4) {
                    const size_t offset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* symlink = rdb->SymbolicLinkReparseBuffer.PathBuffer + offset;                 
                    size_t len = wcstombs(resolved, symlink, sizeof(resolved) - 1);

                    if (len != (size_t)-1) {
                        assert(len < sizeof(resolved));
                        resolved[len] = 0;

                        //
                        //  XXX: rdb->SymbolicLinkReparseBuffer.Flags & 1) 
                        //          SYMLINK_FLAG_RELATIVE
                        //
                        if (len >= 2 &&         /* eg: "C://link" */
                                ':' == resolved[1] && isalpha(resolved[0])) {
                            memxcpy(buf, resolved, len, maxlen);

                        } else if (0 == strncmp(resolved, "\\??\\", 4)) {
                                                /* eg: "C:/Users/All Users" */
                            memxcpy(buf, resolved + 4, len - 4, maxlen);

                        } else {                /* relative */
                            replace_dir(buf, maxlen, name, resolved);
                        }
                        ret = 0;
                    }
                }
                break;

            case IO_REPARSE_TAG_MOUNT_POINT:
                //
                //  Mount points and junctions.
                //
                if ((length = rdb->MountPointReparseBuffer.SubstituteNameLength) > 0) {
                    const size_t offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
                    const wchar_t* mount = rdb->MountPointReparseBuffer.PathBuffer + offset;
                    size_t len = wcstombs(resolved, mount, sizeof(resolved) - 1);

                    if (len != (size_t)-1) {
                        assert(len < sizeof(resolved));
                        resolved[len] = 0;

                        if (0 == strncmp(resolved, "\\??\\", 4)) {
                            /* absolute. */
                            if (0 == strncmp(resolved, "\\??\\Volume{", 11)) {
                                                /* mount, resolve volume. */
                                wchar_t volume[1024], pathNames[1024 * 4];
                                DWORD pathLen = 0;

                                wcscpy(volume, L"\\\\?\\");
                                wcscpy(volume + 4, mount + 4);

                                if (GetVolumePathNamesForVolumeNameW(volume, pathNames, _countof(pathNames), &pathLen) && pathLen > 0) {
                                    if ((size_t)-1 != (len = wcstombs(resolved, pathNames, _countof(resolved)-1))) {
                                        memxcpy(buf, resolved, len, maxlen);
                                        ret = 0;
                                    }
                                }
                            } else {            /* junction, remove trailing \??\ */
                                memxcpy(buf, resolved + 4, len - 4, maxlen);
                                ret = 0;
                            }
                        } else {                /* relative. */
                            replace_dir(buf, maxlen, name, resolved);
                            ret = 0;
                        }
                    }
                }
                break;
            }
        }
    }

    CloseHandle(fileHandle);
    return ret;
}

/*end*/
