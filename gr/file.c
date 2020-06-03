#include <edidentifier.h>
__CIDENT_RCSID(gr_file_c,"$Id: file.c,v 1.84 2020/06/03 16:01:35 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: file.c,v 1.84 2020/06/03 16:01:35 cvsuser Exp $
 * File-buffer primitives and support.
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

#if !defined(ED_LEVEL)
#define ED_LEVEL 1
#endif

#include <editor.h>
#include <errno.h>
#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#include <edfileio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#if defined(WIN32)
#include <../libw32/win32_io.h>
#endif

#include "accum.h"                              /* acc_...() */
#include "anchor.h"                             /* do_write_block */
#include "asciidefs.h"                          /* ASCII_... */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "chunk.h"
#include "cmap.h"
#include "debug.h"                              /* trace_...() */
#include "display.h"                            /* vtupdate */
#include "echo.h"                               /* errorf, infof */
#include "eval.h"
#include "file.h"
#include "line.h"
#include "lisp.h"
#include "lock.h"                               /* flock...() */
#include "mac1.h"                               /* do_set_top_left */
#include "macros.h"                             /* macro_lookup */
#include "main.h"                               /* xf_readonly */
#include "map.h"                                /* linep */
#include "register.h"                           /* register_...() */
#include "symbol.h"                             /* system_call */
#include "sysinfo.h"
#include "system.h"                             /* sys_...() */
#include "tty.h"                                /* ttrows() */
#include "undo.h"
#include "wild.h"
#include "window.h"                             /* window_title */
#include "word.h"

#include "m_region.h"                           /* do_write_block() */
#include "m_backup.h"                           /* bkcfg_...() */

#include "../libvfs/vfs.h"
#include "../libchartable/iconv_stream.h"       /* TODO, hide within mchar.h */
#include "mchar.h"

#ifndef IOBUF_SIZ                               /* local i/o buffer */
#define IOBUF_SIZ           (BUFSIZ * 16)
#endif

#define PATH_SEPERATOR      '/'                 /* common path element separator */
#define KBYTES              1024

static int              buf_insert(BUFFER_t *bp, const char *fname, int inserting, const int32_t flags, const char *encoding);
static int              buf_diskchanged(BUFFER_t *bp);
static int              buf_backup(BUFFER_t *bp);
static int              buf_readin(BUFFER_t *bp, int fd, const char *fname, FSIZE_t fsize, int flags, const char *encoding);
static int              buf_writeout(BUFFER_t *bp, const char *fname, int undo_term, int append);
static int              buf_trimline(BUFFER_t *bp, const LINECHAR *text, LINENO length);

static int              file_copy(const char *src, const char *dst, mode_t perms, uid_t owner, gid_t group);
static int              file_cmp_char(const int c1, const int c2);

static void             file_canonicalize2(const char *fname, char *buf);

static size_t           varlen(const char *dp, const char *dpend);
static char *           varend(char *dp, char *dpend, const int what);

#define CHUNKSIZE_DEFAULT   64

                                                /* binary chunk/line size */
static unsigned         x_chunksize = CHUNKSIZE_DEFAULT;

static char             x_cwd[MAX_PATH];        /* working directory return buffers */
#if defined(DOSISH)         /* X: */
static char             x_cwdd[MAX_PATH];
#endif


/*  Function:           do_output_file
 *      output_file primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: output_file - Change the output file-name.

        int
        output_file([string filename])

    Macro Description:
        The 'output_file()' primitive changes the file-name
        associated with the current buffer to 'filename'; the new
        name should be unique, it cannot be the file-name of an exist
        buffer or file.

        By default the associated file-name associated with a buffer
        is the file-name specified on an <edit_file> or
        <create_buffer>.

        If 'filename' is omitted the user shall be prompted as follows;

>           Enter new output file name:

        Note!:
        Once changed backups shall be created under the new file-name,
        not the original name.

    Macro Parameters:
        filename - Optional string containing the new output
            file-name, if omitted the user is prompted.

    Macro Returns:
        The 'output_file()' primitive returns greater than zero on
        success, otherwise zero or less on error.

        The following error conditions shall be reported to the user;

            o 'filename' must be a unique buffer.

>               Duplicate buffer name 'xxx'.

            o Unless a system buffer stated 'filename' must not
                already exist on the file-system.

>               Output file 'xxx' already exists.

    Macro Portability:
        n/a

    Macro See Also:
        edit_file, create_buffer
 */
void
do_output_file(void)            /* ([string filename]) */
{
    char buf[MAX_PATH] = {0};
    const char *cp = get_arg1("Enter new output file name: ", buf, sizeof(buf));
    WINDOW_t *wp;
    BUFFER_t *bp;
    char *fname;

    acc_assign_int(-1);                         /* default result, failure */
    if (NULL == cp) {
        return;
    }
    fname = file_canonicalize(cp, NULL, 0);

    /* not an existing buffer */
    for (bp = buf_first(); bp; bp = buf_next(bp)) {
        if (bp != curbp && file_cmp(bp->b_fname, fname) == 0) {
            errorf("Duplicate buffer name '%s'.", fname);
            chk_free(fname);
            return;
        }
    }

    /* file image not exist */
    if (!BFTST(curbp, BF_SYSBUF) &&
            fileio_access(fname, 0) >= 0) {
        errorf("Output file '%s' already exists.", fname);
        chk_free(fname);
        return;
    }

    /* lock new image (if required) */
    if (1 == curbp->b_lstat) {                  /* modified buffer */
        if (-1 == flock_set(fname, TRUE)) {     /* unable to lock */
            chk_free(fname);
            return;
        }
        flock_clear(curbp->b_fname);
    }

    /* replace file details */
    buf_name(curbp, fname);

    BFCLR(curbp, BF_BACKUP);                    /* name changed no backup */
    BFSET(curbp, BF_NEW_FILE);                  /* new file */

    triggerx(REG_BUFFER_RENAME, "\"%s\"", fname);

    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_bufp == curbp) {
            window_title(wp, sys_basename(fname), "");
            wp->w_status |= WFHARD;
        }

    vtupdate();
    acc_assign_int(1);
}


/*  Function:           do_read_file
 *      read_file primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: read_file - Read into the current buffer.

        int
        read_file([string filename],
            [int glob = TRUE], [string encoding = NULL])

    Macro Description:
        The 'read_file()' primitive reads the content of the
        specified file 'filename' into the current buffer.

        If 'filename' is omitted the user shall be prompted as
        follows;

>           File to read:

    Macro Parameters:
        filename - Optional string containing the file to read, if
            omitted the user shall be prompted.

        glob - Optional boolean flag, if either *TRUE* or omitted
            the filename shall be expanded, see <expandpath>.

        encoding - Optional string containing the character encoding
            to be applied to the source file.

    Macro Returns:
        The 'read_file()' primitive returns a positive value on
        success, 0 if the user was prompted and cancelled, otherwise
        -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, edit_file
 */
void
do_read_file(void)              /* int (string filename, [int glob = TRUE], [string encoding = NULL]) */
{
    const char *encoding = NULL;                /* extension */
    const int doglob = get_xinteger(2, 1);      /* extension */
    char path[MAX_PATH];
    const char *cp;
    mode_t perms_mode;
    int val = 0;

    if (rdonly() ||
            NULL == (cp = get_arg1("File to read: ", path, sizeof(path)))) {
        acc_assign_int(0);
        return;
    }

    perms_mode = curbp->b_mode;
    if (doglob) {
        char **files;
        unsigned j;

        if (NULL == (files = shell_expand(cp))) {
            errorf("Name expansion error.");
            acc_assign_int(-1);
            return;
        }

        for (j = 0; files[j]; ++j) {
            if (files[j][0]) {
                val = buf_insert(curbp,         /* insert content */
                        file_canonicalize(files[j], path, sizeof(path)), FALSE, EDIT_NORMAL, encoding);
            }
        }
        shell_release(files);

    } else {
        val = buf_insert(curbp,                 /* insert content */
                file_canonicalize(cp, path, sizeof(path)), FALSE, EDIT_NORMAL, encoding);
    }

    curbp->b_mode = perms_mode;
    BFCLR(curbp, BF_RDONLY);

    window_center_line(curwp, *cur_line);       /* center buffer/window */

    acc_assign_int(val);
}


/*  Function:           inq_file_change
 *      inq_file_change primitive.
 *
 *  Description:
 *      none
 *
 *  Return value:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_file_change - Determine state of underlying file.

        int
        inq_file_change([int bufnum])

    Macro Description:
        The 'inq_file_change()' primitive determines the state of the file
        which underlies the specified buffer 'bufnum'. This primitive
        checks whether the associated file has been modified or deleted.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_file_change()' primitive returns the reason code for the
        file state change.

        0  - No change.
        1  - File status detected; possible in-place changes.
        2  - File modified, size differences detected.
        3  - Underlying file does not exist (i.e. has been deleted).
        -1 - Unknown error, the cause of the error condition can be
                derived from the system return code (see errno).

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_modified, edit_file
 */
