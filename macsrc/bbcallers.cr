/* -*- mode: cr; indent-width: 4; -*-
 * $Id: bbcallers.cr,v 1.6 2014/10/27 23:28:17 ayoung Exp $
 * Interface to bb find-callers tool.
 *
 *
 */

#include "grief.h"

#define F_ALTERNATIVE   0x0001

/*static*/ int          bbcallers_buf = -1;
/*static*/ int          bbcallers_lineno = 1;
static string           bbcallers_function;

#if defined(__PROTOTYPES__)
static void             find_callers(string function, int flags);
static void             bb_display(void);
static void             bb_list(void);
static void             bb_next(void);
static void             bb_prev(void);
#endif

void
main()
{
    int curbuf;

    curbuf = inq_buffer();
    bbcallers_buf = create_buffer("find-callers", NULL, 1);
    set_buffer(bbcallers_buf);
    set_buffer_flags(NULL, NULL, ~BF_CHANGED);
    set_buffer(curbuf);
}


/*  Function:       _crixxx_bbcallers
        Configuration interface.

**/
string
_griget_bbcallers()
{
    return "";
}


void
_griset_bbcallers(string arg)
{
    UNUSED(arg);
}


/*  Function:       bbcallers
        find-callers generates a report listing routines that call the specified routine.

**/
void
bbcallers(...)
{
    string function;
    int idx, flags, param;

    idx = 0;
    param = 0;
    flags = 0;

    while (1) {
        if (! get_parm(idx++, function, "Symbol: "))
            return;

        if (function == "/x") {                 /* no alternative */
            flags |= F_ALTERNATIVE;

        } else {
            break;                              /* assume a 'filename' */
        }
    }

    if (function == "") {
        error("bbcallers: must specify a function name.");
        return;
    }

    find_callers(function, flags);
}


void
bbcallers_inline(void)
{
    string function;
    int i;

    save_position();
    re_search(SF_BACKWARDS, "<|{[^_A-Za-z0-9]\\c}");
    function = trim(read());
    i = re_search(NULL, "[^_A-Za-z0-9]", function);
    if (i > 0)
        function = trim(substr(function, 1, i - 1));
    restore_position();
    if (function == "") {
        beep();
        return;
    }

    find_callers( function, 0 );
}


static void
find_callers(string function, int flags)
{
    int len = strlen(function);
    int is_fortran = (substr(function, len) == "_" ? 1 : 0);
    string cmd;

    sprintf(cmd, "find-callers -v %s", function);
    message(cmd);

    bbcallers_buf = perform_command(cmd, "find-callers", bbcallers_buf);
    if (inq_lines(bbcallers_buf) == 0) {
        if ((flags & F_ALTERNATIVE) == 0 && len) {
            /* try alternative */
            flags |= F_ALTERNATIVE;

            if (is_fortran)
                function = substr(function, 1, len-1);
            else function += "_";

            find_callers(function, flags);
            return;
        }
    }

    assign_to_key("<Ctrl-N>",   "::bb_next");
    assign_to_key("<Ctrl-P>",   "::bb_prev");
    assign_to_key("<Ctrl-B>",   "::bb_list");

    if (is_fortran)
        function = substr(function, 1, len-1);
    bbcallers_function = function;

    bb_list();
}


static void
bb_display(void)
{
    string result, filename;
    int lines, curbuf;

    curbuf = inq_buffer();
    set_buffer(bbcallers_buf);
    lines = inq_lines();

    if (bbcallers_lineno < 1 || bbcallers_lineno > lines) {
        set_buffer(curbuf);
        bbcallers_lineno = 1;
        message("No more matchs.");

    } else {
        int delimit;
        list parts;

        move_abs(bbcallers_lineno, 1);
        result = trim(read());
        set_buffer(curbuf);

        parts = split( result, "()" );          /* caller (file) */
        delimit = rindex( parts[1], "/" );      /* xxx - unix assumed */

        if (length_of_list(parts) == 2 && delimit) {
            if (exist(parts[1] + ",v")) {
                string wdir = "./.bbsource";

                if (mkdir(wdir) == 0) {         /* working directory */
                    int readme;

                    if ((readme = create_buffer("README")) >= 0) {
                        set_buffer( readme );
                        insert("bb tools temporary working directory.\n");
                        write_buffer(wdir + "/README");
                        set_buffer(curbuf);
                        delete_buffer(readme );
                    }
                }

                filename = wdir + substr(parts[1], delimit);

                if (inq_buffer(filename) <= 0 && cd(wdir)) {
                    /* checkout working image */
                    string cmd;

                    sprintf(cmd, "co -q %s,v", parts[1]);
                    shell(cmd, 0);
                    cd("..");
                }

                if (edit_file(filename) >= 0) { /* edit and select symbol */
                    string caller = parts[0];
                    int len = strlen(caller);
                    int is_fortran = (substr(caller, len) == "_" ? 1 : 0);

                    top_of_buffer();
                    if (is_fortran)
                        caller = substr(caller, 1, len-1);

                    re_search(SF_NOT_REGEXP, caller);
                }
            } else {
                error("Unable to locate %s", parts[1]);
            }
        } else {
            error("Unknown format %s", result);
        }

        message("<Ctrl-P> prev, <Ctrl-N> next, <Ctrl-B> list.");
    }
}


/*  Function:       find_list
        list the find results.

**/
static void
bb_list(void)
{
    int curbuf, win, lines;

    curbuf = inq_buffer();
    set_buffer(bbcallers_buf);                  /* previous results */
    top_of_buffer();
    lines = inq_lines();
    set_buffer(curbuf);
    if (lines <= 0)                             /* empty */
    {
        error("No matching found.");
        return;
    }

    keyboard_flush();
    bbcallers_lineno = 1;
    win = sized_window(lines+1, -1, "<Enter> to select, <Esc> to quit.");
    bbcallers_lineno =
        select_buffer(bbcallers_buf, win, NULL, NULL, NULL,
                "help_display \"features/bbcallers.hlp\" \"Help on bbcallers\"",
                bbcallers_lineno, FALSE);
    set_buffer(curbuf);

    if (bbcallers_lineno < 1)                   /* Esc ? */
        return;

    bb_display();
}


static void
bb_next(void)
{
    ++bbcallers_lineno;
    bb_display();
}


static void
bb_prev(void)
{
    --bbcallers_lineno;
    bb_display();
}

/*end*/
