/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: print.cr,v 1.4 2014/10/27 23:28:25 ayoung Exp $
 * Print functions.
 *
 *
 */

#include "grief.h"

/* This variable is set from the .grinit file and is used to print files
 */
static string     print_cmd  = "";


/*
 *  print ---
 *      Function to print the current buffer. We set a region over the whole file and
 *      do a write_block() allowing user to select the appopriate print spooler. If
 *      this macro is called with an argument, then we assume that we are being passed
 *      the default command for printing so we just save it away without actually
 *      printing.
 */
replacement void
print(void)
{
    int old_msg_level;
    string filename;
    int is_marked = inq_marked();

    if (print_cmd != "") {
        filename = print_cmd;
    } else {
        if (get_parm(NULL, filename, "Type print command: ") <= 0 ||
            filename == "")
        return;
    }

    if (!is_marked) {
        save_position();
        top_of_buffer();
        drop_anchor(MK_LINE);
        end_of_buffer();
    }

    old_msg_level = set_msg_level(1);
    write_block(filename);
    set_msg_level(old_msg_level);
    if (is_marked) {
        raise_anchor();
    } else {
        raise_anchor();
        restore_position();
    }
}


void
_griset_print(string arg)
{
    arg = ltrim(arg);
    if (substr(arg, 1, 1) != "|")
        arg = "|" + arg;
    print_cmd = arg;
}


/*
 *  Function called on exit to save the print command.
 */
string
_griget_print(void)
{
    return print_cmd;
}

/*end*/
