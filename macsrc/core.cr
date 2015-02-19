/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: core.cr,v 1.10 2014/10/27 23:28:19 ayoung Exp $
 * Buffer save if core dumped.
 *
 *
 */

#include "grief.h"

#define FILENAME        "BUFFER"

void
_fatal_error(int signo, string signame)
{
    int curwin = inq_window(), curbuf = inq_buffer();
    int win, buf, fileno, t_buf, idx;
    string prompt, tmp;
    string filename, bufname;

    buf = create_buffer("*** GRIEF Internal Error ***", NULL, 1);
    set_buffer(buf);

    insert("A fatal error has been detected with the software.\n");
    if (signo > 0) {
        insertf("the software generated signal %d %s.\n", signo, signame);
    }
    insert("GRIEF shall attempt to save your modified buffers.\n");
    insert("\n");
    insert("It will  write  the  buffers  away to files called\n");
    insert("BUFFER.1, BUFFER.2, etc.\n");
    insert("\n");
    insert("It will not  overwrite the  original files in case\n");
    insert("the buffers have  been corrupted or it dies during\n");
    insert("the attempted salvage.\n");
    insert("\n");
    insert("You will be prompted to save each file.");

    top_of_buffer();
    win = sized_window(inq_lines(), inq_line_length(), "");
    set_window(win);
    attach_buffer(buf);
    message("**** GRIEF Internal Error - %d (%s)", signo, signame);
    refresh();
    sleep(2);

    /*
     *  Now attempt to save the files
     */
    t_buf = next_buffer(1);
    fileno = 1;

    while (t_buf != buf) {
        set_buffer(t_buf);

        if (!inq_system() && inq_modified()) {
            //
            //  Non-system modified buffer ...
            //
            inq_names(filename, NULL, bufname);
            if (strlen(filename) > 20) {
                filename = bufname;
            }

            if (idx < 5) {                      /* !All */
                sprintf(tmp, "Save %s as %s.%d ? (y/n/a) ", filename, FILENAME, fileno);
                prompt = "x";

                while (0 == (idx = index("NnYyaa", prompt))) {
                    get_parm(NULL, prompt, tmp, 1);
                }
            }

            if (idx >= 3) {                     /* Yes or All */
                sprintf(filename, "%s.%d", FILENAME, fileno);
                write_buffer(filename);
                ++fileno;
            }
        }

        t_buf = next_buffer(1);
    }

    set_window(curwin);
    attach_buffer(curbuf);
    delete_window(win);
    delete_buffer(buf);
    message("");
}

/*eof*/
