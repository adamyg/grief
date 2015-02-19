/* -*- indent-width: 4; -*- */
/* $Id: command.cr,v 1.12 2014/10/27 23:28:18 ayoung Exp $
 * External command.
 *
 *
 */

#include "grief.h"

/*
 *  fixslash ---
 *      Fixes slashes
 */
string
fixslash(string str)
{
#if defined(OS2) || defined(MSDOS)
    string tmpstr = str;
    int i = 0;

    while ((i = index(tmpstr, "/")) != 0) {
        str = substr(tmpstr, 1, i - 1) + "\\" + substr(tmpstr, i + 1);
        tmpstr = str;
    }
#endif
    return str;
}


/*
 *  tmpdir ---
 *      Returns temporary directory.
 */
string
tmpdir(void)
{
    return inq_tmpdir();
}


string
inq_shell(void)
{
    string sh = getenv("SHELL");

    if (sh == "") {
#if defined(MSDOS) || defined(OS2)
        sh = getenv("COMSPEC");
#endif
        if (sh == "")
#if defined(MSDOS)
            sh = "command.com";
#elif defined(OS)
            sh = "cmd.exe";
#else
        sh = "/bin/sh";
#endif
    }
    return sh;
}


/*
 *  perform_command ---
 *      Execute a unix command and read the output into a buffer.
 *
 *  parameter:
 *      cmd -           Unix command to execute.
 *      header -        Header to display at top of window.
 *      [buf] -         If specified name of existing buffer to apply
 *                      command to otherwise, create a new one.
 *      [sys] -         If specified whether the created buffer is marked
 *                      as a system buffer.
 *
 *  Returns:
 *      allocated buffer is returned.
 */
int
perform_command(string cmd, ~ string header, ~ int, ~ int)
{
    int tmp_buf, curbuf = inq_buffer();
    string file;

    sprintf(file, "%s/gr%06d.out", inq_tmpdir(), getpid());
    shell(cmd, 0, NULL, NULL, fixslash(file));

    if (get_parm(2, tmp_buf) > 0 && tmp_buf >= 0) {
        set_buffer(tmp_buf);
        clear_buffer();
        read_file(file);
        top_of_buffer();
        set_buffer(curbuf);

    } else {
        int sys = 1;

        get_parm(3, sys);
        tmp_buf = create_buffer(header, file, sys);
        set_buffer(curbuf);
    }
    remove(file);
    return (tmp_buf);
}

/*eof*/
