#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_mknod_c,"$Id: w32_mknod.c,v 1.18 2025/02/03 02:27:35 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mknod() system calls.
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"

#include <sys/cdefs.h>
#include <unistd.h>

/*
//  NAME
//      mknod - make a directory, a special file, or a regular file
//
//  SYNOPSIS
//
//      #include <sys/stat.h>
//
//      int mknod(const char *path, mode_t mode, dev_t dev);
//
//  DESCRIPTION
//
//      The mknod() function shall create a new file named by the pathname to which the
//      argument path points.
//
//      The file type for path is OR'ed into the mode argument, and the application shall
//      select one of the following symbolic constants:
//
//          Name        Description
//
//          S_IFIFO     FIFO-special
//
//          S_IFCHR     Character-special (non-portable)
//
//          S_IFDIR     Directory (non-portable)
//
//          S_IFBLK     Block-special (non-portable)
//
//          S_IFREG     Regular (non-portable)
//
//                      The only portable use of mknod() is to create a FIFO-special
//                      file. If mode is not S_IFIFO or dev is not 0, the behavior of
//                      mknod() is unspecified.
//
//      The permissions for the new file are OR'ed into the mode argument, and may be
//      selected from any combination of the following symbolic constants:
//
//          Name        Description
//
//          S_ISUID     Set user ID on execution.
//
//          S_ISGID     Set group ID on execution.
//
//          S_IRWXU     Read, write, or execute (search) by owner.
//
//          S_IRUSR     Read by owner.
//
//          S_IWUSR     Write by owner.
//
//          S_IXUSR     Execute (search) by owner.
//
//          S_IRWXG     Read, write, or execute (search) by group.
//
//          S_IRGRP     Read by group.
//
//          S_IWGRP     Write by group.
//
//          S_IXGRP     Execute (search) by group.
//
//          S_IRWXO     Read, write, or execute (search) by others.
//
//          S_IROTH     Read by others.
//
//          S_IWOTH     Write by others.
//
//          S_IXOTH     Execute (search) by others.
//
//          S_ISVTX     On directories, restricted deletion flag.
//
//      The user ID of the file shall be initialized to the effective user ID of the
//      process. The group ID of the file shall be initialized to either the effective
//      group ID of the process or the group ID of the parent directory. Implementations
//      shall provide a way to initialize the file's group ID to the group ID of the parent
//      directory. Implementations may, but need not, provide an implementation-defined way
//      to initialize the file's group ID to the effective group ID of the calling process.
//      The owner, group, and other permission bits of mode shall be modified by the file
//      mode creation mask of the process. The mknod() function shall clear each bit whose
//      corresponding bit in the file mode creation mask of the process is set.
//
//      If path names a symbolic link, mknod() shall fail and set errno to [EEXIST].
//
//      Upon successful completion, mknod() shall mark for update the st_atime, st_ctime,
//      and st_mtime fields of the file. Also, the st_ctime and st_mtime fields of the
//      directory that contains the new entry shall be marked for update.
//
//      Only a process with appropriate privileges may invoke mknod() for file types other
//      than FIFO-special.
//
//  RETURN VALUE
//
//      Upon successful completion, mknod() shall return 0. Otherwise, it shall return -1,
//      the new file shall not be created, and errno shall be set to indicate the error.
//
//  ERRORS
//
//      The mknod() function shall fail if:
//
//      [EACCES]
//          A component of the path prefix denies search permission, or write permission is
//          denied on the parent directory.
//
//      [EEXIST]
//          The named file exists.
//
//      [EINVAL]
//          An invalid argument exists.
//      [EIO]
//          An I/O error occurred while accessing the file system.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path argument.
//
//      [ENAMETOOLONG]
//          The length of a pathname exceeds {PATH_MAX} or a pathname component is longer
//          than {NAME_MAX}.
//
//      [ENOENT]
//          A component of the path prefix specified by path does not name an existing
//          directory or path is an empty string.
//
//      [ENOSPC]
//          The directory that would contain the new file cannot be extended or the file
//          system is out of file allocation resources.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [EPERM]
//          The invoking process does not have appropriate privileges and the file type is
//          not FIFO-special.
//
//      [EROFS]
//          The directory in which the file is to be created is located on a read-only file system.
//
//      The mknod() function may fail if:
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          Pathname resolution of a symbolic link produced an intermediate result whose
//          length exceeds {PATH_MAX}.
*/
LIBW32_API int
mknod(const char *path, int mode, int dev)
{
    __CUNUSED(path)
    __CUNUSED(mode)
    __CUNUSED(dev)
    errno = EIO;
    return -1;
}


LIBW32_API int
mknodA(const char *path, int mode, int dev)
{
    __CUNUSED(path)
    __CUNUSED(mode)
    __CUNUSED(dev)
    errno = EIO;
    return -1;
}


LIBW32_API int
mknodW(const wchar_t *path, int mode, int dev)
{
    __CUNUSED(path)
    __CUNUSED(mode)
    __CUNUSED(dev)
    errno = EIO;
    return -1;
}

/*end*/
