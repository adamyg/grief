#include <edidentifier.h>
__CIDENT_RCSID(gr_m_region_c,"$Id: m_region.c,v 1.11 2022/05/31 16:18:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_region.c,v 1.11 2022/05/31 16:18:21 cvsuser Exp $
 * Block primitives.
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

#include "m_region.h"                           /* public header */

#include "accum.h"
#include "anchor.h"
#include "builtin.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "mac1.h"
#include "main.h"
#include "map.h"
#include "procspawn.h"
#include "region.h"
#include "symbol.h"
#include "system.h"
#include "undo.h"
#include "wild.h"
#include "window.h"


/*  Function:           do_delete_block
 *      delete_block primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_block - Deleted selected region.

        int
        delete_block()

    Macro Description:
        The 'delete_block()' primitive deleted the current marked block,
        leaving the cursor position on the last line of the marked region.

        The characters included in the mark depend on the its type, and
        once deleted the mark is removed.

        On completion the following is echoed on the command prompt.

>           Block deleted.

        In the event of no active region the following is echoed.

>           No marked block.

    Macro Parameters:
        none

    Macro Returns:
        The 'delete_block()' primitive returns one if the block was
        delete, otherwise zero.

    Macro See Also:
        cut, copy, delete_char
 */
void
do_delete_block(void)           /* int delete_block() */
{
    REGION_t r;

    acc_assign_int((accint_t) 0);
    if (rdonly())
        return;
    if (! region_get(FALSE, FALSE, NULL, &r))
        return;
    region_delete(&r);
    anchor_raise();
    infof("Block deleted.");
    acc_assign_int((accint_t) 1);
}


/*  Function:           do_write_block
 *      write_block primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: write_block - Write selected region.

        int
        write_block([string filename],
            [int append = FALSE], [int keep = FALSE],
            [int pause = TRUE])

    Macro Description:
        The 'write_block()' primitive writes out the currently marked
        region to the file 'filename'. If filename is not specified,
        then it is prompted for as follows

>           Write marked area as:

        The characters included in the mark depend on the its type, and
        once written the mark is removed unless 'keep' is specified.

        Writing out a marked region does not affect the backup flag or
        the undo information flag; see <undo> and <set_backup>.

        On completion the following is echoed on the command prompt.

>           Write successful.

        In the event of no active region the following is echoed.

>           No marked block.

      *File Name*

        Several special leading characters within the stated filename
        act as modifiers on the output mode of the file.

        '|' - data is written to a pipe instead of a file. The string
            content after the '|' is passed as an argument to popen().

        '>', '>>' - data shall be appended to the specified file
            following the '>'; same effect as stated 'append'.

    Macro Parameters:
        filename - Optional string value containing the path of the
            destination filename. If omitted the user shall be prompted.

        append - Optional boolean value, if true the block is appended to
            the end of the file; otherwise the file content is replaced.

        keep - Optional boolean value, if true the marked region is
            retainined, otherwise on completion the region is cleared.

        pause - Optional boolean value control pipe completion handling.
            During pipe operations the command may destroy the screen.
            If omitted or is non-zero then the user is prompted to hit
            <Enter> before continuing.

    Macro Returns:
        The 'write_block()' primitive returns one on success, otherwise
        zero on error.

    Macro Portability:
        The 'filename' and 'append' options are Grief extensions to BRIEF.

    Macro See Also:
        delete_block, write_buffer, undo, set_backup
 */
void
do_write_block(void)            /* int ([string fname], [int append = FALSE], [int keep = FALSE]) */
{
    const char *open_mode = (0 == get_xinteger(2, 0) ? "w" : "a");
    const int keep = get_xinteger(3, FALSE);
    const int pipe_msg = ((x_display_ctrl & DC_WINDOW) ? FALSE : get_xinteger(4, TRUE));

    const char *fname;
    char buf[MAX_PATH] = {0}, path[MAX_PATH];
    int pipe_open = FALSE;
    char *mem = NULL;
    FILE *fp = NULL;
    REGION_t r;

    acc_assign_int(0);

    /*
     *  Arrange region.
     */
    if (! region_get(FALSE, FALSE, NULL, &r)) {
        return;
    }

    if (NULL == (fname =
            get_arg1("Write marked area as: ", buf, sizeof(buf)))) {
        return;
    }

    /*
     *  If leading characters are '>' or '>>' characters then override the
     *  'open_mode' parameter allowing user to select the append mode.
     */
    while (isspace(*fname)) ++fname;
    if ('>' == *fname) {                        /* >[>] */
        if ('>' == *++fname) ++fname;
        while (isspace(*fname)) ++fname;
        open_mode = "a";                        /* append mode */
    }

    /*
     *  If first character of file name is a pipe symbol, then open up a pipe
     *  to write the data. This allows people to send things to the print spooler.
     */
    if ('|' == *fname) {                        /* PIPE */
        pipe_open = TRUE;
        fp = sys_popen(fname + 1, open_mode);
        if (fp != NULL && pipe_msg) {           /* allow user to view exec */
            proc_prep_stop(TRUE);
        }
    } else {
        pipe_open = FALSE;
        shell_expand0(fname, path, sizeof(path));
        fp = fopen(path, open_mode);            /* TODO -- VFS open */
        fname = path;
    }

    if (NULL == fp) {
        ewprintf("Write failed <%s>.", fname);
        acc_assign_int(-1);
        chk_free(mem);
        return;
    }

    region_write(fp, pipe_open ? (const char *)NULL : "Writing", &r);

    if (pipe_open) {
        sys_pclose(fp);
        if (pipe_msg) {
            static const char contmsg[] = "Press any <Enter> to continue: ";

            if (sys_write(1, contmsg, (sizeof(contmsg) - 1)) > 0) {
                if (sys_read(0, buf, sizeof(buf)) < 0) {
                    infof("<return?>");
                }
            }
            proc_prep_start();
        }
    } else {
        fclose(fp);
        infof("Write successful.");
    }

    if (! keep) {
        anchor_raise();                         /* remove region, unless keep */
    }

    acc_assign_int(1);
    chk_free(mem);
}
/*end*/
