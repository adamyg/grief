/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: setup.cr,v 1.8 2024/05/11 17:44:46 cvsuser Exp $
 * System setup.
 *
 *
 */

#include "grief.h"

static list         setup_menu = {
        " Color preferences           =>",
        " Echo line options           =>",
        " Searching/translate options =>",
        " Language packages           =>",
    //  " Autosave & Backups          =>",
    //  " Color preferences           =>",
    //  " Colorizer colors            =>",
    //  " Key bindings                =>",
    //  " Keyboard settings           =>",
    //  " Language editing modes      =>",
    //  " Memory configuration        =>",
    //  " Miscellaneous Items         =>",
    //  " Printer                     =>",
    //  " Screen settings             =>",
    //  " Startup                     =>",
    //  " State saving                =>",
        "                               ",
        " Save options to disk",
        " Save options and exit setup",
        " Abort setup without saving",
        };

static list         bracket_styles = {
        "K&R",
        "GRIEF",
        "Unindented",
        "Indented",
        "GNU"
        };

void                package_setup(void);
void                popup_message(string title, string msg);


void
main(void)
{
}


void
setup(~int inform)
{
    int  do_warning;
    int ret;

    if (inform) {
        popup_message("Setup",
            "This is the either the first time you have run GRIEF\n" +
            "or this specific release of GRIEF has newer features\n" +
            "to your previous version. \n" +
            "\n" +
            "The appropriate environment variables needed to run GRIEF\n" +
            "have not been setup, and may stop you from running GRIEF.\n" +
            "\n" +
            "GRIEF will now display a Set Up menu where you should\n" +
            "configure your terminal type. \n" +
            "\n"+
            "After the Set Up is complete, GRIEF will exit. You should\n" +
            "then try and run GRIEF again.  If set up was successful you\n" +
            "will no longer see this message.\n");
    }

    do_warning = FALSE;
    while (1) {
        extern int window_offset;               /* sized_window() globals */
        int o_window_offset = window_offset;

        window_offset = 999;
        message("Use <Tab> to move cursor, <Enter> to select.");
        ret = select_list("Setup Menu", "", 1, setup_menu, SEL_NORMAL);
        window_offset = o_window_offset;

        if (ret <= 0) {
            if (do_warning) {
                error( "Warning: setup has not been saved on disk" );
            } else {
                message("");
            }
            return;
        }

        switch (ret) {
        case 1: setcolor(); break;
        case 2: echo_line_options(); break;
        case 3: search_options(); break;
        case 4: package_setup(); break;
    //  case 15:
    //      do_warning = FALSE;
    //      write_setup_file();
    //      break;
    //  case 17:
    //      if (inform)
    //          exit();
    //      message("");
    //      return;
    //  case 99:
    //      if (write_setup_file() > 0) {
    //          message("");
    //      }
    //      return;
        }
    }
}


void
package_setup(void)
{
}


/*
 *
 */
void
popup_message(string title, string msg)
{
    extern int top_line;                        /* sized_window() globals */
    extern int window_offset;                   /* sized_window() globals */

    int o_window_offset = window_offset;
    int o_top_line = top_line;
    int curbuf, buf, win;
    int i, height, width;
    int screen_rows, screen_cols;
    list lst;

    curbuf = inq_buffer();                      /* build message buffer */
    buf = create_buffer(title, NULL, 1);
    set_buffer(buf);
    lst = split(msg, "\n" );
    width = length_of_list(lst);
    for (i = 0; i < width; i++)
        insert(lst[i] + "\n");
    height = inq_lines()+1;
    width = inq_line_length()+2;

    set_buffer(curbuf);                         /* restore buffer */
    inq_screen_size(screen_rows, screen_cols);
     top_line = (screen_rows - height)/2;
    window_offset = (screen_cols - width)/2;
    win = sized_window( height, width, "" );
    select_buffer(buf, win, SEL_CENTER, "", NULL, NULL);
    top_line = o_top_line;
    window_offset = o_window_offset;
    delete_buffer(buf);                         /* release local buffer */
}

/*eof*/

