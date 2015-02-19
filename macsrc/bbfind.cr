/* -*- mode: cr; indent-width: 4; -*-
 * $Id: bbfind.cr,v 1.6 2014/10/27 23:28:17 ayoung Exp $
 * Interface to bb find-source tool.
 *
 *
 */

#include "grief.h"

#define F_ALL           0x0001
#define F_NOSTATIC      0x0002
#define F_ALTERNATIVE   0x0004

#if defined(__PROTOTYPES__)
static void             find_source(string function, int flags);
static void             bbfind_display(void);
/*static*/ void         bbfind_list(void);
/*static*/ void         bbfind_log(void);
/*static*/ void         bbfind_next(void);
/*static*/ void         bbfind_prev(void);
#endif

/*static*/ int          bbfind_buf = -1;
/*static*/ int          bbfind_lineno = 1;
static string           bbfind_function;


void
main()
{
    int curbuf;

    curbuf = inq_buffer();
    bbfind_buf = create_buffer( "find-source", NULL, 1 );
    set_buffer(bbfind_buf);
    set_buffer_flags(NULL, NULL, ~BF_CHANGED);
    set_buffer(curbuf);
}


/*  Function:       _crixxx_bbfind

        Configuration interface.

**/
string
_griget_bbfind()
{
    return "";
}


void
_griset_bbfind(string arg)
{
    UNUSED(arg);
}


/*  Function:       bbfind

        BB find-source interface, find-source identifies source file where
        <routine> is defined.

**/
void
bbfind()
{
    string function;
    int idx, flags, param;

    idx = 0;
    param = 0;
    flags = 0;

    while (1) {
        if (!get_parm(idx++, function, "Symbol: "))
            return;

        if (function == "/a")                   /* show all, even dummy */
            flags &= F_ALL;

        else if (function == "/s")              /* exclude static */
            flags |= F_NOSTATIC;

        else if (function == "/x")              /* no alternative */
            flags |= F_ALTERNATIVE;

        else
            break;                              /* assume a 'filename' */
    }

    if (function == "") {
        error( "bbfind: must specify a function name." );
        return;
    }

    find_source(function, flags);
}


void
bbfind_inline()
{
    string function;
    int i;

    save_position();
    re_search( SF_BACKWARDS, "<|{[^_A-Za-z0-9]\\c}" );
    function = trim(read());
    i = re_search( NULL, "[^_A-Za-z0-9]", function );
    if (i > 0)
        function = trim(substr(function, 1, i - 1));
    restore_position();
    if (function == "") {
        beep();
        return;
    }

    find_source(function, 0);
}


static void
find_source(string function, int flags)
{
    int len = strlen(function);
    int is_fortran = (substr(function, len) == "_" ? 1 : 0);
    string cmd;

    sprintf( cmd, "find-source %s%s%s", (flags & F_ALL ? "--all " : ""),
        (flags & F_NOSTATIC ? "--nostatic " : ""), function );
    message( cmd );

    bbfind_buf = perform_command( cmd, "find-source", bbfind_buf );
    if (inq_lines(bbfind_buf) == 0)
        if ((flags & F_ALTERNATIVE) == 0 && len)
        {                                       /* try alternative */
            flags |= F_ALTERNATIVE;

            if (is_fortran)
                function = substr(function, 1, len-1);
            else function += "_";

            find_source( function, flags );
            return;
        }

    assign_to_key("<Ctrl-N>", "bbfind_next");   /* next match */
    assign_to_key("<Ctrl-P>", "bbfind_prev");   /* prev match */
    assign_to_key("<Ctrl-B>", "bbfind_list");   /* show results */

    if (is_fortran)
        function = substr(function, 1, len-1);
    bbfind_function = function;

    bbfind_list();
}


