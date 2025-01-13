#include <edidentifier.h>
__CIDENT_RCSID(gr_m_fileio_c,"$Id: m_fileio.c,v 1.21 2025/01/13 15:12:17 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_fileio.c,v 1.21 2025/01/13 15:12:17 cvsuser Exp $
 * File i/o primitives.
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
#include <eddir.h>

#include <libstr.h>                             /* str_...()/sxprintf() */
#include "../libvfs/vfs.h"			/* FIXME */

#include "m_fileio.h"
#include "m_file.h"                             /*acc_assign_stat*/

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "symbol.h"
#include "sysinfo.h"
#include "system.h"
#include "word.h"

static int              importflags(const char *flags);


/*  Function;           do_fopen
 *      fopen primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fopen - Open a stream.

        int
        fopen(string path, int|string flags,
                    [int mode = 0644], [int bufsiz])

    Macro Description:
        The 'fopen()' primitive shall open the file whose pathname is
        the string 'path', and associates a stream with it.

        The 'flags' argument contains a string. If the string is one
        of the following, the file shall be opened in the indicated
        mode.

            r -     Open file for reading.

            w -     Truncate to zero length or create file for writing.

            a -     Append; open or create file for writing at end-of-file.

            r+ -    Open file for update (reading and writing).

            w+ -    Truncate to zero length or create file for update.

            a+ -    Append; open or create file for update, writing at end-of-file.

            wx -    create text file for writing with exclusive access.

            wbx -   create binary file for writing with exclusive access.

            w+x -   create text file for update with exclusive access.

            w+bx -  create binary file for update with exclusive access.

    Macro Parameters:
        path - String containing either the full or relative path
            name of a file to be opened.

        flags - Creation flags, see above.

        mode - Optional creation mode.

        bufsiz - Optionally stream buffer size, in bytes.

    Macro Returns:
        Upon successful completion, fopen() shall return a handle to the
        stream. Otherwise, -1 shall be returned, and 'errno' shall be set
        to indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fclose, fread, fwrite
 */
void
do_fopen(void)                  /* (string path, int|string flags, [int mode], [int bufsize]) */
{
    const char *path = get_str(1);
    const int flags = get_xinteger(2, -1);
    const int mode = get_xinteger(3, 0644);
    int fd;

    if ((fd = vfs_open(path, (flags > 0 ? flags : importflags(get_xstr(2))), mode)) < 0) {
        system_call(-1);
    }
    acc_assign_int(fd);
}


/*  Function;           importflags
 *      Process the fopen() flag specification.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static int
importflags(const char *flags)
{
    int nflags = -1;

    if (flags && flags[0]) {
        /*
         *  primary mode
         */
        switch (*flags) {
        case 'r': nflags = O_RDONLY; break;
        case 'w': nflags = O_WRONLY; break;
        case 'a': nflags = O_WRONLY | O_APPEND; break;
        default:
            return -1;
        }
        ++flags;

        /*
         *  modifiers
         */
        while (*flags) {
            switch (*flags) {
            case '+':
                if (nflags & O_RDONLY) {        /* r+ - read/write */
                    nflags &= ~O_RDONLY;
                    nflags |= O_RDWR;

                } else if (nflags & O_WRONLY) {
                    if (nflags & O_APPEND) {    /* a+ - append for writing and reading */
                        nflags &= ~O_WRONLY;
                        nflags |= O_RDWR;

                    } else {                    /* w+ - truncate for writing and reading */
                        nflags &= ~O_WRONLY;
                        nflags |= O_RDWR|O_TRUNC;
                    }
                }
                break;
            case 'b':
#if defined(O_BINARY)
                nflags |= O_BINARY;
#endif
                break;
            case 't':
#if defined(O_TEXT)
                nflags |= O_TEXT;
#endif
                break;
            case 'x':   /* C11 */
                nflags |= O_EXCL;               /* exclusive */
                break;
            default:
                return -1;
            }
        }
    }
    return nflags;
}



/*  Function;           do_fmkopen
 *      fmkopen primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fmktemp - Make a unique filename

        int
        fmktemp(string template);

    Macro Description:
        The 'fmktemp()' primitive shall replace the contents of the
        string 'template' by a unique filename, and return a stream for
        the file open for reading and writing.

        The primitive is equivalent to the 'mkstemp()'.

        The function thus prevents any possible race condition between
        testing whether the file exists and opening it for use. The
        string in template should look like a filename with six trailing
        'X' s; 'fmktemp' replaces each 'X' with a character from the
        portable filename character set.

        The characters are chosen such that the resulting name does not
        duplicate the name of an existing file at the time of a call to
        'fmktemp'.

    Macro Returns:
        Upon successful completion, fmktemp() shall return an open stream
        and return the resulting 'filename'. Otherwise, -1 shall be
        returned if no suitable file could be created.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fopen, mktemp
 */