void
inq_file_change(void)           /* int ([int bufnum]) */
{
    BUFFER_t *bp = buf_argument(1);
    int ret = -2;

    if (bp) {
        ret = buf_diskchanged(bp);
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           inq_terminator
 *      inq_terminator primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_terminator - Retrieve a buffers line terminator.

        int
        inq_terminator([int bufnum], [string &term])

    Macro Description:
        The 'inq_terminator()' primitive retrieves the line
        terminator of the specified buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        term - Optional string variable reference, to be populated
            with the line terminator of referenced buffer.

    Macro Returns:
        The 'inq_termintor()' primitive returns the line terminator
        type of the specified buffer (see set_terminator), otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_terminator
 */
void
inq_terminator(void)            /* int ([int bufnum], [string &term]) */
{
    BUFFER_t *bp = buf_argument(1);
    char terminator[16] = {0};
    int termtype = -1;

    if (bp) {
        file_terminator_get(bp, terminator, sizeof(terminator), &termtype);
    }
    argv_assign_str(2, terminator);
    acc_assign_int(termtype);
}


/*  Function:           do_set_terminator
 *      set_terminator primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *
 *<<GRIEF>>
    Macro: set_terminator - Set a buffers line terminator.

        int
        set_terminator([int bufnum], int|string term)

    Macro Description:
        The 'set_terminator()' primitive retrieves the line
        terminator of the specified buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current
            buffer shall be referenced.

        term - Either the integer enumeration or string description
            of the line terminator to be assigned.

    Enumerations::

(start table)
        [Constant           [Description                ]
      ! LTERM_UNDEFINED     Unknown/default.
      ! LTERM_NONE          <none> (i.e. binary)
      ! LTERM_UNIX          CR/LF
      ! LTERM_DOS           LF
      ! LTERM_MAC           CR
      ! LTERM_NEL           NEL
      ! LTERM_UCSNL         Unicode next line
      ! LTERM_USER          User defined
(end table)

    Macro Returns:
        The 'set_terminator()' primitive returns 1 is the line
        terminator was modified, 0 when no change occured, otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_terminator
 */
void
do_set_terminator(void)         /* int ([int bufnum], [int type|string terminator]) */
{
    BUFFER_t *bp = buf_argument(1);
    const char *terminator = get_xstr(2);
    const int termtype = (!terminator ? get_xinteger(2, -1) : 0);
    int ret = -1;

    if (bp) {
        ret = file_terminator_set(bp, terminator, (terminator ? get_strlen(1) : 0), termtype);
    }
    acc_assign_int(ret);
}


/*  Function:           inq_byte_pos
 *      inq_byte_pos primitive.
 *
 *  Parameters:
 *      none
 *
 *  Macro Parameters:
 *      File byte offset.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_byte_pos - Get current position in buffer stream

        int
        inq_byte_pos([int bufnum],
            [int line], [int col], [int flags])

    Macro Description:
        The 'inq_byte_pos()' primitive is reserved for future
        compatibility.

        The 'inq_byte_pos()' primitive calculates and returns the
        byte offset from the start of the specified buffer with the
        first byte within the underlying buffer being at offset 0.

        This primitive is similar the native library function 'tell'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        line - Optional line number.

        col - Optional column.

        flags - Offset origin flag, omitted when 0x00.

    Macro Returns:
        The 'inq_byte_pos()' primitive returns the current value of
        the file-position indicator for the associated buffer
        measured in bytes from the beginning of the file. Otherwise,
        it returns -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        inq_terminator
 */
void
inq_byte_pos(void)              /* int ([int bufnum], [int line], [int col], [int flags]) */
{
#if (TODO)
    offset = bp->b_bomlen;
    foreach (line = 1; lineno < line;) {
        offset += line->length;
        offset += termlen;
    }
    offset += char_width(col);
#endif
    acc_assign_int(-1);
}


/*  Function:           do_set_binary_size
 *      set_binary_size primitive, which sets the binary chunk size.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_binary_size - Set binary chunk size.

        int
        set_binary_size([int size])

    Macro Description:
        The 'set_binary_size()' primitive sets the chunk or block
        size utilised when loading binary file images. Each chunk
        shall be represented within the buffer as a single line.

        Note:!
        If the specified value is equal or less-then zero (<= 0),
        then files shall never be interpreted as binary when read;
        instead the default system specific type as be applied.

    Macro Parameters:
        size - Optional integer number, if omitted the current size is
            not changed.

    Macro Returns:
        The 'set_binary_size()' primitive returns the previous chunk size.

    Macro Portability:
        n/a

    Macro See Also:
        inq_terminator
 */
void
do_set_binary_size(void)        /* int ([int size]) */
{
    acc_assign_int((accint_t) x_chunksize);
    x_chunksize = get_xinteger(1, x_chunksize);
}


/*  Function:           file_readin
 *      Populate the specified buffer 'bp' with the content of the file
 *      'fname', with any existing content cleared prior.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      fname - File name.
 *      flags - Edit flags (EDIT_XXX).
 *
 *  Returns:
 *      see buf_insert()
 */
int
file_readin(BUFFER_t *bp, const char *fname, const int32_t flags, const char *encoding)
{
    int status;
    WINDOW_t *wp;

    buf_clear(bp);

    status = buf_insert(bp, fname, TRUE, flags, encoding);

    BFCLR(bp, BF_CHANGED);
    BFSET(bp, BF_READ);

    if (0 == status) {
        BFSET(bp, BF_BACKUP);
        if (0 == BFTST(bp, BF_SYSBUF)) {
            if (1 == bp->b_lstat || 0 == (EDIT_NOLOCK & flags)) {
                BFSET(bp, BF_LOCK);             /* EDIT_LOCK or auto-lock */
            }
            if ((EDIT_READONLY|EDIT_PREVIEW) & flags) {
                BFSET(bp, BF_RDONLY);
            }
            if (TRUE == xf_spell) {
                BFSET(bp, BF_SPELL);
            }
            if (xf_autosave) {
                BF3SET(bp, BF3_AUTOSAVE);
            }
            BF3SET(bp, BF3_AUTOINDENT);
        }
    }

    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_bufp == bp) {
            wp->w_top_line = wp->w_line = wp->w_col = 1;
        }
    return status;
}


/*  Function:           buf_insert
 *      Insert the specified file 'fname' into the given buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      fname - File name.
 *      inserting - *true* if inserting.
 *      flags - Edit flags (EDIT_XXX).
 *
 *  Returns:
 *      0 -     Success
 *      1 -     New file
 *      -1 -    Error
 */
static int
buf_insert(BUFFER_t *bp, const char *fname, int inserting, const int32_t flags, const char *encoding)
{
    const int startup = ((EDIT_STARTUP & flags) ? TRUE : FALSE);
    BUFFER_t *saved_bp = curbp;
    int fd, readonly = 0;

#if (TODO_POPEN)
    /*
     *  pipe request
     *      If a filename starts with a '|' character then everything after the '|'
     *      character is assumed to be a command which is passed to the popen()
     *      call and the data from the pipe is read in to a new buffer, whose name
     *      includes the pipe-character at the start of the file name.
     */
    if ('|' == *fname) {                        /* PIPE */
    }
#endif

#if defined(TODO_VFS_PIPES)
    /*
     *  TODO - trigger registered extension processing
     */
    const char *ext;

    if (NULL != (ext = strrchr(fname, '.'))) {
        char t_command[MAX_PATH], t_fname[MAX_PATH];

        /*
         *  TODO - lookup registered function ending with
         *              "_ext" and then execute.
         */
        if (trigger_rstring(REG_INPUT_FILENAME, ext, t_command, sizeof(t_command))) {
            if (! vfs_local_get(fname, t_fname)) {
                return -1;                      /* could not localise file */
            }
            strxcat(t_command, t_fname);        /* add the localise file */
            if ((fd = vfs_popen(t_command, O_RDONLY)) < 0) {
                return -1;                      /* could not create pipe */
            }
            ispipe = TRUE;
        }
    }
#endif

    bp->b_mode = 0666 & ~x_umask;               /* default mode for new files */
    if (0 == bp->b_mode) bp->b_mode = 0600;

    if ((fd = vfs_open(fname, OPEN_R_BINARY | O_RDWR, bp->b_mode)) < 0) {
        fd = vfs_open(fname, OPEN_R_BINARY | O_RDONLY, bp->b_mode);
        readonly = 1;
    }

    if (fd >= 0) {
        struct stat sb = {0};
        WINDOW_t *wp;

        vfs_fstat(fd, &sb);

        if (! S_ISREG(sb.st_mode) || S_ISDIR(sb.st_mode)) {
            vfs_close(fd);
            errno = EPERM;
            fd = -1;

        } else {
            int numlines;

            if (inserting) {    /*update file attributes*/
                bp->b_mode  = sb.st_mode;       /* mode */
                bp->b_mtime = sb.st_mtime;      /* modification time */
                bp->b_rsize = sb.st_size;       /* size, in bytes */
                if (S_IEXEC & sb.st_mode) {
                    BFSET(bp, BF_EXEC);
                }
            }

            if (EDIT_SYSTEM & flags) {
                BFSET(bp, BF_SYSBUF);
            }

            curbp = bp;
            set_hooked();
            numlines = buf_readin(bp, fd, fname, sb.st_size, flags, encoding);
            vfs_close(fd);
            fd = -1;

            if ((xf_readonly && !BFTST(bp, BF_SYSBUF)) || readonly) {
                BFSET(bp, BF_RDONLY);
            }

            bp->b_ctime = time(NULL);           /* change time-stamp */
            BFSET(bp, BF_CHANGED);              /* buffer changed attribute */

#if defined(HAVE_CHOWN)
            if (inserting) {
                bp->b_uid = sb.st_uid;
                bp->b_gid = sb.st_gid;
            }
#endif

            if (numlines >= 0)                  /* force update of related windows */
                buf_mined(bp, bp->b_line, bp->b_line + numlines);

            for (wp = window_first(); wp; wp = window_next(wp))
                if (wp->w_bufp == curbp) {
                    wp->w_status |= WFHARD;
                }

            curbp = saved_bp;
            set_hooked();
            return (numlines >= 0 ? 0 : -1);
        }
    }

    system_call(-1);                            /* set global errno */

    if (! BFTST(bp, BF_SYSBUF)) {
        int x_errno = errno;
                                                /* new file image (ie can it be created) ? */
        if ((fd = vfs_open(fname, OPEN_R_BINARY|O_CREAT|O_EXCL|O_RDWR, bp->b_mode)) != -1) {
            vfs_close(fd);
            vfs_remove(fname);                  /* remove it new version */
        }
        errno = x_errno;

        if (inserting && fd != -1) {            /* new file */
            if (! startup) {
                infof("%s: new file.", sys_basename(fname));
            }
            BFSET(bp, BF_NEW_FILE);
            bp->b_uid = (uid_t) -1;
            bp->b_gid = (gid_t) -1;
            if (BFTYP_UNDEFINED == bp->b_type) {
                if (0 == BFTST(bp, (BF_PROCESS|BF_BINARY))) {
                    buf_type_default(bp);
                } else {
                    buf_type_set(bp, BFTYP_BINARY);
                    buf_encoding_set(bp, NULL);
                }
            }
            return 1;
        }
        ewprintx("%s", sys_basename(fname));
    }
    return -1;
}


/*  Function:           buf_diskchanged
 *      Has on-disk image changed since our last write.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Returns 0 if no change, 1 if changed, 2 if deleted, otherwise -1 on error.
 */
static int
buf_diskchanged(BUFFER_t *bp)
{
    struct stat sb;

    if (bp->b_fname[0] && bp->b_mtime) {        /* was read in */
        if (stat(bp->b_fname, &sb) >= 0) {
            if (sb.st_mtime > bp->b_mtime) {
                if ((size_t)sb.st_size != bp->b_rsize) {
                    return 2;                   /* size change */
                }
                return 1;                       /* status change; possible in-place changes */
            }
        } else {
            if (ENOENT == errno) {
                return 3;                       /* file deleted */
            }
            system_call(-1);                    /* set global errno */
            return -1;                          /* other error */
        }
    }
    return 0;
}


/*  Function:           file_write
 *      Write the current buffer away to the specified filename 'fname'
 *      otherwise (if NULL) the current assigned filename.
 *
 *  Parameters:
 *      fname - File name.
 *      flags - Edit flags.
 *
 *  Returns:
 *      -1 if any errors occur so that the exit can be avoided.
 *
 *  Extended return codes (TODO);
 *
 *      -1  Disk full condition.
 *      -2  File creation error.
 *      -3  Permission errors replacing original file.
 *      -4  User aborted (e.g. trigger callback(.
 *      -5  Buffer is not associated with a file-name.
 *      -6  Underlying file status has changed (e.g. size, permissions).
 *      -7  Read-only mode.
 */
int
file_write(const char *fname, const int32_t flags)
{
    const int append = (flags & WRITE_APPEND ? 1 : 0);
    int s;

    if (rdonly()) {
        return 0;                               /* read-only */
    }

    /*
     *  Dont save file if it is a normal buffer and there aren't any changes;
     *  if its a system buffer save it anyway, because we dont keep track of
     *  'b_nummod' for system buffers.
     */
    if (0 == (WRITE_FORCE & flags)) {
        if (0 == curbp->b_nummod && 0 == BFTST(curbp, (BF_SYSBUF | BF_NEW_FILE))) {
            infof("Buffer has not been modified -- not written.");
            return 0;
        }
    }

    /*
     *  Is a file associated?
     */
    if ((fname && 0 == *fname) ||
            (NULL == fname && 0 == curbp->b_fname[0])) {
        infof("No file name");
        return -1;
    }

    /*
     *  File save request
     */
#if (TODO_REG_FILE_SAVE)
    if (0 == (WRITE_NOTRIGGER & flags))
        if (0 == trigger_rint(REG_FILE_SAVE)) {
            infof("Buffer write was denyed -- not written.");
            return -4;
        }
#endif

    /*
     *  Has the image changed (ignore system/volatile buffers)?
     */
    if (NULL == fname && !BFTST(curbp, BF_SYSBUF) && !BFTST(curbp, BF_VOLATILE) &&
                buf_diskchanged(curbp) >= 1) {
        if ((s = eyesno("On disk image has changed, save anyway")) != TRUE) {
            return -1;
        }
    }

    /*
     *  Create the backup image,
     *      Turn off new file flag as we write the file away, otherwise we keep
     *      letting user save the file.
     */
    BFCLR(curbp, BF_NEW_FILE);

#if !defined(_VMS)
    if (NULL == fname &&
            ((flags & WRITE_BACKUP) ||
                (BFTST(curbp, BF_BACKUP) && bkcfg_ask(curbp->b_fname)))) {
        if (FALSE == (s = buf_backup(curbp)) &&
                (s = eyesno("Backup error, save anyway")) != TRUE) {
            return -1;
        }
    }
#endif  /*!VMS*/

    /*
     *  Write out image.
     */
    if ((s = buf_writeout(curbp,
                (fname ? fname : curbp->b_fname), (fname ? 0 : 1), append)) == TRUE) {
        if (NULL == fname) {
            struct stat sb;

            /*
             *  Clear changed and backup,
             *      Clearing backup creates the situation where a back is created
             *      only once per edit session.
             *
             *  Update mtime
             */
            BFCLR(curbp, BF_CHANGED);
            BFCLR(curbp, BF_BACKUP);
            curbp->b_nummod = 0;
            if (stat(curbp->b_fname, &sb) >= 0) {
                curbp->b_mtime = sb.st_mtime;   /* on disk time-stamp */
                curbp->b_rsize = sb.st_size;
            }
        }

        if (0 == (flags & WRITE_NOTRIGGER) ) {
            triggerx(REG_FILE_WRITTEN, "\"%s\"", (fname ? fname : curbp->b_fname));
        }
        return 0;                               /* success */
    }
    return -1;
}


/*  Function:           reload_buffer
 *      reload_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: reload_buffer - Reload the buffer content.

        int
        reload_buffer([int bufnum], [string encoding])

    Macro Description:
        The 'reload_buffer()' primitive is reserved for future use.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        encoding - Optional string containing the character encoding
            to be applied to the source file.

    Macro Returns:
        n/a

    Macro Portability:
        n/a

    Macro See Also:
        edit_file

 */
void
do_reload_buffer(void)          /* ([int bufnum], [string encoding]) */
{
    /*TODO*/
    acc_assign_int(-1);
}


/*  Function:           write_buffer
 *      write_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: write_buffer - Write to buffer content.

        int
        write_buffer(
            [string filename], [int flags], [string encoding])

    Macro Description:
        The 'write_buffer()' primitive writes the content of the
        current buffer to its associated file.

    Macro Parameters:
        filename - Options string containing the name of the output
            filename. If omitted then the file is written to the
            name associated with the current buffer during creation
            using <<create_buffer>.

        flags - Optional integer flags, one or more of the
            following flags OR'ed together control the functions of
            the write operation.

        encoding - Optional string containing the character encoding
            to be utilised within the output file.

    Flags::

(start table,format=nd)
        [Constant       [Description                                    ]
     !  WRITE_APPEND    Append, otherwise overwrite.
     !  WRITE_NOTRIGGER Do not generate buffer triggers.
     !  WRITE_NOREGION  Ignore any selected region.
     !  WRITE_FORCE     Force write, even if no change.
     !  WRITE_BACKUP    Generate a backup image regardless whether
                        already performed for this edit session.
(end table)

    Macro Returns:

       o Returns greater than zero on success.

       o Returns zero if file was not saved, eg. because the file has
         already been saved.

       o Returns less than zero if an error occurs.

(start table,format=nd)
        [Value  [Description                                            ]
     !  -1      Disk space occurred.

     !  -2      Output file could not be created.

     !  -3      The output file was created with a different temporary
                name but could not be renamed to the target file due to
                permission errors.

     !  -4      User aborted the attempt to save the file from one of
                the callback triggers.

     !  -5      The output buffer does not have a valid filename.

     !  -6      The originally loaded file has changed its permissions,
                size or status on disk. This option avoids potentially
                losing work when someone else has written to the file
                whilst we were editing it.

     !  -7      The file is read-only, either due to file writes having
                been disabled by the command line switch (-R) or the
                current file permissions.
(end table)

       In many cases the underlying cause of the error condition can be
       derived from the system return code (see errno), for example out
       of disk space.

    Macro Portability:
       Flags are incompatible with CrispEdit(tm)

>           write_buffer([string filename], [int and_flags], [or_flags])

    Macro See Also:
        edit_file
 */
void
do_write_buffer(void)           /* ([string filename], [int flags], [string encoding]) */
                                /* ([string filename], [int flags], [int or_flags], [string encoding]) -- TODO */
{
    const char *fname = get_xstr(1);
    const int flags = get_xinteger(2, 0);
//  const char *encoding = get_xstr(3);         /* TODO */

    trace_log("write_buffer(%s,%d/0x%x)\n", (fname ? fname : "NULL"), flags, flags);

    if (NULL == fname && curbp->b_anchor && (flags & WRITE_NOREGION) == 0) {
        do_write_block();

    } else {
        acc_assign_int(rdonly() ? -7 : file_write(fname, flags /*TODO - encoding*/));
    }
}


int
file_terminator_set(BUFFER_t *bp, const void *terminator, int length, int termtype)
{
#define B_TERMBUF_SIZE      ((int)sizeof(bp->b_termbuf))

    char oldterm[B_TERMBUF_SIZE+1] = {0};
    int changed = FALSE;

    __CUNUSED(length)

    file_terminator_get(bp, oldterm, sizeof(oldterm)-1, NULL);

    if (NULL == terminator || 0 == *((const char *)terminator)) {
        /*
         *  type
         */
        trace_ilog("file_terminator_set() = %d\n", termtype);

        if (termtype > 0) {
            if ((unsigned)termtype != bp->b_termtype) {
                bp->b_termtype = termtype;
                changed = TRUE;
            }
        }
        memset(bp->b_termbuf, 0, B_TERMBUF_SIZE);
        bp->b_termlen = 0;

    } else {
        /*
         *  explicit
         */
        int termlen = (int)strlen((const char *)terminator);

        trace_ilog("file_terminator_set() = %d [", termlen);
        trace_data(terminator, termlen, "]\n");

        if (0 != strncmp(terminator, oldterm, B_TERMBUF_SIZE)) {
            if (termlen > B_TERMBUF_SIZE) {
                termlen = B_TERMBUF_SIZE;
            }
            memset(bp->b_termbuf, 0, B_TERMBUF_SIZE);
            memcpy(bp->b_termbuf, terminator, termlen);
            bp->b_termlen = termlen;
            bp->b_termtype = (termtype <= 0 ? LTERM_USER : termtype);
            changed = FALSE;
        }
    }
    return changed;
}


/*  Function:           file_terminator_get
 *      Return the line terminator for the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      buffer - Line terminator result buffer.
 *      length - Length of destination buffer.
 *
 *  Returns:
 *      Length in bytes.
 */
int
file_terminator_get(BUFFER_t *bp, void *buffer, int length, int *ptermtype)
{
    int termlen = 0, termtype = LTERM_UNDEFINED;
    const unsigned char *termbuf = NULL;
    unsigned char t_termbuf[16] = {0};

    if (0 == BFTST(bp, BF_BINARY)) {
        /*
         *  class line terminator
         *      buffer specific terminator
         *      or otherwise, buffer term/type/flags
         */
        if ((termlen = bp->b_termlen) > 0) {
            termtype = bp->b_termtype;
            termbuf = bp->b_termbuf;

        } else if (LTERM_UNDEFINED == (termtype = bp->b_termtype)) {
            switch (bp->b_type) {
            case BFTYP_DOS:             /* CR+LF */
                termtype = LTERM_DOS;
                break;
            case BFTYP_MAC:             /* CR */
                termtype = LTERM_MAC;
                break;
            case BFTYP_UNIX:            /* LF */
                termtype = LTERM_UNIX;
                break;
            case BFTYP_BINARY:
                termtype = LTERM_NONE;
                break;
            case BFTYP_UTF8:            /* UTF8 */
            default:
                termtype =              /* Allow CR_MODE override */
                    (BFTST(bp, BF_CR_MODE) ? LTERM_DOS : LTERM_UNIX);
                break;
            }
        }
    } else {
        termtype = LTERM_NONE;
    }

    if (NULL == termbuf && LTERM_NONE != termtype)  {
        if (LTERM_DOS == termtype || LTERM_UNIX == termtype) {
            if (BFTST(bp, BF_CR_MODE)) {
                t_termbuf[termlen++] = ASCIIDEF_CR;
            }
            t_termbuf[termlen++] = ASCIIDEF_LF;
            termbuf = t_termbuf;

        } else if (LTERM_MAC   == termtype) {
            t_termbuf[termlen++] = ASCIIDEF_CR;
            termbuf = t_termbuf;

        } else if (LTERM_NEL   == termtype) {
            t_termbuf[termlen++] = ASCIIDEF_NEL;
            termbuf = t_termbuf;

        } else if (LTERM_UCSNL == termtype) {
            mchar_iconv_t *iconv;

            if (NULL != (iconv = bp->b_iconv) &&
                    (termlen = mchar_encode(iconv, 0x2028, t_termbuf)) > 0) {
                termbuf = t_termbuf;
            } else {
                t_termbuf[termlen++] = ASCIIDEF_LF;
                termbuf = t_termbuf;
            }
        }
    }

    trace_ilog("file_terminator_get(termtype:%d) = %d [", termtype, termlen);
    trace_data(termbuf, termlen, "]\n");

    if (buffer) {
        if (termlen && termbuf) {
            strncpy(buffer, (const char *)termbuf, length);
        } else {
            *((char *)buffer) = 0;
        }
    }

    if (ptermtype) {
        *ptermtype = termtype;
    }
    return termlen;
}


static int
buf_readin(BUFFER_t *bp, int fd, const char *fname, FSIZE_t fsize, int flags, const char *encoding)
{
#define CACHEBUFSZ              (32 * 1024)

    BUFFER_t *saved_bp = curbp;
    const LINENO cline = *cur_line, numlines = bp->b_numlines;
    LINENO previewlines = 0x7ffffff, newlines = 0, crs = 0;
    LINE_t *clp, *lp;

    mcharguessinfo_t fileinfo = {0};
    int32_t eolchar = ASCIIDEF_LF;              /* EOL character */
    const unsigned char *termbuf = NULL;
    size_t termlen = 0;
    int is8bit = 0;

    FSIZE_t fleft = fsize, fcursor = 0, bufsize = 0, bufcnt;
    char *buffer = NULL, *cache = NULL;

    const unsigned chunksize = (x_chunksize > 0 ? x_chunksize : CHUNKSIZE_DEFAULT);
    uint32_t chunkrefs = 0;
    void *chunk = NULL;

    const char *ovbuf = NULL;                   /* overflow from previous crunk */
    uint32_t ovlen = 0;

    mchar_iconv_t *iconv = NULL;                /* base conversion */
    mchar_istream_t *istream = NULL;            /* stream conversion interface */

    trace_ilog("Loading:(fname:%s, size:%d, line:%d)\n", fname, (int)fsize, (int)cline);

    /*
     *  determine file encoding, best effort
     */
    if (NULL == encoding) {
        if (EDIT_NORMAL == (EDIT_MASK & flags)) {

            if (NULL == (buffer = cache = chk_calloc(1, CACHEBUFSZ))) {
                errorfx("%s: building cache", fname);
                return -1;
            }

            if ((bufcnt = vfs_read(fd, buffer, CACHEBUFSZ)) > 0) {

                mchar_guess(x_encoding_guess, (EDIT_QUICK ? 0x01 : 0x00), buffer, bufcnt, &fileinfo);

                if (BFTYP_UNSUPPORTED == fileinfo.fi_type) {
#if defined(EDIT_KNOWN)                         /* unknown, default to binary */
                    if (EDIT_KNOWN & flags) {
                        const char *description;
                        if (NULL != (description = fileinfo.fi_desc)) {
                            infof("%s: unsupported file encoding '%s' -- read aborted", fname, description);
                        } else {
                            infof("%s: unsupported file encoding -- read aborted", fname);
                        }
                        goto error;             /* only edit 'known' file types */
                    }
#endif

                    fileinfo.fi_type =          /* binary support status */
                        (x_chunksize <= 0 ? BFTYP_DEFAULT : BFTYP_BINARY);
                    BFSET(bp, BF_RDONLY);

                } else {
                    unsigned bomlen;

                    /* reposition cursor past BOM, if present */
                    if ((bomlen = fileinfo.fi_bomlen) > 0) {
                        bufcnt -= bomlen;
                        buffer += bomlen;
                    }

                    /* apply buffer type results */
                    if (BFTYP_BINARY == fileinfo.fi_type) {
                        if (x_chunksize <= 0) { /* binary support status */
                            fileinfo.fi_type = BFTYP_DEFAULT;
                        } else {
                            flags |= EDIT_BINARY;
                        }
                    } else if (BFTYP_DOS == fileinfo.fi_type ||
                                    (MCHAR_FI_CRLF & fileinfo.fi_flags)) {
                        flags |= EDIT_STRIP_CR;
                    }
                    encoding = fileinfo.fi_encoding;
                }
                                                /* cache line terminator details */
                eolchar = (BFTYP_MAC == fileinfo.fi_type ? ASCIIDEF_CR :
                                BFTYP_ANSI == fileinfo.fi_type ? ASCIIDEF_NEL : ASCIIDEF_LF);
                termlen = fileinfo.fi_termlen;
                termbuf = fileinfo.fi_termbuf;
                if (1 == termlen && eolchar == *termbuf) {
                    termlen = 0;
                }
            }

        } else {
            fileinfo.fi_type = BFTYP_BINARY;
            eolchar = 0;
        }
    }

    /*
     *  create decoding stream and import,
     *      mchar_iconv represents both the external and internal encoding which shall
     *      be used for the associated buffer.
     *
     *  Examples:
     *      encoding        streaming       internal
     *      latin-1         NULL            latin1 -> unicode
     *      iso2022         utf8            utf8   -> unicode
     */
#if defined(TODO)
    if (numlines) {
        check encoding against buffer encoding
            if compatible ok, otherwise prompt options
    }
#endif  /*TODO*/

    if (XF_TEST(6)) {
        char t_encoding[64];

        t_encoding[0] = '+';                    /* force stream usage */
        strxcpy(t_encoding + 1, (encoding ? encoding : "ascii"), sizeof(t_encoding) - 1);
        if (NULL == (iconv = mchar_iconv_open(t_encoding))) {
            infof("%s: encoding '%s' not available -- read aborted", fname, encoding);
            goto error;
        }
    } else {
        if (NULL == (iconv = mchar_iconv_open(encoding))) {
            infof("%s: encoding '%s' not available -- read aborted", fname, encoding);
            goto error;
        }
    }

    is8bit = BFTYP_IS8BIT(fileinfo.fi_type);    /* TODO - mchar_is8bit(iconv) */
    istream = mchar_stream_open(iconv, fd, fname, "r");

    trace_ilog("istream:%c, is8bit:%c, eolchar:%d/0x%02x, termlen:%lu, encoding:%s, internal:%s\n",
        (istream ? 'y' : 'n'), (is8bit ? 'y' : 'n'), eolchar, eolchar, (unsigned long)termlen, \
            mchar_encoding(iconv), mchar_internal_encoding(iconv));

    if (buffer) {                               /* push-back guess buffer, if any */
        trace_ilog("push: %d\n", (int)bufcnt);
        if (NULL == istream)  {
            fleft  -= bufcnt;
            ovbuf   = buffer;
            ovlen   = bufcnt;
        } else {
            mchar_stream_push(istream, buffer, bufcnt);
        }
    }

    /*
     *  import
     */
    curbp = bp;
    clp = (cline > 1 ? linepx(bp, cline) : NULL);

    if (EDIT_PREVIEW & flags) {
        previewlines = ttrows();                /* Limit to window size */
    }

    second_passed();
    for (;;) {                                  /* next chunk sized for remaining bytes, for streams add 5% */

        const size_t chunkreq = (ovlen + fleft + (istream ? (fleft / 20) : 0));
        const size_t chunklen =
            chunk_size(chunkreq > CACHEBUFSZ ? CACHEBUFSZ : chunkreq);
        const char *bpsol, *bpeol, *bpend;
        size_t llen, cnt = 0;
        int broken = 0;

        /*
         *  Allocate and read the next buffer chunk
         */
        if (chunk) {                            /* protect previous chunk */
            chunk_protect(bp, chunk, chunkrefs);
            chunkrefs = 0;
            chunk = NULL;
        }

        if (NULL == (buffer = (char *)chunk_new(bp, chunklen, &chunk))) {
            ewprintf("%s: cannot allocate %lu sized chunk", fname, (unsigned long)chunklen);
            goto error;
        }

        if (ovlen) {                            /* overflow */
            assert(ovlen <= chunklen);
            memcpy(buffer, ovbuf, ovlen);
            ovbuf = NULL;
        }

        if (ovlen < chunklen) {                 /* read additional */
            if (NULL == istream) {
                if ((cnt = vfs_read(fd, buffer + ovlen, chunklen - ovlen)) > 0) {
                    if ((fleft -= cnt) < 0) fleft = 0;
                    fcursor += cnt;
                }

            } else {
                size_t inbytes = 0;
                if ((cnt = mchar_stream_read(istream, (char *)(buffer + ovlen), chunklen - ovlen, &inbytes)) > 0) {
                    if ((fleft -= inbytes) < 0) fleft = 0;
                    fcursor += inbytes;
                }
            }
        }

        if (cnt <= 0) {
            if (0 == cnt) {                     /* EOF */
                if (ovlen <= 0) {
                    if (fleft) {
                        infof("%s: short read on stream -- aborted", fname);
                    }
                    trace_ilog("EOF\n");
                    break;
                }
            } else {                            /* error conditions */
                errorfx("%s: error reading", fname);
                goto error;
            }
        }

        trace_ilog("chunk_read(%lu of %lu, %d) = %lu\n",
            (unsigned long)(chunklen - ovlen), (unsigned long)chunklen, (int)fleft, (unsigned long)cnt);
        cnt += ovlen;
        ovlen = 0;

        /*
         *  EOL scanner/
         *      Select specialisation based on terminator length and buffer encoding.
         *      On a EOL condition, determine length remove terminator and optional preceeding CR.
         */
        bpsol = buffer;
        bpend = bpsol + cnt;
        assert(bpend <= buffer + chunklen);

        while (bpsol < bpend) {                 /* iterate chunk, line-by-line */

            if (EDIT_BINARY & flags) {          /* chunksize fixed lines */
                if ((bpeol = (bpsol + chunksize)) > bpend) {
                    bpeol = bpend;
                }

            } else {
                if (is8bit) {
                    /*
                     *  SBCS, either 7 or 8 bit ....
                     */
                    if (termlen) {
                        unsigned char prev = 0, raw;

                        for (bpeol = bpsol; bpeol < bpend; ++bpeol) {
                            raw = *((unsigned char *)bpeol);
                            if (raw == eolchar) {
                                llen = bpeol - bpsol;
                                if ((EDIT_STRIP_CR & flags) && ASCIIDEF_CR == prev) {
                                    --llen;
                                    ++crs;
                                }
                                ++bpeol;
                                goto done;

                            } else if (raw == *termbuf) {
                                if (1 == termlen) {
                                    llen = bpeol - bpsol;
                                    if ((EDIT_STRIP_CR & flags) && ASCIIDEF_CR == prev) {
                                        --llen;
                                        ++crs;
                                    }
                                    ++bpeol;
                                    goto done;

                                } else if (bpeol + termlen <= bpend && 0 == memcmp(bpeol, termbuf, termlen)) {
                                    llen  = bpeol - bpsol;
                                    bpeol += termlen;
                                    goto done;
                                }
                            }
                            prev = raw;
                        }

                    } else {
                        unsigned char prev = 0, raw;

                        for (bpeol = bpsol; bpeol < bpend;) {
                            raw = *((unsigned char *)bpeol);
                            if (raw == eolchar) {
                                llen = bpeol - bpsol;
                                if ((EDIT_STRIP_CR & flags) && ASCIIDEF_CR == prev) {
                                    --llen;
                                    ++crs;
                                }
                                ++bpeol;
                                goto done;
                            }
                            ++bpeol;
                            prev = raw;
                        }
                    }

                } else {
                    /*
                     *  UTF-8 and other encodings ...
                     */
                    int32_t prev = 0, raw, cooked;
                    const char *ncursor;

                    if (termlen) {              /* <CHAR>.... */
                        for (bpeol = bpsol; bpeol < bpend &&
                                    (ncursor = mchar_decode(iconv, bpeol, bpend, &raw, &cooked)) > bpeol;) {
                            if (raw == eolchar) {
                                llen = bpeol - bpsol;
                                if ((EDIT_STRIP_CR & flags) && ASCIIDEF_CR == prev) {
                                    llen -= (ncursor - bpeol);
                                    ++crs;
                                }
                                bpeol = ncursor;
                                goto done;

                            } else if (*bpeol == *termbuf && bpeol + termlen <= bpend) {
                                if (0 == memcmp(bpeol, termbuf, termlen)) {
                                    llen = bpeol - bpsol;
                                    bpeol += termlen;
                                    goto done;  /* terminator match */
                                }
                            }
                            bpeol = ncursor;
                            prev = raw;
                        }

                    } else {                    /* <CHAR> */
                        for (bpeol = bpsol; bpeol < bpend &&
                                    (ncursor = mchar_decode(iconv, bpeol, bpend, &raw, &cooked)) > bpeol;) {
                            if (raw == eolchar) {
                                llen = bpeol - bpsol;
                                if (EDIT_STRIP_CR & flags) {
                                    if (ASCIIDEF_CR == prev) {
                                        llen -= (ncursor - bpeol);
                                        ++crs;
                                    }
                                }
                                bpeol = ncursor;
                                goto done;
                            }
                            bpeol = ncursor;
                            prev = raw;
                        }
                    }
                }
            }
            llen = bpeol - bpsol;

            /*
             *  end-of-chunk conditions
             */
            if (bpeol >= bpend) {
                if (fleft > 0) {
                    if (bpsol > buffer) {       /* force line into new chunk */
                        ED_TRACE2(("line[%4d]! (%3d) <%.*s>\n", newlines, llen, llen, bpsol))
                        ovlen = llen;
                        ovbuf = bpsol;
                        break;                  /* next chunk */
                    }
                } else {
                    if  (EDIT_STRIP_CTRLZ & flags) {
                        while (llen > 0) {      /* consume trailing CTRLZ's */
                            if (ASCIIDEF_SUB != bpeol[-1]) {
                                break;
                            }
                            --llen; --bpeol;
                        }
                    }
                }
                broken = 1;
            }

            /*
             *  push line, plus user feed-back (percentage)
             */
done:;      assert(bpsol >= buffer);
            assert(bpsol < buffer + cnt);
            assert(bpeol <= buffer + cnt);
            if (NULL == (lp = line_alloc(0, FALSE, FALSE))) {
                ewprintf("%s: cannot allocate line object", fname);
                goto error;
            }

            ED_TRACE2(("line[%4d]%c (%3d) <%.*s>\n", newlines, (broken ? '+' : ' '), llen, llen, bpsol))
            if (llen > 0) {
                lp->l_text  = (void *)bpsol;
                if (broken) lflagset(lp, L_BREAK);
                lp->l_size  = llen;
                lp->l_used  = llen;
            }
            lp->l_chunk = chunk;
            ++chunkrefs;

            if (clp) {
                TAILQ_INSERT_BEFORE(clp, lp, l_node);
            } else {
                TAILQ_INSERT_TAIL(&bp->b_lineq, lp, l_node);
            }

            if (++newlines > previewlines) {
                break;
            }
                                                /* percentage done */
            if (0 == (newlines % 50) && !BFTST(bp, BF_SYSBUF)) {
                percentage(PERCENTAGE_FILE, fcursor, fsize, "Reading", fname);
            }

            bufsize += llen + 1;                /* insert plus EOL */
            bpsol = bpeol;                      /* step over line */
        }
    }

    linep_flush(bp);
    if (chunk) {                                /* close existing chunk */
        if (chunkrefs) {
            chunk_protect(bp, chunk, chunkrefs);
        } else {
            chunk_delete(bp, chunk);
        }
    }

    u_dot();
    bp->b_numlines += newlines;
    bp->b_col = 1;
    curbp = saved_bp;
    u_delete(bufsize);
    bp->b_line = cline + newlines;

    if (istream) {
        mchar_stream_close(istream);
    }
    chk_free(cache);

    /*
     *  apply file type to a new buffer, not during insert operations
     */
    if (0 == numlines) {
        if (EDIT_BINARY & flags) {
            BFSET(bp, BF_BINARY);
            bp->b_cmap = x_binary_cmap;
            bp->b_bin_chunk_size = chunksize;
            buf_type_set(bp, BFTYP_BINARY);

        } else {
            if (crs) {
                BFSET(bp, BF_CR_MODE);          /* default */
            }
            bp->b_type = fileinfo.fi_type;
            file_terminator_set(bp, termbuf, (int)termlen, fileinfo.fi_termtype);
            if (fileinfo.fi_encoding[0]) {
                buf_encoding_set(bp, fileinfo.fi_encoding);
            }
            if ((bp->b_bomlen = fileinfo.fi_bomlen) > 0) {
                memcpy(bp->b_bombuf, fileinfo.fi_bombuf, sizeof(bp->b_bombuf));
            }
        }

        trace_ilog("iconv(%s/%s)\n", mchar_encoding(iconv), mchar_internal_encoding(iconv));
        if (iconv != bp->b_iconv) {
            if (bp->b_iconv) mchar_iconv_close(bp->b_iconv);
            bp->b_iconv = iconv;
        }

    } else {
        if (iconv != bp->b_iconv) {
            mchar_iconv_close(iconv);
        }
    }
    trace_ilog("==> loaded(bytes:%d) : %d\n", (int)bufsize, (int)newlines);
    return newlines;

error:;
    curbp = saved_bp;
    linep_flush(bp);

    if (chunk) {                                /* close existing chunk */
        if (chunkrefs) {
            chunk_protect(bp, chunk, chunkrefs);
        } else {
            chunk_delete(bp, chunk);
        }
    }

    if (newlines > 0) {
        bp->b_numlines += newlines;
        if (clp) {
            while (newlines > 0 && lp) {
                clp = TAILQ_NEXT(lp, l_node);
                lrelease(bp, lp, cline - 1);
                lp = clp;
                --newlines;
            }

        } else {
            while (newlines > 0 && NULL != (lp = TAILQ_LAST(&bp->b_lineq, LineList))) {
                lrelease(bp, lp, cline - newlines);
                --newlines;
            }
        }
    }

    if (iconv) {
        if (istream) {
            mchar_stream_close(istream);
        }
        if (iconv != bp->b_iconv) {
            mchar_iconv_close(iconv);
        }
    }

    chk_free(cache);
    trace_ilog("==> loaded(bytes:%d) : -1 (error)\n", (int)bufsize);
    return -1;
}


/*  Function:           buf_writeout
 *      Write the given buffer to disk.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      fname - File name.
 *      undo - Undo information flag.
 *      append - Append flag.
 *
 *  Returns:
 *      TRUE on success, otherwise FALSE.
 *
 */
static int
buf_writeout(BUFFER_t *bp, const char *fname, int undo, int append /*const char *encoding*/)
{
    BUFFER_t *saved_bp = curbp;
    const LINENO numlines = bp->b_numlines;
    mchar_iconv_t *iconv = bp->b_iconv;
    unsigned char termbuf[16] = {0};
    int termlen, oflags;
    struct stat sb = {0};
    LINENO line = 0;
    register LINE_t *lp;

    mchar_istream_t *istream = NULL;            /* stream conversion interface */
    vfs_file_t *fp = NULL;
    int handle = -1;

    __CUNUSED(undo)

    oflags = OPEN_W_BINARY | O_WRONLY | O_CREAT;
    if (stat(fname, &sb) >= 0) {
        bp->b_mode = sb.st_mode;                /* update permissions */
    } else {
        oflags |= O_EXCL;
    }
    if (append) {
        oflags |= O_APPEND;
    } else {
        oflags |= O_TRUNC;
    }


    trace_ilog("writing(%s, %s/%s)\n", fname, mchar_encoding(iconv), mchar_internal_encoding(iconv));

    if ((handle = vfs_open(fname, oflags, 0600)) < 0) {
        system_call(-1);
        errorf("Cannot open file: %s", fname);
        return FALSE;
    }

    curbp = bp;
    termlen = file_terminator_get(bp, termbuf, sizeof(termbuf), NULL);

    infof("Writing ...");
    if (bp->b_bomlen) {                         /* BOM (Byte Order Marker) */
        if (vfs_write(handle, (const void *)bp->b_bombuf, bp->b_bomlen) != (int)bp->b_bomlen) {
            goto error;
        }
    }

    if (NULL != iconv &&
            NULL != (istream = mchar_stream_open(iconv, handle, fname, "w"))) {
        size_t outbytes = 0;

        trace_ilog("... stream_open()\n");

        TAILQ_FOREACH(lp, &bp->b_lineq, l_node) {
            const LINECHAR *text = ltext(lp);
            int length = buf_trimline(bp, text, llength(lp));

            if (length > 0) {                   /* line data */
                if (mchar_stream_write(istream, (const void *)text, length, &outbytes) != (size_t)length) {
                    goto error;
                }
            }

            if (termlen > 0) {                  /* line terminator */
                if (mchar_stream_write(istream, (const char *)termbuf, termlen, &outbytes) != (size_t)termlen) {
                    goto error;
                }
            }

            percentage(PERCENTAGE_LINES, line, numlines, "Writing", bp->b_fname);
            ++line;
        }
        mchar_stream_close(istream);

        if (0 == vfs_close(handle)) {
            infof("Write successful.");
        } else {
            errorf("Error closing file: File system may be full.");
        }

    } else {
        trace_ilog("... binary_open()\n");

        if (NULL == (fp = vfs_fdopen(fname, handle, oflags, 0600, IOBUF_SIZ))) {
            system_call(-1);
            errorf("Cannot open file: %s", fname);
            return FALSE;
        }

        TAILQ_FOREACH(lp, &bp->b_lineq, l_node) {
            const void *text = ltext(lp);
            int length = buf_trimline(bp, text, llength(lp));

            if (length > 0) {               /* line data */
                if (vfs_fwrite(fp, (const void *)text, length) != length) {
                    goto error;
                }
            }

            if (termlen > 0) {              /* line terminator */
                if (vfs_fwrite(fp, termbuf, termlen) != termlen) {
                    goto error;
                }
            }

            percentage(PERCENTAGE_LINES, line, numlines, "Writing", bp->b_fname);
            ++line;
        }

        if (0 == vfs_fclose(fp)) {
            infof("Write successful.");
        } else {
            errorf("Error closing file: File system may be full.");
        }
    }
    curbp = saved_bp;
    return TRUE;

error:;
    switch(errno) {
#if defined(ENOSPC)
    case ENOSPC:
        errorf("Write Failed: Disk Full: %s.", fname);
        break;
#endif
    default:
        errorf("Write error: %s", fname);
        break;
    }

    if (handle >= 0) {
        if (istream) {
            mchar_stream_close(istream);
        }
        vfs_close(handle);
    } else {
        vfs_fclose(fp);
    }
    curbp = saved_bp;
    return FALSE;
}


static __CINLINE int
is_white(const int c)
{
    return (' ' == c || '\t' == c);
}


static int
buf_trimline(BUFFER_t *bp, const LINECHAR *text, LINENO length)
{
    __CUNUSED(bp)
    if (length > 0 && BF4TST(curbp, BF4_OCVT_TRIMWHITE)) {
        while (length && is_white(text[length-1])) {
            --length;                           /* remove trailing whitespace */
        }
    }
    return length;
}


/*  Function:           do_edit_file
 *      edit_file and edit_file2 primitives.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: edit_file - Edit a file.

        int
        edit_file(...)

    Macro Description:
        The 'edit_file()' primitive creates a new buffer, loads the
        contents, attaches the buffer to the current window and
        executes any registered macros, see <register_macro>.

        The argument specification is a set of one or more strings
        with optional leading integer modes. Modes control the flags
        under which the subsequent files are processed.

        If the specified file is already within another buffer, that
        buffer becomes the current buffer and is attached to the
        current window.

        If more than one argument is specified; either directly or as
        the result of file expansion; then a separate edit action is
        performed on each file, with the current buffer being the
        last stated file.

    File Expansion::

        The 'edit_file()' primitive performs file name 'globbing' in
        a fashion similar to the 'csh' shell. It returns a list of
        the files whose names match any of the pattern arguments.

        The pattern arguments may contain any of the following
        special characters:

            ~[user/] -  Home directory of either the current or the
                        specified user.

            ? -         Matches any single character.

            * -         Matches any sequence of zero or more characters.

            [ch] -      Matches any single character in chars. If ch's
                        contains a sequence of the form 'a-b' then any
                        character between 'a' and 'b' (inclusive) will
                        match.

            \x -        Matches the character x.

    File detection::

        During file loading GRIEF performs a number of tests against
        the sections of the file content in an attempt to determine
        the file encoding. These operations are generally invisible
        to end user and behave on most file types without interaction.

        The current default scanners include, see <set_file_magic>
        for additional details on each.

            mark -      Encoding: < marker >
            utf32bom -  UTF-32 BOM marker.
            utf16bom -  UTF-16 BOM marker.
            udet -      Mozilla Universal Character Detector.
            magic -     File magic.
            binary -    Possible binary image.
            ascii -     ASCII only (7-bit).
            latin1 -    Latin-1 (ISO-8859-x) data.
            big5 -      Chinese Big5.
            gb18030 -   Chinese GB-18030.
            shiftjis -  Shift-JIS.
            xascii -    Extended ASCII.

    Macro Parameters:
        ... - Argument specification is a set of one or more
            strings with optional leading integer modes.

            Modes control the flags under which the subsequent files
            are processed, see examples. Unless specified files are
            loaded using *EDIT_NORMAL*.

            If no arguments are specified, the user shall be prompted
            for a file specification.

    Modes::

(start table,format=nd)
        [Constant           [Description                            ]

      ! EDIT_NORMAL         Default mode; auto-guess the file type.

      ! EDIT_BINARY         Force file to be read in binary mode,
                            see <set_binary_size>.

      ! EDIT_ASCII          Force file to be read in ASCII mode.

      ! EDIT_STRIP_CR       Force CR removal on input and write them
                            on output.

      ! EDIT_STRIP_CTRLZ    Force Ctrl-Z removal.

      ! EDIT_SYSTEM         System buffer.

      ! EDIT_RC             Enable extended return codes.

      ! EDIT_QUICK          Quick detection, removes more costly
                            character-encoding methods.

      ! EDIT_AGAIN          <edit_again> implementation.

      ! EDIT_LOCAL          Edit locally, do not rely on the disk
                            being stable.

      ! EDIT_READONLY       Set as read-only.

      ! EDIT_LOCK           Lock on read (strict-locking).

      ! EDIT_NOLOCK         Disable auto-locking.

      ! EDIT_PREVIEW        Limit the load to the window size.
(end table)

    Macro Returns:
        The 'edit_file()' primitive returns a positive value on
        success, 0 if the user was prompted and cancelled, otherwise
        -1 on error.

        If more than one argument is specified; either directly or as
        the result of file expansion; the return relates to the last
        loaded file.

        Under *EDIT_RC* the return code are extended to
        differentiation between success conditions as follows.

(start table,format=nd)
        [Value              [Desciption                             ]
      ! -1                  Error.
      ! 0                   Cancelled.
      ! 1                   Successfully loaded buffer.
      ! 2                   New image, file created.
      ! 3                   Pre-existing buffer, not reloaded.
(end table)

    Macro Example:

        Expand and load all '.cpp' files located within the current
        working directory.

>           edit_file(".cpp");

        Loads a system buffer with the content from 'config.ini'
        sourced from the users home directory.

>           edit_file(EDIT_SYSTEM, "~/config.ini");

    Macro Portability:
        The feature set exposed differs from implementations. It is
        therefore advised that the symbolic constants are using
        within a #ifdef construct.

    Macro See Also:
        edit_file2, read_file, attach_buffer, call_registered_macro,
            create_buffer, set_buffer, set_file_magic.

 *<<GRIEF>>
    Macro: edit_file2 - Extended file edit.

        int
        edit_file2(
            string encoding, string|list file)

    Macro Description:
        The 'edit_file2()' primitive extends the functionality
        provided by <edit_file> with the leading parameter of
        'encoding'.

    Macro Parameters:
        encoding - String containing the character encoding of the
            source file.

        ... - See <edit_file>

    Macro Returns:
        The 'edit_file2()' primitive returns a positive value on
        success, 0 if the user was prompted and cancelled, otherwise
        -1 on error.

        See <edit_file> for additional return information.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        edit_file, read_file, attach_buffer, call_registered_macro,
            create_buffer, set_buffer, set_file_magic.
 */
void
do_edit_file(int version)       /* int ([int mode], [string | list file ...]) */
                                /* int ([int mode], string encoding, string|list file) */
{
    const char *encoding = (2 == version ? get_xstr(1) : NULL);
    const int fileidx = (2 == version ? 2 : 1);
    acc_assign_int(1);

    if (isa_undef(fileidx)) {
        char path[MAX_PATH];

        if (NULL == get_xarg(fileidx, "Edit file: ", path, sizeof(path)))  {
            if (xf_readonly || -1 == fileio_access(curbp->b_fname, W_OK)) {
                BFSET(curbp, BF_RDONLY);
            } else {
                BFCLR(curbp, BF_RDONLY);
            }
            acc_assign_int(0);                  /* edit aborted */
            return;
        }
        file_edit(path, EDIT_NORMAL, NULL);
        return;

    } else {
        const LIST *nextlp, *lp = get_list(fileidx);
        int flags = EDIT_NORMAL;
        LISTV result;

        for (; (nextlp = atom_next(lp)) != lp; lp = nextlp) {
            const int type = eval(lp, &result);

            switch (type) {
            case F_INT:         /* mode */
                flags = result.l_int;           /* flags EDIT_SYSTEM|NORMAL etc */
                break;
            case F_STR:         /* file */
            case F_LIT:
                file_edit(result.l_str, flags, encoding);
                break;
            case F_RSTR:
                file_edit(r_ptr(result.l_ref), flags, encoding);
                break;
            }
        }
    }
}


/*  Function:           do_set_file_magic
 *      set_file_magic primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_file_magic - Define the file type detection rules.

        int
        set_file_magic([string encoding], [int cost])

    Macro Description:
        The 'set_file_magic' primitive configures the global file
        character encoding detection logic.

    Macro Parameters:
        spec - Optional file character encoding specification. If a
            non-empty string the detection rules shall be set to the
            given specification, whereas an empty string shall clear
            the current user specification, enabling the system
            default. If the specification is omitted the current
            rules remain unchanged.

        cost - Optional integer, stating the character cost the
            detection logic is permitted to incur.

    Detection Types::

        The order below is somewhat important as the MBCS checks can
        result in false positives, as such are generally last in line.

(start table)
        [Name       [Default    [Description                            ]

      ! mark        yes         Explicit "Encoding: <marker>" within
                                    the leading file content.

      ! utf32bom    yes         UTF-32 BOM marker.

      ! utf16bom    yes         UTF-16 BOM marker.

      ! utf8        yes         UTF-8

      ! bom         yes         Non UTF BOM markers.

      ! udet        yes         Mozilla Universal Character Detector,
                                    see <libcharudet>.

      ! magic       yes         File magic, see <libmagic>.

      ! binary      yes         Possible binary image.

      ! ascii       yes         ASCII only (7-bit).

      ! latin1      yes         Latin-1 (ISO-8859-x) data.

      ! big5        yes         Chinese Big5.

      ! gb18030     yes         Chinese GB-18030.

      ! shiftjis    yes         Shift-JIS.

      ! xascii      yes         Extended ASCII.

      ! charset     no          Explicit character-set.

      ! guess       n/a         see <libguess>.

(end table)

        Without going into the full details of each search algorithms,
        several are summarised below.

    mark::

        Markup languages have ways of specifying the encoding in a
        signature near the top of the file.

        The following appears inside the <head> area of an HTML page.

>         <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>

        In XML, the XML declaration specifies the encoding.

>         <?xml version="1.0" encoding="iso-8859-1"?>

        Recognised formats are.

          grief/vim -   encoding:<xxx>
          emacs -       coding:<xxx>
          html -        charset=["']<xxx>["']

    utf32bom, utf16bom::

        BOM stands for Byte Order Mark which literally is meant to
        distinguish between 'little-endian' LE and 'big-endian' BE
        byte order. For UTF-32 and UTF-16 the code point U+FEFF
        ("zero width no-break space") are used.

>         UTF-32 Big Endian         0X00,0X00,0XFE,0XFF
>         UTF-32 Little Endian      0XFF,0XFE,0X00,0X00

>         UTF-16 Big Endian         0XFE,0XFF
>         UTF-16 Little Endian      0XFF,0XFE

    uft8::

        UTF-8 auto-detection (when there is no UTF-8 BOM), performs
        UTF-8 decoding on the file looking for an invalid UTF-8
        sequence; correct UTF-8 sequences look like this:

>         0xxxxxxx                              ASCII < 0x80 (128)
>         110xxxxx 10xxxxxx                     2-byte >= 0x80
>         1110xxxx 10xxxxxx 10xxxxxx            3-byte >= 0x400
>         11110xxx 10xxxxxx 10xxxxxx 10xxxxxx   4-byte >= 0x10000

    bom::

        BOM stands for Byte Order Mark which literally is meant to
        distinguish between 'little-endian' LE and 'big-endian' BE
        byte order.

        For Unicode files, the BOM ("Byte Order Mark" also called the
        signature or preamble) is a set of leading bytes at the
        beginning used to indicate the type of Unicode encoding.

        The key to the 'BOM' is that it is generally not included
        with the content of the file when the file's text is loaded
        into memory, but it may be used to affect how the file is
        loaded into memory.

        Recognised sequences include;

>         UTF-32 Big Endian         0X00,0X00,0XFE,0XFF
>         UTF-32 Little Endian      0XFF,0XFE,0X00,0X00

>         UTF-16 Big Endian         0XFE,0XFF
>         UTF-16 Little Endian      0XFF,0XFE

>         UTF-7                     0X2B,0X2F,0X76,0X38
>         UTF-7                     0X2B,0X2F,0X76,0X39
>         UTF-7                     0X2B,0X2F,0X76,0X2B
>         UTF-7                     0X2B,0X2F,0X76,0X2F

>         UTF-EBCDIC                0XDD,0X73,0X66,0X73
>         GB18030                   0X84,0X31,0X95,0X33
>         BOCU-1                    0XFB,0XEE,0X28,0xFF
>         BOCU-1                    0XFB,0XEE,0X28
>         SCSU                      0X0E,0XFE,0XFF
>         UTF-1                     0XF7,0X64,0X4C
>         UTF-8                     0XEF,0XBB,0XBF

    udet::

        Mozilla Universal Character Detector employs a composite
        approach that utilizes Code Scheme, Character Distribution
        and 2-Char Sequence Distribution methods to identify
        language/encodings has been proven to be very effective and
        efficient in our environment; see <libcharudet>.

        http://www-archive.mozilla.org/projects/intl/UniversalCharsetDetection.html

    big5::

        Performs *Big5* decoding on the file looking for an invalid
        sequences.

        Big5 does not conform to the ISO-2022 standard, but rather
        bears a certain similarity to Shift-JIS.

        It is a double-byte character set with the following structure.

>           First Byte:         0xA1 - 0xf9 (non-user-defined characters)
>                           or  0x81 - 0xfe (extended range)
>           Second Byte:        0x40 - 0x7e or 0xa1 - 0xfe

    gb18030::

        Performs *GB18030* decoding on the file looking for an invalid
        sequences.

        GB18030-2000 has the following significant properties;

            * It incorporates Unicode's CJK Unified Ideographs
                Extension A completely.

            * It provides code space for all used and unused code
                points of Unicode's plane 0 (BMP) and its 15 additional
                planes. While being a code- and character-compatible
                "superset" of GBK, GB18030-2000, at the same time,
                intends to provide space for all remaining code points
                of Unicode. Thus, it effectively creates a one-to-one
                relationship between parts of GB18030-2000 and Unicode's
                complete encoding space.

            * In order to accomplish the Unihan incorporation and
                code space allocation for Unicode 3.0, GB18030-2000
                defines and applies a four-byte encoding mechanism.

        GB18030-2000 encodes characters in sequences of one, two, or
        four bytes. The following are valid byte sequences (byte
        values are hexadecimal):

>           Single-byte: 0x00-0x7f
>           Two-byte:    0x81-0xfe + 0x40-0x7e, 0x80-0xfe
>           Four-byte:   0x81-0xfe + 0x30-0x39 + 0x81-0xfe + 0x30-0x39

        The single-byte portion applies the coding structure and
        principles of the standard GB 11383 (identical to ISO
        4873:1986) by using the code points 0x00 through 0x7f.

        The two-byte portion uses two eight-bit binary sequences to
        express a character. The code points of the first (leading)
        byte range from 0x81 through 0xfe. The code points of the
        second (trailing) byte ranges from 0x40 through 0x7e and 0x80
        through 0xfe.

        The four-byte portion uses the code points 0x30 through 0x39,
        which are vacant in GB 11383, as an additional means to
        extend the two-byte encodings, thus effectively increasing
        the number of four-byte codes to now include code points
        ranging from 0x81308130 through 0xfe39fe39.

        GB18030-2000 has 1.6 million valid byte sequences, but there
        are only 1.1 million code points in Unicode, so there are
        about 500, 000 byte sequences in GB18030-2000 that are
        currently unassigned.

    shiftjis::

        Performs *Shift-JIS* decoding on the file looking for an
        invalid sequences.

          o Single Byte

>           ASCII:                  0x21 - 0x7F (also allow control)
>           Katakana:               0xA1 - 0xDF

          o Multiple Byte

>           JIS X 0208 character
>           First byte:             0x81 - 0x9F or 0xE0 - 0xEF
>           Second byte (old 1st):  0x40 - 0x9E
>           Second byte (even 1st): 0xA0 - 0xFD

    guess::

        'libguess' employs discrete-finite automata to deduce the
        character set of the input buffer. The advantage of this is
        that all character sets can be checked in parallel, and
        quickly. Right now, libguess passes a byte to each DFA on the
        same pass, meaning that the winning character set can be
        deduced as efficiently as possible; see <libguess>.

    Macro Returns:
        The 'set_file_magic()' primitive returns a positive value on
        success, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        edit_file, inq_file_magic
 */
void
do_set_file_magic(void)         /* int ([string spec], [int bytes]) */
{
    if (! isa_undef(1)) {
        const char *spec = get_xstr(1);

        chk_free((void *) x_encoding_guess);
        x_encoding_guess =
            (spec && *spec ? chk_salloc(spec) : NULL);
    }
}


/*  Function:           inq_file_magic
 *      inq_file_magic primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_file_magic - Retrieve the file type detection rules.

        string
        inq_file_magic([int &isdefault])

    Macro Description:
        The 'inq_file_magic' primitive retrieves the current file
        character encoding detection rules. The returned shall
        contain one or comma separated detector names with optional
        arguments, see <set_file_magic>.

        Example::

>           mark,utf8,udet,xascii,ascii

    Macro Parameters:
        n/a

    Macro Returns:
        The 'inq_file_magic()' primitive returns a string containing
        the current encoder specification.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        edit_file, set_file_magic
 */
void
inq_file_magic(void)            /* string ([int &isdefault], [int &cost]) */
{
    if (x_encoding_guess && *x_encoding_guess) {
        acc_assign_str(x_encoding_guess, -1);

    } else {
        const char *guess = mchar_guess_default();
        acc_assign_str(guess, -1);
        chk_free((void *)guess);
    }
}


/*  Function:           file_edit
 *      Work horse for the edit_file primitive.
 *
 *  Parameters:
 *      fname - File name.
 *      flags - Edit flags (EDIT_XXX).
 *
 *  Returns:
 *      nothing
 */
void
file_edit(const char *fname, const int32_t flags, const char *encoding)
{
    char **files = 0;
    char path[MAX_PATH];

    strxcpy(path, fname, sizeof(path));         /* local working buffer */

    trace_log("file_edit(fname:%s,flags:0x%x,encoding:%s)\n",\
            fname, flags, encoding ? encoding : "n/a");

    if (NULL == (files = shell_expand(path))) {
        errorf("Name expansion error.");
        acc_assign_int(0);                      /* edit aborted */

    } else {
        unsigned j;

        for (j = 0; files[j]; ++j) {
            if (files[j][0]) {
                int ret;

                file_canonicalize(files[j], path, sizeof(path));
                if ((ret = file_load(path, flags, encoding)) <= -1) {
                    acc_assign_int(-1);         /* error */

                } else if (EDIT_RC & flags) {
                    /*
                     *  3 = Preexisting buffer, not reloaded.
                     *  2 = New image, file created.
                     *  1 = Success.
                     */
                    acc_assign_int(1 + ret);

                } else {
                    acc_assign_int(1);
                }
            }
        }
        shell_release(files);
    }
}


/*  Function:           file_load
 *      file_load.
 *
 *  Parameters:
 *      fname - File name.
 *      flags - Edit flags (EDIT_XXX).
 *
 *  Returns:
 *      -1    - Error.
 *      0     - Success.
 *      1     - New File.
 *      2     - Pre existing buffer, not reloaded.
 *
 *<<GRIEF>> [callback]
    Macro: _extension - Buffer load handler.

        void
        _extension(string ext)

    Macro Description:
        The '_extension()' callback is executed whenever GRIEF edits
        a file via the <edit_file> primitive.

        It is provided to allow macros to hooks buffer loads, for
        example to setup defaults tabs before any extension specific
        settings are applied.

        Once executed if defined the extension specific handler shall
        be executed, which should be named as '_ext'. If not
        available the default extension <_default> handler is executed.

        The extension case shall be preserved on case sensitive
        file-systems otherwise the extension is converted to lower
        case.

    Note!:
        This interface is considered defunct; <register_macro> is
        the preferred method of capturing buffer edit events.

    Macro Parameters:
        ext - File extension.

    Macro Returns:
        nothing

    Macro Portability:
        The 'ext' parameter is a Grief extension.

    Macro See Also:
        edit_file, _default

 *<<GRIEF>> [callback]
    Macro: _default - Default extension handler.

        void
        _default(string ext)

    Macro Description:
        The '_default()' callback is executed whenever GRIEF edits a file
        via the <edit_file> primitive and a extension specific macro of
        the format '_ext' was not available to be executed.

        It is provided to allow macros to hooks buffer loads, for
        example to setup defaults tabs on a file extension basis.

        The extension case shall be preserved on case sensitive
        file-systems otherwise the extension is converted to lower case.

    Note!:
        This interface is considered defunct; <register_macro> is
        the preferred method of capturing buffer edit events.

    Macro Parameters:
        ext - File extension.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        edit_file, _extension
 */
int
file_load(const char *fname, const int32_t flags, const char *encoding)
{
    register BUFFER_t *bp;
    const char *cp;
    int ret;                                    /* success */

    trace_log("file_load(fname:%s,flags:0x%x,encoding:%s)\n",\
                fname, flags, encoding ? encoding : "n/a");

    if (NULL == (bp = buf_find_or_create(fname)))  {
        return -1;
    }

    buf_show(bp, curwp);

    if (0 == (EDIT_AGAIN & flags) && BFTST(bp, BF_READ)) {
        trace_log("=> already(2)\n");
        curbp = bp;                             /* already read */
        set_hooked();
        ret = 2;

    } else {
        const int noundo = BFTST(bp, BF_NO_UNDO);
        char *execbuf;

        /* Load the image
         *
         *  o lock (if required/enabled)
         *      - strict locking and *not* a SYSTEM buffer.
         *      - EDIT_LOCK stated.
         *  o load the image.
         */
        BFSET(bp, BF_NO_UNDO);                  /* disable UNDO (restored later) */
        BFSET(bp, BF_READ);                     /* read status */

                                                /* lock */
        if ((xf_strictlock && 0 == (EDIT_SYSTEM & flags) && 0 == BFTST(bp, BF_SYSBUF))
                || (EDIT_LOCK & flags)) {
            if ((ret = flock_set(fname, TRUE)) != -1) {
                bp->b_lstat = 1;
            }
        } else {
            ret = 0;
        }

        if (EDIT_AGAIN & flags) {
            while (bp->b_numlines > 0) {        /* NEWLINE */
                lremove(bp, 1);
            }
        }

        if (-1 == ret ||
                (ret = file_readin(bp, fname, flags, encoding)) < 0) {
            buf_show(curbp, curwp);
            buf_kill(bp->b_bufnum);
            trace_log("=> error(-1)\n");
            return -1;
        }

        assert(0 == ret || 1 == ret);           /* 0=success, 1=created */

        if (BFTYP_UNDEFINED == bp->b_type) {
            if (encoding) {
                buf_encoding_set(bp, encoding);
            } else {
                buf_type_default(bp);
            }
        }

        curbp = bp;
        set_hooked();

        lrenumber(bp);
        if (!noundo) {
            BFCLR(bp, BF_NO_UNDO);
        }

        /*
         *  Execute a macro based on the file extension to set options eg. tabs etc.
         */
        for (cp = fname + strlen(fname) - 1; cp > fname; cp--) {
            if ('.' == *cp || FILEIO_ISSEP(*cp)) {
                break;                          /* extension or base-name */
            }
        }

        if (cp > fname && '.' == *cp &&         /* _extension and _default */
                    NULL != (execbuf = chk_alloc(32 + strlen(cp) + 1))) {

            char *ext = chk_salloc(cp + 1);
            file_case(ext);                     /* Address case insensitive file-systems */

            if (macro_lookup("_extension")) {
                sprintf(execbuf, "_extension \"%s\"", ext);
                execute_str(execbuf);           /* eg. _extension "c" */
            }

            strcpy(execbuf + 1, ext);
            *execbuf = '_';                     /* leading underscore, replace '.' */

            if (macro_lookup(execbuf)) {
                execute_str(execbuf);           /* eg. _c */

            } else if (macro_lookup("_default")) {
                sprintf(execbuf, "_default \"%s\"", ext);
                execute_str(execbuf);           /* eg. _default "c" */
            }

            chk_free(ext);
            chk_free(execbuf);

        } else {                                /* no extension */
            if (macro_lookup("_extension")) {
                execute_str("_extension");
            }

            if (macro_lookup("_default")) {
                execute_str("_default");
            }
        }
        trigger(REG_NEW);
    }

    trigger(REG_EDIT);
    trace_log("=> edit(%d)\n", ret);
    return ret;
}


/*  Function:           file_rdonly
 *      Test whether the current buffer is read-only and echo to the user
 *      the message "File is read only" if this is the case.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      *0* if the buffer is not read-only, otherwise *-1* if read-only.
 */
int
file_rdonly(BUFFER_t *bp, const char *who)
{
    if (NULL == bp || BFTST(bp, BF_RDONLY)) {   /* XXX - or BF_MAN ??*/
        if (who && *who) {
            errorf("%s: file is read only", who);
        } else {
            errorf("File is read only");
        }
        return -1;
    }

    if (0 == bp->b_lstat) {                     /* lazy locking, lock on first modification */
        if (BFTST(bp, BF_LOCK)) {
            if (0 == BFTST(bp, BF_CHANGED) && bp->b_fname[0]) {
                if (-1 == flock_set(bp->b_fname, TRUE)) {
                    return -1;
                }
                bp->b_lstat = 1;                /* locked */
            }
        }
    }
    return 0;
}


int
rdonly(void)
{
    return file_rdonly(curbp, NULL);
}


/*  Function:           file_attach
 *      Associate/initialise file resources associate with the specified
 *      buffer, called during buffer construction.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
file_attach(BUFFER_t *bp)
{
    bp->b_lstat = 0;
    bp->b_bkdir = NULL;
    bp->b_bkprefix = NULL;
    bp->b_bksuffix = NULL;
    bp->b_bkversions = -1;
    bp->b_bkask = -1;
    bp->b_bkdont = -1;
}


/*  Function:           file_cleanup
 *      Cleanup any file resources associate with the buffer, called on
 *      buffer destruction.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
file_cleanup(BUFFER_t *bp)
{
    if (1 == bp->b_lstat) {
        flock_clear(bp->b_fname);
    }
    bp->b_lstat = 0;

    if (bp->b_bkdir) {
        chk_free((void *)bp->b_bkdir), bp->b_bkdir = NULL;
    }

    bp->b_bkversions = -1;

    if (bp->b_bkprefix) {
        chk_free((void *)bp->b_bkprefix), bp->b_bkprefix = NULL;
    }

    if (bp->b_bksuffix) {
        chk_free((void *)bp->b_bksuffix), bp->b_bksuffix = NULL;
    }
}


/*  Function:           second_passed
 *      second_passed.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
int
second_passed(void)
{
    static time_t last_time = 0;
    time_t tnow = time(NULL);

    if (tnow == last_time) {
        return FALSE;
    }
    last_time = tnow;
    return TRUE;
}


/*  Function:           percentage
 *      percentage.
 *
 *  Parameters:
 *      what - Percentage of type.
 *      pos - Position within object.
 *      sz - Size of object.
 *      str - Operation.
 *      file - Object name, for example filename.
 *
 *  Returns:
 *      nothing
 */
void
percentage(
    int what, accuint_t pos, accuint_t sz, const char *str, const char *str1)
{
    static accuint_t cache_sz, cache_pc = 0;
    static unsigned counter;
    accuint_t pc;

    __CUNUSED(what)

    /*
     *  Determine percentage
     */
    if (sz >= 100000) {
        pc = pos / (sz / 100);                  /* larger values */
    } else {
        pc = (pos * 100) / sz;                  /* smaller values */
    }

    /*
     *  Change?
     *
     *      If the percentage is changing quickly then try and avoid slugging
     *      the system by calling the time() system call in second_passed()
     */
    if (cache_sz == sz) {
        if (0 == pc || cache_pc == pc || counter++ < 127) {
            return;
        }
        counter = 0;
    } else {
        cache_sz = sz;
        counter = 0;
        return;
    }

    if (second_passed()) {
        char iobuf[MAX_CMDLINE], m;

        m = 'b';
        if (sz > 1024) {
            if ((sz /= 1024) < 1024) {
                m = 'k';
            } else if ((sz /= 1024) < 1024) {
                m = 'M';
            } else {
                sz /= 1024;
                m = 'G';
            }
        }
        sxprintf(iobuf, sizeof(iobuf), "[%s %%s%%s: %ld%%%%/%d%c done]", str, pc, (int)sz, m);
        infof_truncated(iobuf, str1);
        cache_pc = pc;
    }
}


/*  Function:           buf_rollbackups
 *      Function called to roll/remove the named file.
 *
 *      The file isn't actually removed, if the GRVERSIONS environment variable
 *      is set, then we keep that number of old versions of the file, and
 *      remove the last one.
 *
 *      If GRVERSIONS is not set then we simply delete the old file, depending
 *      on remove_flag.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      path - File path.
 *      remove_flag - if *true* remove single file image.
 *
 *  Returns:
 *      nothing
 */
static void
buf_rollbackups(BUFFER_t *bp, const char *path, int remove_flag)
{
#if defined(_VMS)
    if (remove_flag) {
        unlink(path);
    }

#else
    const char *filename;
    char oname[MAX_PATH], nname[MAX_PATH];
    int dirlen, bversion;

    /*
     *  retrieve version level
     */
    bversion = bkcfg_versions(bp);
    trace_log("\tVERSION=%d\n", bversion);
    if (bversion <= 1) {
        if (remove_flag) {
            fileio_unlink(path);
        }
        return;
    }

    /*
     *  copy .../8/file to .../9/file, etc
     */
    filename = sys_basename(path);
    assert(filename > path);
    dirlen = (filename - path) - 1;             /* length of base directory */

    strcpy(oname, path); file_slashes(oname);
    strcpy(nname, path); file_slashes(nname);
    while (bversion-- > 0) {
        /*
         *  Move file from one dir to another.
         *      If we fail because the dir doesn't exist, try and make it.
         */
        if (bversion) {                         /* $BACKUP/<version>/<filename> */
            sprintf(oname + dirlen, "%c%d%c%s", PATH_SEPERATOR, bversion, PATH_SEPERATOR, filename);

        } else {                                /* $BACKUP/<filename> */
            sprintf(oname + dirlen, "%c%s", PATH_SEPERATOR, filename);
        }
                                                /* $BACKUP/<version+1>/<filename> */
        sprintf(nname + dirlen, "%c%d%c%s", PATH_SEPERATOR, bversion + 1, PATH_SEPERATOR, filename);
        fileio_unlink(nname);

        if (0 == fileio_access(oname, F_OK)) {
            if (rename(oname, nname) < 0) {
                char *tcp = strrchr(nname, PATH_SEPERATOR);

                *tcp = 0;
                (void) fileio_mkdir(nname, 0777 & ~x_umask);
                *tcp = PATH_SEPERATOR;
                if (-1 == rename(oname, nname)) {
                    eeprintx("unable to rename '%s' to '%s'", oname, nname);
                }
            }
        }
    }
#endif  /*!_VMS*/
}


static int
fname_trim(char *path)
{
    char *cp, *fname = (char *)sys_basename(path);

    /* remove the trailing extension */
    if (fname) {
        ++fname;                                /* avoid removing /.something */
        if (NULL != (cp = strrchr(fname, '.'))) {
            *cp = 0;                            /* remove current extension */
            return TRUE;
        }
    }
    return FALSE;
}


static int
fname_prefix(char *path, const char *prefix)
{
    if (NULL != prefix && prefix[0]) {
        char *fname = (char *)sys_basename(path);
        const int p = (int)strlen(path);
        const int l = (int)strlen(prefix);

        if (p + l > MAX_PATH) {
            errno = ENAMETOOLONG;
            return -1;
        }
        memmove(fname, (const char *)fname + l, (size_t) l);
        memcpy(fname, prefix, (size_t) l);
    }
    return (0);
}


static int
fname_suffix(char *path, const char *suffix)
{
    if (path && suffix && suffix[0]) {
        const int p = (int)strlen(path), l = (int)strlen(suffix);

        if (p + l > MAX_PATH) {
            errno = ENAMETOOLONG;
            return -1;
        }
        memcpy(path + p, suffix, l + 1);        /* suffix plus term */
    }
    return 0;
}


/*  Function:           buf_backup
 *      Rename the file "fname" into a backup copy.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
static int
buf_backup(BUFFER_t *bp)
{
    const char *fname = bp->b_fname;
    const char *file_name = sys_basename(fname);
    const char *prefix = bkcfg_prefix(bp);
    const char *suffix = bkcfg_suffix(bp);
    mode_t perms = bp->b_mode;
#if defined(HAVE_CHOWN)
    uid_t owner = bp->b_uid;
    gid_t group = bp->b_gid;
#else
    uid_t owner = (uid_t) -1;
    gid_t group = (gid_t) -1;
#endif

    const char *bbenv, *bbptr;
    char nname[MAX_PATH], *np;
    int backed_up = 0;

#if defined(HAVE_LINK)
    struct stat sb;
    int r;
#endif

//  file_slashes(fname);
//  file_name = strrchr(fname, PATH_SEPERATOR);
//  if (file_name) {
//      ++file_name;
//  } else {
//      file_name = fname;
//  }

    bbenv = bkcfg_dir(bp);
    trace_log("\tDIR=%s\n", (bbenv ? bbenv : ""));
    trace_log("\t=%s\n", fname);

    if (NULL == bbenv || !*bbenv) {
        infof("GRBACKUP is not available ...");
        return TRUE;
    }

#if defined(HAVE_LINK)
#if defined(HAVE_LSTAT)
    /* Let's look at the *real* entry and see if it is a symbolic link.  */
    r = lstat(fname, &sb);
    if (r == 0 && (sb.st_mode & S_IFLNK)) {
        stat(fname, &sb);
        sb.st_nlink = 1 + 1;                    /* force backup via copy method */

    } else if (r < 0) {
        sb.st_nlink = 1 + 1;
    }
#else
    r = stat(fname, &sb)
    if (r < 0) {
        sb.st_nlink = 1 + 1;
    }
#endif /*HAVE_LSTAT*/

    /*
     *  If file has more than one link to it, dont try and link the file to
     *  the backup directory otherwise we lose the link information and
     *  confuse the user.
     */
#if defined(HAVE_CHOWN)
    else if (owner != (uid_t) -1 && group != (uid_t) -1) {
        /*
         *  If file is owned by someone else, chown() may fail, so we force
         *  backup-by-copy here so that the original ownership remains
         *  unchanged.
         */
        gid_t euid = geteuid();
        gid_t egid = getegid();

        if (euid != owner || egid != group) {
            sb.st_nlink = 1 + 1;
        }
    }
#endif  /*HAVE_CHOWN*/

    /*
     *  See if we can link file to backup directory
     */
    if (1 == sb.st_nlink) {
        for (bbptr = bbenv; bbptr && *bbptr;) {
            /* copy path */
            for (np = nname; *bbptr;) {
                if (FILEIO_DIRDELIM == *bbptr) {
                    ++bbptr;                    /* skip over delimiter */
                    break;
                }
                *np++ = *bbptr++;
            }

            if (np == nname) {
                continue;                       /* empty */
            }

            /* expand */
            *np++ = PATH_SEPERATOR;
            strcpy(np, file_name);
            file_expand(nname, nname, sizeof(nname));

            /* attempt backup */
            if (! backed_up) {
                buf_rollbackups(bp, nname, TRUE);
                ++backed_up;
            }

            if (link(fname, nname) >= 0 && unlink(fname) >= 0) {
                return TRUE;
            }
        }
    }
#endif  /*HAVE_LINK*/

    /*
     *  See if we can copy file to backup directory
     */
    for (bbptr = bbenv; bbptr && *bbptr;) {
        /* copy path */
        for (np = nname; *bbptr;) {
            if (FILEIO_DIRDELIM == *bbptr) {
                ++bbptr;                        /* skip over delimiter */
                break;
            }
            *np++ = *bbptr++;
        }
        if (np == nname) {
            continue;                           /* empty */
        }

        /* expand */
        *np++ = PATH_SEPERATOR;
        strcpy(np, file_name);
        file_expand(nname, nname, sizeof(nname));

        /* attempt backup */
        if (! backed_up) {
            buf_rollbackups(bp, nname, TRUE);
            ++backed_up;
        }

        perms |= S_IWUSR;                       /* make sure "we" can write */

        if (FALSE == file_copy(fname, nname, perms, owner, group)) {
            continue;
        }

        trace_log("\tA->%s\n", nname);
        return TRUE;
    }

    /*
     *  Copying file to backup directory failed (maybe because the directory
     *  is not writable or GRBACKUP is not set. So now we try the old way of
     *  adding an extension (.bak by default).
     */
    strcpy(nname, fname);

    if (bkcfg_oneext() && suffix[0]) {
        fname_trim(nname);                      /* remove trailing extension */
    }

    fname_prefix(nname, prefix);                /* prefix (if any) */
    fname_suffix(nname, suffix);                /* suffix (if any) */

    /* XXX - file_copy() checks fname != nname */
#if defined(DOSISH)
    if (TRUE == file_copy(fname, nname, perms, owner, group)) {
        trace_log("\tB->%s\n", nname);
        return TRUE;
    }

    if (!bkcfg_oneext() && suffix[0]) {
        /*
         *  try removing the extension
         *      XXX - not sure if this is a sane idea!!
         */
        if (fname_trim(nname)) {
            fname_trim(nname);
        }
        fname_suffix(nname, suffix);
        if (TRUE == file_copy(fname, nname, perms, owner, group)) {
            trace_log("\tC->%s\n", nname);
            return TRUE;
        }
    }
    return FALSE;

#else   /*!DOSISH*/
    if (sb.st_nlink != 1) {
        int ret = file_copy(fname, nname, perms, owner, group);

        if (ret) {
            trace_log("\tD->%s\n", nname);
        }
        return ret;
    }

    /* no rename on SysV, so do it dangerous way. */

    (void) unlink(nname);                       /* ignore errors */
    if (link(fname, nname) < 0 || unlink(fname) < 0) {
        return FALSE;
    }
    return TRUE;
#endif
}


