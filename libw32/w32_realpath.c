#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_realpath_c,"$Id: w32_realpath.c,v 1.4 2018/10/11 01:49:01 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 directory access services ...
 *
 *      realpath
 *
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT            0x0501          /* reparse */
#endif

#define _DIRENT_SOURCE
#include "win32_internal.h"
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>


//  NAME
//	realpath - resolve a pathname
//
//  SYNOPSIS
//	#include <stdlib.h> [optional]
//	char *realpath(const char *restrict file_name, char *restrict resolved_name);
//
//  DESCRIPTION
//	The realpath() function shall derive, from the pathname pointed to by file_name, an absolute
//	pathname that names the same file, whose resolution does not involve '.', '..', or symbolic
//	links. The generated pathname shall be stored as a null-terminated string, up to a maximum of
//	{PATH_MAX} bytes, in the buffer pointed to by resolved_name.
//
//	If resolved_name is a null pointer, the behavior of realpath() is implementation-defined.
//
//  RETURN VALUE
//	Upon successful completion, realpath() shall return a pointer to the resolved name. Otherwise,
//	realpath() shall return a null pointer and set errno to indicate the error, and the contents of
//	the buffer pointed to by resolved_name are undefined.
//
//  ERRORS
//	The realpath() function shall fail if:
//
//	[EACCES]
//	    Read or search permission was denied for a component of file_name.
//	[EINVAL]
//	    The file_name argument is a null pointer.
//	[EIO]
//	    An error occurred while reading from the file system.
//	[ELOOP]
//	    A loop exists in symbolic links encountered during resolution of the file_name argument.
//	[ENAMETOOLONG]
//	    The length of the file_name argument exceeds { PATH_MAX} or a pathname component is longer 
//	    than {NAME_MAX}.
//	[ENOENT]
//	    A component of file_name does not name an existing file or file_name points to an empty string.
//	[ENOTDIR]
//	    A component of the path prefix is not a directory.
//
//	The realpath() function may fail if:
//
//	[ELOOP]
//	    More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the file_name argument.
//	[ENAMETOOLONG]
//	    Pathname resolution of a symbolic link produced an intermediate result whose length exceeds {PATH_MAX}.
//	[ENOMEM]
//	    Insufficient storage space is available.
//
LIBW32_API char *
w32_realpath(const char *path, char *resolved_path /*[PATH_MAX]*/)
{
    return w32_realpath2(path, resolved_path, PATH_MAX);
}

LIBW32_API char *
w32_realpath2(const char *path, char *resolved_path, int maxlen)
{
    char *result = NULL;

    if (path && *path) {
        if (NULL != (result = resolved_path)) { // user buffer.
            if (maxlen < 4) {                   // "X:/\0"
                errno = EINVAL;                 // invalid length.
                result = NULL;
            }
        } else {                                // glibc style extension.
            result = (char *)malloc(maxlen = 1024 /*PATH_MAX*/);
                // malloc() shall set errno on error.
        }

        if (result) {
            const size_t size =                 // resolve, including .. and . components
                GetFullPathNameA(path, maxlen, result, 0);

            //
            //  GetFullPathNameA() returns a size larger than buffer if buffer is too small
            if (size >= (size_t)maxlen) {
                if (result != resolved_path) {
                    free((void *)result);
                    result = (char *)malloc(maxlen = (size + 1 /*plus null*/));
                        // malloc() shall set errno on error.

                    if (result) {
                        const size_t new_size =
                            GetFullPathNameA(path, maxlen, result, 0);
                        if (new_size >= (size_t)maxlen) {
                            free((void *)result);
                            errno = ENAMETOOLONG;
                            result = NULL;
                            return NULL;
                        }
                    }
                } else {                        // resolved_path buffer isn't big enough
                    errno = ENAMETOOLONG;
                    result = NULL;
                }
            }

            //
            //  GetFullPathNameA() returns 0 if some path resolve problem occured
            if (0 == size) {
                switch (GetLastError()) {
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION:
                case ERROR_PRIVILEGE_NOT_HELD:
                    errno = EACCES;  break;
                case ERROR_FILE_NOT_FOUND:
                    errno = ENOENT;  break;
                case ERROR_PATH_NOT_FOUND:
                case ERROR_INVALID_DRIVE:
                    errno = ENOTDIR; break;
                default:
                    errno = EIO;
                    break;
                }

                if (result != resolved_path) {
                    free((void *)result);
                }
                result = NULL;
            }

            //
            //  Success, verify the result.
            if (result) {
                struct stat sb = { 0 };
                if (w32_stat(result, &sb)) {
                    if (result != resolved_path) {
                        free(result);
                    }
                    result = NULL;
                        // stat() shall set errno on error.
                }
            }
        }
    } else {
        errno = EINVAL;                         // invalid source.
    }

    if (result) {
        if (result[0] && result[1] == ':') {    // normalize directory slashes.
            char *cursor;
            for (cursor = result; *cursor; ++cursor) {
                if ('\\' == *cursor) *cursor = '/';
            }
        }
    }
    return result;
}

/*end*/
