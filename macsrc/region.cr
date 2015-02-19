/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: region.cr,v 1.9 2014/10/27 23:28:26 ayoung Exp $
 * Region management.
 *
 *
 */

#include "grief.h"

#define BLOCK_NOTHING   0                       /* Do nothing with original text */
#define BLOCK_REPLACE   1                       /* Delete original text in block */

#if defined(__PROTOTYPES__)
static int              block_uc(string line);
static int              block_lc(string line);
static int              block_sum1(string line);
static void             block(string macro_name, ~int, ~string msg);
#endif

static string           block_string;           /* Buffer saved for column cut/copy */

replacement void
paste()
{
    int type;
    list lines;
    int len, col, line_no;
    int i;
    int old_msg_level;

    inq_scrap(NULL, type);
    if (type != MK_COLUMN) {
        old_msg_level = inq_msg_level();
        if (inq_called() == "")
            set_msg_level(0);
        paste();
        set_msg_level(old_msg_level);
        return;
    }

    /* Split the column string into separate lines. */
    lines = split(block_string, "\n");

    /* Now paste them in, one at a time. */
    len = length_of_list(lines);
    inq_position(line_no, col);
    for (i = 0; i < len;) {
        message("%pPasting column: %d%% done", (i * 100) / len);
        insert(lines[i++]);
        move_abs(line_no + i, col);
    }
    message("Scrap inserted.");
}


replacement void
copy()
{
    int old_msg_level;

    if (inq_called() != "") {
        copy();
        return;
    }

    if (inq_marked()) {
        old_msg_level = inq_msg_level();
        set_msg_level(0);
        if (inq_marked() == MK_COLUMN) {
            block_string = "";
            block("::block_copy", 0, "%pCopying column: %d%% done");
            set_scrap_info(NULL, MK_COLUMN);
            message("Column copied to scrap.");
        } else {
            copy();
        }
        set_msg_level(old_msg_level);
        return;
    }

    drop_anchor(MK_LINE);
    message("Line copied to scrap.");
    copy();
}


replacement void
cut()
{
    int old_msg_level;

    if (inq_called() != "") {
        cut();
        return;
    }

    if (inq_marked()) {
        old_msg_level = inq_msg_level();
        set_msg_level(0);
        if (inq_marked() == MK_COLUMN) {
            block_string = "";
            block("::block_cut", 0, "%pCutting column: %d%% done");
            set_scrap_info(NULL, MK_COLUMN);
            message("Column cut to scrap.");
        } else {
            cut();
        }
        set_msg_level(old_msg_level);
        return;
    }

    drop_anchor(MK_LINE);
    message("Line cut to scrap.");
    cut();
}


/*
 *  block_cut ---
 *      Macro called on each line when we try and cut a column block.
 */
static int
block_cut(string line)
{
    block_string += line + "\n";
    return BLOCK_REPLACE;
}


/*
 *  block_copy ---
 *      Macro called on each line when we try and copy a column block.
 */
static int
block_copy(string line, int width)
{
    string s = trim(line, "\n");

    s += " " * (width - strlen(s));
    block_string += s + "\n";
    return BLOCK_NOTHING;
}


/*
 *  block_upper_case ---
 *      Convert the current block to upper-case
 */
void
block_upper_case(void)
{
    block("::block_uc", 0, "%pUpper case: %d%% done");
}


static int
block_uc(string line)
{
    insert(upper(line));
    return BLOCK_REPLACE;
}


/*
 *  block_lower_case ---
 *      Convert the current block to lower-case
 */
void
block_lower_case(void)
{
    block("::block_lc", 0, "%pLower case: %d%% done");
}


static int
block_lc(string line)
{
    insert(lower(line));
    return BLOCK_REPLACE;
}


/*
 *  sum ---
 *      Sum the leading values within the block
 */
void
sum(void)
{
    float sum = 0;
    int buf = inq_buffer();
    string s;

    block("::block_sum1", 0, "%pSumming column: %d%% done");

    /* Copy the number to the scrap buffer. */
    set_buffer(inq_scrap());
    clear_buffer();
    sprintf(s, "%g", sum);
    insert(s + "\n");
    set_buffer(buf);
    message("Result=%g - copied to scrap.", sum);
}


static int
block_sum1(string line)
{
    extern float sum;
    declare x;

    x = cvt_to_object(line);
    switch (typeof(x)) {
    case "float":
    case "integer":
        sum += x;
        break;
    default:
        break;
    }
    return BLOCK_NOTHING;
}


void
block_delete(void)
{
    block("", BLOCK_REPLACE);
}


static void
block(string macro_name, ~int, ~string msg)
{
    int type, start_line, start_col, end_line, end_col;
    int col = 1, result, width, size, cnt, total;
    string line;

    if (msg == "") {
        msg = "%p%d%% done";                    /* default, display time. */
    }

    type = inq_marked(start_line, start_col, end_line, end_col);
    if (type == 0) {
        error("No marked region.");
        return;
    }

    if (type == MK_COLUMN)
        col = start_col;
    raise_anchor();

    move_abs(start_line, start_col);
    total = end_line - start_line;
    width = end_col - start_col;
    cnt = 0;

    while (start_line <= end_line) {
        message(msg, (cnt++ * 100) / total);

        move_abs(start_line, col);
        drop_anchor(MK_NORMAL);
        save_position();
        if (type == MK_COLUMN || start_line == end_line) {
            move_abs(0, end_col);
        } else {
            end_of_line();
            prev_char();
        }
        size = inq_mark_size();
        raise_anchor();
        restore_position();

        line = rtrim(read(size), "\n");
        if (macro_name != "") {
            result = execute_macro(macro_name, line, width);
        } else {
            get_parm(1, result);
        }

        switch (result) {
        case BLOCK_REPLACE:
            delete_char(strlen(line));
            break;
        }

        ++start_line;
    }
    move_abs(end_line, end_col);
}

/*end*/
