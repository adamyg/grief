/* $Id: savehist.cr,v 1.7 2018/10/01 21:05:01 cvsuser Exp $
 * History save and restore.
 *
 * This macro saves the current history information to the state file on
 * exit. To use it, put the string "-msavehist" (no quotes) after the
 * "-mstate" in your GRFLAGS.
 *
 * For example:
 *    set GRFLAGS=-mstate -msavehist
 * or
 *    restore: savehist=yes
 *
 *
 */

#include "grief.h"


void
savehist(void)
{
}


void
_savehist_save(void)
{
    int histbuf = _inq_history();

    if (histbuf > 0) {
        int end_line, end_col, curbuf;

        curbuf = set_buffer(histbuf);
        save_position();
        end_of_buffer();
        up(); end_of_line();
        inq_position(end_line, end_col);
        restore_position();
        set_buffer(curbuf);

        if (end_line) {
            end_of_buffer();
            insert("[grief-history]\n");
            transfer(histbuf, 1, 1, end_line, end_col);
            insert("[grief-history-end]\n");
        }
    }
}

void
_savehist_restore(void)
{
    int histbuf = _inq_history();

    save_position();
    top_of_buffer();
    if (histbuf > 0 &&
            search_fwd("<\\[grief-history\\]>")) {

        down();
        drop_anchor(4);
        search_fwd("<\\[grief-history-end\\]>");

        if (inq_mark_size() > 0) {
            int start_line, start_col, end_line, end_col, curbuf;

            up(); end_of_line();
            inq_position(end_line, end_col);
            swap_anchor();
            inq_position(start_line, start_col);
            curbuf = set_buffer(histbuf);
            if (end_line >= start_line)         /* history exists */
               transfer(curbuf, start_line, start_col, end_line, end_col);
            set_buffer(curbuf);
        }

        raise_anchor();
    }
    restore_position();
}

/*end*/