void
do_fmktemp(void)                /* int (string template) */
{
#if (0)
    char path[MAX_PATH];
    int handle, ret = -1;

    strxcpy(path, get_str(1), sizeof(path));
    if ((handle = sys_mkstemp(path)) >= 0) {    /* TODO, vfs_reopen() */
        if ((ret = vfs_reopen(handle, path, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0) {
            argv_assign_str(1, path);

        } else {
            fileio_close(handle);
        }
    }
    acc_assign_int(ret);
#endif
    acc_assign_int(-1);
}


/*  Function;           do_fclose
 *      fclose primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fclose - Close a stream.

        int
        fclose(int handle)

    Macro Description:
        The 'fclose()' primitive shall cause the stream pointed to by
        stream to be flushed and the associated file to be closed.
        Any unwritten buffered data for the stream shall be written
        to the file; any unread buffered data shall be discarded.

    Macro Returns:
        On successful completion, 'fclose()' shall return 0;
        otherwise, it shall return -1 and set 'errno' to indicate the
        error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fopen, fread, fwrite
 */
void
do_fclose(void)                 /* (int handle) */
{
    int handle = get_xinteger(1, -1);
    int ret;

    if ((ret = vfs_close(handle)) < 0) {
        system_call(-1);
    }
    acc_assign_int(ret);
}


/*  Function;           do_fread
 *      fread primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fread - Read from a stream.

        int
        fread(int handle, string buffer,
                [int bufsiz = BUFSIZ], [int null = ' '])

    Macro Description:
        The 'fread()' primitive shall read into the array pointed to
        by ptr up to bufsize elements whose size is specified by size
        in bytes, from the stream referenced by 'handle'.

    Macro Parameters:
        handle - Stream handle.

        buffer - String buffer to be written.

        bufsiz - Optional length of buffer to be read.

    Macro Returns:
        Upon successful completion, fread() shall return the number
        of elements successfully read which is less than 'bufsiz'
        only if a read error or end-of-file is encountered.

        If 'bufsiz' is 0, fread() shall return 0 and the contents of
        the array and the state of the stream remain unchanged.
        Otherwise, if a read error occurs, the error indicator for
        the stream shall be set, and 'errno' shall be set to indicate
        the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fopen, fwrite
 */
void
do_fread(void)                  /* (int handle. string buffer, [int bufsiz], [int null = ' ']) */
{
    int handle = get_xinteger(1, -1);
    int bufsiz = get_xinteger(3, BUFSIZ);       /* bufsiz (optional) */
    int null = get_xinteger(4, 0);
    int ret = -1;

    if (bufsiz < 0) {
        system_errno(EINVAL);
        ret = -1;

    } else if (0 == bufsiz) {
        ret = 0;

    } else {
        /*
         *  Use accumulator at temporary buffer
         */
        char *buffer;
        int len = 0;

        if (NULL == (buffer = acc_expand(bufsiz)) ||
                (len = vfs_read(handle, buffer, bufsiz)) < 0) {
            system_call(-1);
            ret = -1;

        } else {
            ret = len;

            if (null <= 0 || null > 255) null = ' ';
            while (len-- > 0) {
                if (0 == *buffer) {             /* convert NUL's */
                    *buffer = (unsigned char)null;
                }
                ++buffer;
            }
            *buffer = 0;

            acc_assign_strlen(ret);
            if (!isa_undef(2)) {                /* assign result */
                sym_assign_str(get_symbol(2), buffer /*length*/);
            }
        }
    }
    acc_assign_int(ret);
}


/*  Function;           do_fwrite
 *      fwrite primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fwrite - Write to a stream.

        int
        fwrite(int handle, string buffer, [int length])

    Macro Description:
        The 'fwrite()' primitive shall write, from the string
        'buffer', up to 'length', to the stream pointed to by
        'handle'. If omitted, length by default to the string current
        length.

        The file-position indicator for the stream (if defined) shall
        be advanced by the number of bytes successfully written. If
        an error occurs, the resulting value of the file-position
        indicator for the stream is unspecified.

    Macro Parameters:
        handle - Stream handle.

        buffer - String buffer to be written.

        length - Optional length of buffer to be written.

    Macro Returns:
        The 'fwrite()' primitive shall return the number of elements
        successfully written, which may be less than 'length' if a
        write error is encountered.

        If 'length' is 0, fwrite() shall return 0 and the state of
        the stream remains unchanged. Otherwise, if a write error
        occurs, the error indicator for the stream shall be set, and
        'errno' shall be set to indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fwrite, fopen, fclose
 */
void
do_fwrite(void)                 /* (int handle, string buffer, [int length]) */
{
    int handle = get_xinteger(1, -1);
    const char *buffer = get_str(2);
    int bufsize = get_strlen(2);                /* FIXME - UTF encoding and NUL's are an issue */
  /*int length = get_xinteger(3, -1);*/
    int ret = -1;

    if (bufsize < 0) {
        system_errno(EINVAL);

    } else if (0 == bufsize) {
        ret = 0;

    } else {
        if ((ret = vfs_write(handle, buffer, bufsize)) < 0) {
            system_call(-1);
            ret = -1;
        }
    }
    acc_assign_int(ret);
}



/*  Function;           do_fseek
 *      fseek primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fseek - Reposition a file-position indicator in a stream.

        int
        fseek(int handle, int offset, int whence)

    Macro Description:
        The 'fseek()' primitive shall set the file-position indicator
        for the stream pointed to by stream. If a read or write error
        occurs, the error indicator for the stream shall be set and
        fseek() fails.

        The argument 'offset' is the position to seek to relative to
        one of three positions specified by the argument 'whence'.

            SEEK_SET - The new file position is computed relative to
                the start of the file. The value of offset must not
                be negative.

            SEEK_CUR - The new file position is computed relative to
                the current file position. The value of offset may be
                positive, negative or zero.

            SEEK_END - The new file position is computed relative to
                the end of the file.

    Macro Parameters:
        handle - Stream handle.

        offset - The new position, measured in bytes from the
            beginning of the file, shall be obtained by adding offset
            to the position specified by whence.

        whence - The specified point is the beginning of the file for
            *SEEK_SET*, the current value of the file-position
            indicator for *SEEK_CUR*, or end-of-file for *SEEK_END*.

    Macro Returns:
        On success the fseek() functions returns, otherwise -1 and set
        'errno' to indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        ftell, fwrite, fread
 */
void
do_fseek(void)                  /* (int handle, int offset, int whence) */
{
    int handle = get_xinteger(1, -1);
    int offset = get_xinteger(2, -1);
    int whence = get_xinteger(3, SEEK_SET);
    int ret = -1;

    if ((ret = vfs_seek(handle, offset, whence)) < 0) {
        system_call(-1);
        ret = -1;
    }
    acc_assign_int(ret);
}


/*  Function;           do_ftell
 *      ftell primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ftell - Report the file-position indicator of a stream.

        int
        ftell(int handle)

    Macro Description:
        The 'ftell()' primitive shall obtain the current value of the
        file-position indicator for the stream pointed to by stream.

    Macro Returns:
        On successful completion, ftell() shall return the current
        value of the file-position indicator for the stream measured
        in bytes from the beginning of the file.

        Otherwise, ftell() shall return -1, and set 'errno' to
        indicate the error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fseek, fwrite, fread
 */
void
do_ftell(void)                  /* (int handle) */
{
    int handle = get_xinteger(1, -1);
    int ret;

    if ((ret = vfs_tell(handle)) < 0) {
        system_call(-1);
        ret = -1;
    }
    acc_assign_int(ret);
}


/*  Function;           do_fstat
 *      fstat primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fstat - Stream status information.

        int
        fstat(int handle, [int size], [int mtime], [int ctime],
                [int atime], [int mode], [int uid], [string uid2name],
                [int gid], [string gid2name], [int nlink])

    Macro Description:
        The 'fstat()' primitive obtain information about the file
        referenced by the handle 'handle'.

        This information is returned in the parameters following the
        'handle' parameter (if supported by the underlying filesystem)

            size    - Total file size, in bytes.

            mode    - File's mode (see chmod).

            mtime   - The files "last modified" time (see time).

            atime   - Time the file was "last accessed".

            ctime   - Time of the files "last status change".

            uid     - user-id.

            gid     - group-id.

            nlink   - Number of hard links.

    Macro Returns:
        The 'fstat' function returns zero if successful, and a
        non-zero value (-1) otherwise. When an error has occurred,
        'errno' contains a value indicating the type of error that
        has been detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        fopen, stat, lstat
 */
void
do_fstat(void)                  /* (int handle, [int size], [int mtime], [int ctime], [int atime],
                                        [int mode], [int uid], [string uid2name], [int gid], [string gid2name], [int nlink] */
{
    struct stat sb;

    acc_assign_stat(&sb, vfs_fstat(get_xinteger(1, -1), &sb));
}


/*  Function;           do_fioctl
 *      fioctl primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fioctl - File miscellaneous control.

        int fioctl(int handle, ..)

    Macro Description:
        The 'fioctl()' primitive is reserved for future use.

    Macro Returns:
        Returns -1.

    Macro Portability:
        An experimental Grief extension; functionality may change.

    Macro See Also:
        fopen, flock
 */
void
do_fioctl(void)                 /* (int handle, ..) */
{
    acc_assign_int(-1);
}


/*  Function;           do_fflush
 *      fflush primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: fflush - Flush a stream.

        int
        fflush(int handle, [int sync])

    Macro Description:
        The 'fflush()' primitive is reserved for future use.

    Macro Returns:
        The 'fflush' function returns non-zero if a write error
        occurs and zero otherwise. When an error has occurred,
        'errno' contains a value indicating the type of error that
        has been detected

    Macro Portability:
        An experimental Grief extension; functionality may change.

    Macro Notes:
        The 'fflush()' primitive is reserved for future use, and
        currently returns -1 in all cases.

    Macro See Also:
        fopen
 */
void
do_fflush(void)                 /* (int handle, [int sync]) */
{
    acc_assign_int(-1);
}



/*  Function;           do_feof
 *      feof primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: feof - Test end-of-file indicator on a stream

        int
        feof(int handle)

    Macro Description:
        The 'feof()' primitive shall test the end-of-file indicator
        for the stream pointed to by stream.

    Macro Returns:
        The 'feof()' primitive returns non-zero if and only if the
        end-of-file indicator is set for stream.

    Macro Portability:
        A Grief extension.

    Macro Notes:
        The 'feof()' primitive is reserved for future use, and
        currently returns -1 in all cases.

    Macro See Also:
        fopen, ferror
 */
void
do_feof(void)                   /* int (int handle) */
{
    acc_assign_int(-1);
}



/*  Function;           do_ferror
 *      ferror primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ferror - Test error indicator on a stream.

        int
        ferror(int handle, [int clearerr])

    Macro Description:
        The 'ferror()' primitive shall test the error indicator for
        the stream referenced by 'handle'.

    Macro Returns:
        The 'ferror()' primitive shall return non-zero if and only if
        the error indicator is set for stream.

    Macro Portability:
        A Grief extension.

    Macro Notes:
        The 'ferror()' primitive is reserved for future use, and
        currently returns -1 in all cases.

    Macro See Also:
        fopen, feof
 */
void
do_ferror(void)                 /* int (int handle, [int clear]) */
{
    acc_assign_int(-1);
}



/*  Function;           do_flock
 *      flock primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: flock - File lock operations.

        int
        flock(int handle, ..)

    Macro Description:
        The 'flock()' primitive is reserved for future use.

    Macro Returns:
        Returns -1.

    Macro Portability:
        A Grief extension.

    Macro Notes:
        The 'flock()' primitive is reserved for future use, and
        currently returns -1 in all cases.

    Macro See Also:
        fopen, fioctl
 */
void
do_flock(void)                  /* (int handle, ..) */
{
    acc_assign_int(-1);
}


/*  Function;           do_ftruncate
 *      ftruncate primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: ftruncate - Truncate the specified file.

        int
        ftruncate(int handle, [int size])

    Macro Description:
        The 'ftruncate()' primitive cause the regular file referenced
        by 'handle' to be truncated to a size of precisely length
        bytes.

        If the file previously was larger than this size, the extra
        data is lost. If the file previously was shorter, it is
        extended, and the extended part reads as null bytes ('\0').

        The file offset is not changed.

        With ftruncate(), the file must be open for writing.

    Macro Returns:
        The 'ftruncate' function returns zero on success. When an
        error has occurred, 'errno' contains a value indicating the
        type of error that has been detected

    Macro Portability:
        A Grief extension.

    Macro Notes:
        The 'ftruncate()' primitive is reserved for future use, and
        currently returns -1 in all cases.

    Macro See Also:
        fseek, ftell, fwrite.
 */
void
do_ftruncate(void)              /* (int handle, ..) */
{
    acc_assign_int(-1);
}

/*end*/
