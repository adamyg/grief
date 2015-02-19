/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: nroff.cr,v 1.10 2014/10/27 23:28:25 ayoung Exp $
 * Filter a buffer through nroff
 *
 *
 */

#include "grief.h"


static void nroff_keys(void);

void
main()
{
    load_macro("man");
}


void
nroff()
{
    int tmp_buf, tmp_win, curbuf = inq_buffer();
    string infile, outfile;
    int mark_set;

    message("formatting text...");
    refresh();

    /* If no marked region, then do the whole buffer */
    mark_set = inq_marked();
    if (!mark_set)
    {
        save_position();
        top_of_buffer();
        drop_anchor(MK_LINE);
        end_of_buffer();
    }

    sprintf(infile, "%s/gr%06d.in", inq_tmpdir(), getpid());
    write_block(infile, NULL, TRUE);
    sprintf(outfile, "%s/gr%06d.out", inq_tmpdir(), getpid());
    shell("nroff -man | col -b", 0, NULL, fixslash(infile), fixslash(outfile));
    remove(infile);

    if ((tmp_buf = create_buffer("Nroff-Output", outfile, 1)) != -1) {
        remove(outfile);
        set_buffer(curbuf);
        if (!mark_set) {
            raise_anchor();
            restore_position();
        }
        message("<Esc> to exit nroff view.");
        tmp_win = sized_window(inq_lines(tmp_buf), inq_line_length(tmp_buf) + 1, "");
        select_buffer(tmp_buf, tmp_win, SEL_NORMAL, nroff_keys());
        delete_buffer(tmp_buf);
    }
    set_buffer(curbuf);
    attach_buffer(curbuf);
}


static void
nroff_keys(void)
{
    assign_to_key("<Alt-S>",      "search__fwd");
    assign_to_key("<Alt-Y>",      "search__back");
    assign_to_key("<F5>",         "search__fwd");
    assign_to_key("<F6>",         "translate__fwd");
    assign_to_key("<Shift-F5>",   "search_next");
    assign_to_key("<Keypad-5>",   "search_next");
    assign_to_key("<Shift-F6>",   "search_prev");
    assign_to_key("<Alt-N>",      "search_prev");
}

/*end*/