/*  Function:           file_copy
 *      Copy file from one to another, and set permisions on new file This is
 *      used for file backups where other links exist for a file or links are
 *      not supported.
 *
 *  Parameters:
 *      src - Source file.
 *      dst - Destination file.
 *      perms - Permissions.
 *      owner - Owner identifier.
 *      group - Group identifier.
 *
 *  Returns:
 *      *TRUE* if successful, otherwise *FALSE*.
 */
static int
file_copy(
    const char *src, const char *dst, mode_t perms, uid_t owner, gid_t group)
{
    int ret;

    if (0 == file_cmp(src, dst)) {              /* Same image? */
        return FALSE;
    }

    if ((ret = sys_copy(src, dst, perms, owner, group)) >= 0) {
        return (ret ? TRUE : FALSE);

    } else {
        char iobuf[IOBUF_SIZ];
        int ofd, ifd;
        FSIZE_t n;

        if ((ofd = fileio_open(dst, OPEN_W_BINARY | O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
            return FALSE;
        }

        if ((ifd = fileio_open(src, OPEN_R_BINARY | O_RDONLY, 0)) < 0) {
            fileio_close(ofd);
            fileio_unlink(dst);
            return TRUE;
        }

        (void) fileio_chmod(dst, perms);        /* FIXME: return */
#ifdef HAVE_CHOWN
        if (-1 == chown(dst, owner, group))
            ewprintf("warning: unable to chown(%s)", dst);
#else
        __CUNUSED(owner)
        __CUNUSED(group)
#endif

        while ((n = sys_read(ifd, iobuf, sizeof(iobuf))) > 0) {
            sys_write(ofd, iobuf, n);
        }

        fileio_close(ofd);
        fileio_close(ifd);
    }
    return TRUE;
}


/*  Function:           file_cmp
 *      Compare file names taking local system dependent requirements into
 *      consideration.
 *
 *  Parameters:
 *      f1 - File name
 *      f2 - Second filename.
 *
 *  Returns:
 *      The file_cmp routine returns a value less than, equal to, or greater
 *      than zero (0).
 *
 *      A returned value of less than zero (0) indicates that the first file
 *      is lexicographically less than the second filename. A returned value
 *      of zero (0) means that both filenames are equal. A returned value of
 *      greater than zero means that the first filename is lexicographically
 *      greater than the second filename.
 */
int
file_cmp(
    const char *f1, const char *f2)
{
    if (f1 != f2)
        for (;;) {
            if (file_cmp_char(*f1, *f2) == 0) {
#if defined(NOCASE_FILENAMES)
                return (tolower(*((unsigned char *)f1)) - tolower(*((unsigned char *)f2)));
#else
                return (*f1 - *f2);
#endif
            }
            if (*f1++ == '\0') {
                break;
            }
            ++f2;
        }
    return (0);                                 /* match */
}


int
file_ncmp(
    const char *f1, const char *f2, int len)
{
    if (f1 != f2 && len > 0)
        do {
            if (file_cmp_char(*f1, *f2) == 0) {
#if defined(NOCASE_FILENAMES)
                return (tolower(*((unsigned char *)f1)) - tolower(*((unsigned char *)f2)));
#else
                return (*f1 - *f2);
#endif
            }
            if (*f1++ == '\0') {
                break;
            }
            ++f2;
        } while (--len != 0);
    return (0);                                 /* match */
}


static int
file_cmp_char(
    const int c1, const int c2)
{
    if (c1 == c2) return 1;

#if defined(NOCASE_FILENAMES)
    if (tolower(c1) == tolower(c2)) {
        return 1;
    }
#endif
#if defined(DOSISH)         /* / and \ */
    if ((c1 == '\\' || c1 == '/') && (c2 == '\\' || c2 == '/')) {
        return 1;
    }
#endif
    return 0;
}


/*  Function:           file_case
 *      Convert the filename case.
 *
 *  Parameters:
 *      str - String buffer.
 *
 *  Returns:
 *      Original buffer.
 */
char *
file_case(char *str)
{
#if defined(MONOCASE_FILENAMES)
    register char *cp;

#if defined(WIN32)
    if (1 == sys_fstype(str)) {
        return str;
    }
#endif
    for (cp = str; *cp; ++cp) {
        unsigned char ch = *((unsigned char *)cp);
        if (isupper(ch)) {
            *cp = (char)tolower(ch);
        }
    }
#else
    __CUNUSED(str)
#endif
    return str;
}


/*  Function:           file_slashes
 *      Convert backslashes to forward slashes (if required)
 *
 *  Parameters:
 *      str - String buffer.
 *
 *  Returns:
 *      Original buffer.
 */
char *
file_slashes(char *str)
{
#if defined(DOSISH)         /* / and \ */
    register char *cp = str;

    for (; *cp; ++cp) {
        if (FILEIO_ISSEP(*cp)) {
            *cp = PATH_SEPERATOR;                     /* standardise to the system separator */
        }
    }
#endif
    return str;
}


/*  Function:           file_tilder
 *      This function expands any C-shell style home directory ~[user]
 *      references/short-hands contained within the specified 'path',
 *      returning the result.
 *
 *      The supported C-shell constructs are:
 *
 *          ~/              is expanded to your current home directory.
 *
 *          ~user/          is expanded to the specified 'users' home directory (unix only).
 *
 *  Parameters:
 *      file - File to be expanded.
 *      path - Derived path buffer.
 *      legn - Length of the buffer, in length.
 *
 *  Returns:
 *      Original buffer.
 */
char *
file_tilder(const char *file, char *path, int len)
{
    assert(file);
    assert(path && len > 32);

#if defined(_VMS)
    if (file != path) {
        strxcpy(path, file, len);
    }
    return path;

#elif defined(DOSISH)       /* / and \ */
    if ('~' == *file && (file[1] == '/' || file[1] == '\\')) {
        char t_name[MAX_PATH];
        const char *home;

        if (NULL == (home = sysinfo_homedir(NULL, -1))) {
            return NULL;
        }

        if (path == file) {                     /* same buffer */
            sxprintf(path, len, "%s%c%s", home, file[1], strxcpy(t_name, file + 2, sizeof(t_name)));
        } else {
            sxprintf(path, len, "%s%c%s", home, file[1], file + 2);
        }

    } else if (file != path) {
        strxcpy(path, file, len);
    }

    file_slashes(path);
    return path;

#else
    if ('~' == *file) {
        /*
         *  ~/ ==> home directory of user.
         */
        const char *ofile = file;
        char t_name[MAX_PATH];
        struct passwd *pwd;
                                                /* ~[/...] */
        if (*++file == PATH_SEPERATOR || 0 == *file) {
            if (PATH_SEPERATOR == *file) {
                ++file;
            }
            pwd = getpwuid(getuid());
        } else {                                /* ~<name>/... */
            char *cp;

            for (cp = t_name; *file && *file != PATH_SEPERATOR;) {
                *cp++ = *file++;
            }
            if (PATH_SEPERATOR == *file) {
                ++file;
            }
            *cp = 0;
            pwd = getpwnam(t_name);
        }

        if (NULL == pwd) {
            path = NULL;                        /* unknown user */
        } else {
            if (path == ofile) {                /* same buffer */
                sxprintf(path, len, "%s/%s", pwd->pw_dir, strxcpy(t_name, file, sizeof(t_name)));
            } else {
                sxprintf(path, len, "%s/%s", pwd->pw_dir, file);
            }
        }
        endpwent();

    } else if (path != file) {
        strxcpy(path, file, len);
    }
    return path;
#endif
}


/*  Function:           file_modedesc
 *      Describe the specified 'mode' using the ls style notation.
 *
 *  Parameters:
 *      mode - Permissions mode.
 *      format - Output format (0 = standard, 1=extended; ls -lF style).
 *      buffer - Destination buffer.
 *      len - Length of buffer, in bytes.
 *
 *  Returns:
 *      Original buffer.
 */
char *
file_modedesc(mode_t mode, const char *source, int format, char *buffer, int len)
{
    char t_buffer[16];

    __CUNUSED(source)

    /* type */
    if (S_ISDIR(mode)) {
        t_buffer[0] = (format ? '/' : 'd');     /* directory */

#if defined(S_ISCHR)
    } else if (S_ISCHR(mode)) {
        t_buffer[0] = (format ? '-' : 'c');     /* character device */
#endif
#if defined(S_ISBLK)
    } else if (S_ISBLK(mode)) {
        t_buffer[0] = (format ? '+' : 'b');     /* block device */
#endif
#if defined(S_ISLNK)
    } else if (S_ISLNK(mode)) {
        if (format && source && source[0]) {
            /*
             *  ~   - directory link.
             *  @   - link.
             *  !   - broken linkOB.
             */
            struct stat st = {0};

            if (-1 == vfs_stat(source, &st) || 0 == st.st_mode) {
                t_buffer[0] = '!';
            } else if (S_ISDIR(st.st_mode)) {
                t_buffer[0] = '~';
            } else {
                t_buffer[0] = '@';
            }
        } else {
            t_buffer[0] = (format ? '@' : 'l'); /* link */
        }
#endif
#if defined(S_ISFIFO)
    } else if (S_ISFIFO(mode)) {
        t_buffer[0] = (format ? '|' : 'p');     /* fifo/pipe */
#endif
#if defined(S_ISSOCK)
    } else if (S_ISSOCK(mode)) {
        t_buffer[0] = (format ? '=' : 's');     /* sockets */
#endif
#if defined(S_ISNAM)
    } else if (S_ISNAM(mode)) {
        t_buffer[0] = (format ? '$' : 'n');     /* name */
#endif
#if defined(S_ISDOOR)
    } else if (S_ISDOOR(mode)) {
        t_buffer[0] = (format ? '$' : 'D');     /* door */
#endif
#if defined(S_ISWHT)
    } else if (S_ISWHT(mode)) {
        t_buffer[0] = (format ? '$' : 'w');     /* whiteout */
#endif
    } else {
        if (format) {
            if (mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
                t_buffer[0] = '*';              /* executable */
            } else {
                t_buffer[0] = ' ';              /* normal */
            }
        } else {
            t_buffer[0] = '-';
        }
    }

    /* permissions */
#if !defined(S_IWGRP)                           /* MINGW32 etc */
#define S_IRGRP         S_IRUSR
#define S_IWGRP         S_IWUSR
#define S_IXGRP         S_IXUSR
#endif
#if !defined(S_IROTH)
#define S_IROTH         S_IRUSR
#define S_IWOTH         S_IWUSR
#define S_IXOTH         S_IXUSR
#endif
    t_buffer[1] = (mode & S_IRUSR ? 'r' : '-'); /* read permission: owner */
    t_buffer[2] = (mode & S_IWUSR ? 'w' : '-'); /* write permission: owner */
    t_buffer[3] = (mode & S_IXUSR ? 'x' : '-'); /* execute permission: owner */
    t_buffer[4] = (mode & S_IRGRP ? 'r' : '-'); /* read permission: group */
    t_buffer[5] = (mode & S_IWGRP ? 'w' : '-'); /* write permission: group */
    t_buffer[6] = (mode & S_IXGRP ? 'x' : '-'); /* execute permission: group */
    t_buffer[7] = (mode & S_IROTH ? 'r' : '-'); /* read permission: other */
    t_buffer[8] = (mode & S_IWOTH ? 'w' : '-'); /* write permission: other */
    t_buffer[9] = (mode & S_IXOTH ? 'x' : '-'); /* execute permission: other */
    t_buffer[10] = '\0';

    /* sticky bits */
#if defined(S_ISUID) && (S_ISUID)
    if (mode & S_ISUID) {
        t_buffer[3] = (t_buffer[3] == 'x') ? 's' : 'S';
    }
#endif
#if defined(S_ISGID) && (S_ISGID)
    if (mode & S_ISGID) {
        t_buffer[6] = (t_buffer[6] == 'x') ? 's' : 'S';
    }
#endif
#if defined(S_ISVTX) && (S_ISVTX)
    if (mode & S_ISVTX) {
        t_buffer[9] = (t_buffer[9] == 'x') ? 't' : 'T';
    }
#endif

    if (NULL == buffer) {
        return chk_salloc(t_buffer);
    }
    strncpy(buffer, t_buffer, (size_t) len);
    return buffer;
}


/*  Function:           file_getenv
 *      Performs an in-line expansion of environment variable references.
 *
 *      It understands the following syntax. If a macro is not defined it expands to
 *      "", and {} are synonyms for ().
 *
 *          $$          Expands to $.
 *          $(name)     Expands to value of name.
 *          ${name}     As above, expands to value of name.
 *          $name[/|\]  As above, expands to value of name.
 *
 *  Parameters:
 *      path - Address of the path buffer.
 *      len - Length of the buffer, in bytes.
 *
 *  Returns:
 *      Path buffer.
 *
 */
char *
file_getenv(char *path, int len)
{
    char *dp = path, *dpend = dp + len;

    while (*dp) {
        if (*dp != '$') {
            /*
             *  standard character
             */
            ++dp;

        } else if (dp[1] == '$') {
            /*
             *  $$ = $
             */
            memmove(dp, (const char *)dp + 1, varlen(dp, dpend));
            ++dp;

        } else {
            /*
             *  $var, $(var) and ${var}
             */
            const char *name = dp + 1;
            const char delim = ('(' == *name ? ')' : '{' == *name ? '}' : 0);
            const char *end  = varend(dp, dpend, delim);
            const char *var;
            size_t vlen, remaining;

            if (delim) {
                if (*end != delim) {
                    dp = (char *)end;           /* unterminated */
                    continue;
                }
                ++name;                         /* consume open */
            }

            if (NULL != (var = ggetnenv(name, end - name))) {
                vlen = strlen(var);
            } else {    /*nomatch*/
                vlen = 0;
                var = "";
            }

            if (delim && *end == delim) ++end;  /* consume close */

            remaining = varlen(end, dpend);
            if ((dp + vlen + remaining) >= dpend) {
                *dp = '\0';                     /* buffer overflow */
                break;
            }
            memmove(dp + vlen, end, remaining + 1);
            memcpy(dp, var, vlen);
        }
    }
    return path;
}


static size_t
varlen(const char *dp, const char *dpend)
{
    const char *dpstart = dp;

    while (dp < dpend && *dp) {
        ++dp;
    }
    return (dp - dpstart);
}


static char *
varend(char *dp, char *dpend, const int what)
{
    while (dp < dpend && *dp) {
        if (*dp == what || FILEIO_ISSEP(*dp)) {
            return dp;
        }
        ++dp;
    }
    return dp;
}


/*  Function:           file_expand
 *      Expand the specified file 'file' into the buffer 'path' of the
 *      length 'len' in bytes.
 *
 *      The following elements shall be processed:
 *
 *          ~/          Is expanded to your current home directory.
 *          ~user/      Is expanded to the specified 'users' home directory (unix only).
 *          $$          Expands to $.
 *          $(name)     Expands to value of name.
 *          ${name}     As above, expands to value of name.
 *          $name[/|\]  As above, expands to value of name.
 *
 *  Parameters:
 *      file - File path to be expanded.
 *      path - Address of the path buffer.
 *      len - Length of the buffer, in bytes.
 *
 *  Returns:
 *      Path buffer.
 */
char *
file_expand(const char *file, char *path, int len)
{
    return file_getenv(file_tilder(file, path, len), len);
}


#if (NOT_USED)
/*  Function:           file_pathexpand
 *      Cook the paths within the path spec.
 *
 *  Parameters:
 *      file - File path to be expanded.
 *      path - Address of the destination path buffer.
 *      len - Length of the buffer, in bytes.
 *
 *  Returns:
 *      Path buffer.
 */
char *
file_pathexpand(const char *file, char *path, int len)
{
    char *p = path;
    char nname[MAX_PATH], *np;
    int nlen;

    while (*file) {
        /* copy path */
        for (np = nname; *file;) {
            if (FILEIO_DIRDELIM == *file) {
                ++file;                         /* skip over delimiter */
                break;
            }
            *np++ = *file++;
        }

        if (np == nname) {
            continue;                           /* empty */
        }
        *np = '\0';

        /* expand */
        file_expand(nname, nname, sizeof(nname));
        nlen = strlen(nname);

        /* copy results */
        len -= nlen + 1;                        /* length plus delimiter */
        if (len < 1)
            return NULL;
        memcpy(p, (const char *)nname, nlen);
        p += nlen;
        *p++ = FILEIO_DIRDELIM;
        *p = '\0';
    }
    return path;
}
#endif


/*  Function:           file_chdir
 *      Set the current working directory for the specified drive.
 *
 *  Parameters:
 *      dir - Directory path.
 *
 *  Returns:
 *      zero(0) on success, otherwise non-zero.
 */
int
file_chdir(const char *dir)
{
    x_cwd[0] = '\0';                            /* clear cache */
#if defined(DOSISH)         /* X: */
    x_cwdd[0] = '\0';
#endif
    if (dir && *dir) {
#if defined(DOSISH)         /* X: */
        if (':' == dir[1] && isalpha(*((unsigned char *)dir))) {
            if (-1 == sys_drive_set(dir[0])) {  /* change drive */
                return -1;
            }
        }
#endif
#if defined(NOT_VFS)
        return sys_chdir(dir);
#else   /*VFS*/
        return vfs_chdir(dir);
#endif
    }
    return -1;
}


/*  Function:           file_cwd
 *      Retrieve the current working directory.
 *
 *  Parameters:
 *      cwd - Working buffer (optional).
 *      length - Length of the working buffer, in bytes.
 *
 *  Returns:
 *      Address of buffer containing the current working directory.
 *
 */
char *
file_cwd(char *cwd, unsigned length)
{
#if defined(NOT_VFS)
    sys_cwd(x_cwd, sizeof(x_cwd));
    file_slashes(x_cwd);
    file_case(x_cwd);
    if (cwd & length) {
        strxcpy(cwd, (const char *)x_cmd, length);
        return cwd;
    }
    return x_cwd;

#else   /*VFS*/
    if (vfs_cwd(x_cwd, sizeof(x_cwd)))
        if (cwd) {
            strxcpy(cwd, (const char *)x_cwd, length);
            return cwd;
        }
    return x_cwd;
#endif
}


/*  Function:           file_cwdd
 *       Retrieve the current working directory for the specified drive.
 *
 *  Parameters:
 *      drv - Drive letter 'A' - 'Z'.
 *      cwd - Working buffer (optional).
 *      length - Length of the working buffer, in bytes.
 *
 *  Returns:
 *      Address of buffer containing the current working directory.
 */
char *
file_cwdd(int drv, char *cwdd, unsigned length)
{
    drv = toupper(drv);

#if defined(DOSISH)         /* X: */
    sys_cwdd(drv, x_cwdd, sizeof(x_cwdd));      /* drive specific directory */
    file_slashes(x_cwdd);
    file_case(x_cwdd);
    if (cwdd) {
        strxcpy(cwdd, (const char *)x_cwdd, length);
        return cwdd;
    }
    return x_cwdd;

#else
    return file_cwd(cwdd, length);              /* ignore drive request */
#endif
}


/*  Function:           file_canonicalize
 *       Canonicalize the specific filename.
 *
 *  Description:
 *      Canonicalize path and return a new path.
 *
 *      The new path differs from path in:
 *
 *      o Slashes are normalised with '\' replaced with '/'.
 *
 *      o Relative paths are prefixed with the current
 *          working directory.
 *
 *      o Multiple `/'s are collapsed to a single `/'.
 *
 *      o Leading `./'s and trailing `/.'s are removed.
 *
 *      o Trailing `/'s are removed.
 *
 *      o Non-leading `../'s and trailing `..'s are handled by
 *          removing portions of the path.
 *
 *  Parameters:
 *      filename - Source path name.
 *      path - Resulting path.
 *      length - Length of the resulting path buffer, in bytes.
 *
 *  Notes:
 *      Look at using realpath() and/or resolvepath().
 */
char *
file_canonicalize(const char *filename, char *path, int length)
{
    if (path && filename != path && length >= MAX_PATH) {
        file_canonicalize2(filename, path);     /* normal case, correctly sized buffer */
        return path;

    } else {
        char t_path[MAX_PATH];

        file_canonicalize2(filename, t_path);
        if (path && length > 0) {               /* local result */
            strxcpy(path, (const char *)t_path, length);
            return path;
        }
        return chk_salloc(t_path);              /* dynamic */
    }
    /*NOTREACHED*/
}


static void
file_canonicalize2(const char *filename, char *path)
{
    char t_filename[MAX_PATH];
    int unc = FALSE, len;
    char *p, *s;

    strxcpy(t_filename, filename, sizeof(t_filename));
    filename = t_filename;                      /* working copy */

#if defined(_VMS)
    if (strchr(filename, PATH_SEPERATOR) != NULL) {
        filename = sys_fname_unix_to_vms(filename, path, sizeof(t_filename));
    }

    if (filename != path) {
        strxcpy(path, (const char *)filename, sizeof(t_filename));
    }

    if (strchr(path, ':') == NULL) {
        vms_filename_canon(path);
    }

#else   /*!VMS*/

#if defined(DOSISH)                             /* normalize */
    file_slashes(t_filename);
#endif

    /* preserve UNC paths (//servername/...) */
    if (PATH_SEPERATOR == filename[0] && PATH_SEPERATOR == filename[1]) {
        const char *cursor = filename + 2;

        while (cursor[0] && PATH_SEPERATOR != *cursor) {
            ++cursor;
        }

        if (*cursor && cursor > filename + 2) { /* trailing separator */
            while (filename < cursor) {
                *path++ = *filename++;
            }
            strcpy(path, filename);
            unc = TRUE;
        }
    }

    /* absolute or relative to current directory */
    if (! unc) {
        const char *cwd = NULL;

#if defined(DOSISH)         /* X: */
        char drv = 0;

        if (isalpha(*((unsigned char *)filename)) && ':' == filename[1]) {
            drv = (char)toupper((unsigned char)*filename); filename += 2;
            cwd = file_cwdd(drv, NULL, 0);      /* drive specific path */
            if (*cwd && ':' == cwd[1]) {
                drv = cwd[0];
                cwd += 2;
            }
        } else {
            cwd = file_cwd(NULL, 0);
            if (isalpha(*((unsigned char *)cwd)) && ':' == cwd[1]) {
                drv = cwd[0];
                cwd += 2;
                                                /* UNC path (//servername/...) */
            } else if (PATH_SEPERATOR == cwd[0] && PATH_SEPERATOR == cwd[1]) {
                *path++ = *cwd++;
                *path++ = *cwd++;
                while (*cwd && PATH_SEPERATOR != *cwd) {
                    *path++ = *cwd++;           /* preserve UNC */
                }
                unc = TRUE;
            }
        }
        if (drv > 0) {                          /* assign and preserve drive */
            *path++ = drv;
            *path++ = ':';
        }
#endif /*DOSISH*/

        if (PATH_SEPERATOR == *filename) {      /* absolute */
            strcpy(path, filename);
        } else {
            if (NULL == cwd) cwd = file_cwd(NULL, 0);
            if (PATH_SEPERATOR == cwd[0] && 0 == cwd[1]) {
                                                /* /<fn> */
                sprintf(path, "%c%s", PATH_SEPERATOR, filename);
            } else {                            /* <cwd/fn> */
                sprintf(path, "%s%c%s", cwd, PATH_SEPERATOR, filename);
            }
        }
    }

    /* collapse multiple slashes and convert slashes */
    p = path;
    while (*p) {
        if (PATH_SEPERATOR == p[0] && PATH_SEPERATOR == p[1]) {
            s = p + 1;
            while (*(++s) == PATH_SEPERATOR) {
                /*continue*/;
            }
            str_cpy(p + 1, s);
        }
        ++p;
    }

    /* collapse "/./" -> "/" */
    p = path;
    while (*p) {
        if (PATH_SEPERATOR == p[0] && '.' == p[1] && PATH_SEPERATOR == p[2]) {
            str_cpy(p, p + 2);
        } else {
            ++p;
        }
    }

    /* remove trailing slashes */
    p = path + strlen(path) - 1;
    while (p > path && PATH_SEPERATOR == p[0]) {
        *p-- = 0;
    }

    /* remove leading "./" */
    if ('.' == path[0] && PATH_SEPERATOR == path[1]) {
        if (path[2] == 0) {
            path[1] = 0;                        /* . */
            return;
        }
        str_cpy(path, path + 2);
    }

    /* remove trailing "/" or "/." */
    if ((len = (int)strlen(path)) < 2) {
        return;
    }

    if (PATH_SEPERATOR == path[len - 1]) {      /* XXX/  -> XXX */
        path[len - 1] = 0;

    } else if ('.' == path[len - 1] && PATH_SEPERATOR == path[len - 2]) {
        if (2 == len) {                         /* /.    -> /   */
            path[1] = 0;
            return;
        }
        path[len - 2] = 0;                      /* XXX/. -> XXX */
    }

    /* collapse "/.." with the previous part of path */
    p = path;
    while (p[0] && p[1] && p[2]) {
        if ((PATH_SEPERATOR != p[0] || '.' != p[1] || '.' != p[2]) ||
                (PATH_SEPERATOR != p[3] && p[3] != 0)) {
            ++p;
            continue;
        }

        /* search for the previous token */
        s = p - 1;
        while (s >= path && *s != PATH_SEPERATOR) {
            --s;
        }
        ++s;
        if ('.' == s[0] && '.' == s[1] && s + 2 == p) {
            p += 3;                             /* prev token is "..", cannot collapse */
            continue;
        }

        if (p[3] != 0) {
            if (s == path && PATH_SEPERATOR == *s) {
                str_cpy(s + 1, p + 4);          /* "/../XXX"    -> "/XXX" */
            } else {
                str_cpy(s, p + 4);              /* "yyy/../XXX" -> "XXX" */
            }
            p = (s > path) ? s - 1 : s;
            continue;
        }

        /* trailing ".." */
        if (s == path) {                        /* "yyy/.."     -> "." */
            if (PATH_SEPERATOR != path[0]) {
                path[0] = '.';
            }
            path[1] = 0;
        } else {                                /* "XXX/yyy/.." -> "XXX" */
            if (s == (path + 1)) {
                s[0] = 0;                       /* /XXX*/
            } else {
                s[-1] = 0;                      /* XXX */
            }
            break;
        }
        break;
    }
#endif  /*! _VMS*/
}
/*end*/