static void
bbfind_display(void)
{
    string result, filename;
    int lines, curbuf;

    curbuf = inq_buffer();
    set_buffer(bbfind_buf);
    lines = inq_lines();

    if (bbfind_lineno < 1 || bbfind_lineno > lines)
    {
        set_buffer(curbuf);
        bbfind_lineno = 1;
        message( "No more matchs." );
    }
    else
    {
        int comma, delimit;

        move_abs(bbfind_lineno, 1);
        result = rtrim(read());
        set_buffer(curbuf);

        comma = rindex( result, "," );
        delimit = rindex( result, "/" );        /* xxx - unix assumed */

        if (comma && delimit && substr(result, comma) == ",v")
        {
            string wdir = "./.bbsource";

            if (mkdir( wdir ) == 0)
            {                                   /* working directory */
                int readme;

                if ((readme = create_buffer( "README" )) >= 0)
                {
                    set_buffer( readme );
                    insert( "bb tools temporary working directory.\n" );
                    write_buffer( wdir + "/README" );
                    set_buffer( curbuf );
                    delete_buffer( readme );
                }
            }

            filename = wdir +
                substr( result, delimit, comma - delimit );

            if (inq_buffer( filename ) <= 0 && cd( wdir ))
            {                                   /* checkout working image */
                string cmd;

                sprintf( cmd, "co -q %s", result );
                shell( cmd, 0 );
                cd( ".." );
            }

            if (edit_file( filename ) >= 0)
            {                                   /* edit and select symbol */
                top_of_buffer();
                re_search( SF_NOT_REGEXP, bbfind_function );
            }
        }
        else
        {
            error( "bbfind: unknown format %s", result );
        }

        message( "<Ctrl-P> prev, <Ctrl-N> next, <Ctrl-B> list." );
    }
}


/*  Function:       find_list

        list the find results.

**/
/*static*/ void
bbfind_list(void)
{
    int curbuf, win, lines;

    curbuf = inq_buffer();
    set_buffer(bbfind_buf);                     /* previous results */
    top_of_buffer();
    lines = inq_lines();
    set_buffer(curbuf);
    if (lines <= 0)                             /* empty */
    {
        error( "bbfind: no matching found." );
        return;
    }

    keyboard_flush();
    bbfind_lineno = 1;
    win = sized_window( lines+1, -1, "<Enter> select, <Esc> quit, <Alt-L> changelog." );
    bbfind_lineno =
        select_buffer( bbfind_buf, win, NULL,
                assign_to_key( "<Alt-L>", "bbfind_log" ),
                NULL,
                "help_display \"features/bbfind.hlp\" \"Help on bbfind\"",
                bbfind_lineno, FALSE );
    set_buffer(curbuf);

    if (bbfind_lineno < 1)                      /* Esc ? */
        return;

    bbfind_display();
}


/*static*/ void
bbfind_log(void)
{
    string result;
    int comma, delimit;

    result = rtrim(read());
    comma = rindex(result, ",");
    delimit = rindex(result, "/");              /* xxx - unix assumed */

    if (comma && substr(result, comma) == ",v")
    {
        string cwd, cmd;
        int cols, buf, win;

        getwd( NULL, cwd );
        inq_screen_size( NULL, cols );
//      sprintf( cmd, "rlog %s", result );
        sprintf( cmd, "rcs2log -i 4 -l %d %s", (cols < 60 ? 60 : cols - 8), result );
        if (delimit)
            cd( substr(result, 1, delimit) );
        buf = perform_command( cmd, "changelog" );
        if (delimit)
            cd( cwd );

        if (buf >= 0)
        {
            win = sized_window( inq_lines(buf)+1, -1, "<Esc> to quit." );
            select_buffer( buf, win );
        }
        delete_buffer( buf );
    }
}


/*static*/ void
bbfind_next(void)
{
    ++bbfind_lineno;
    bbfind_display();
}


/*static*/ void
bbfind_prev(void)
{
    --bbfind_lineno;
    bbfind_display();
}

/*end*/
