/* -*- indent-width: 4; -*- */
/* $Id: compile.cr,v 1.22 2014/11/27 15:54:09 ayoung Exp $
 * Inline make/compile support.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
void                    load();
void                    make(string arg);

static void             cc_buffer_switch(int newbuf);
static void             cc_default_find(int reverse);
static void             cc_user_find(int reverse);
static void             cc_find_error(string regexp, list token_list, int reverse);
static void             cc_display_error(string regexp, list token_list, int reverse);
#endif

static int              cc_ubuf = -1;           /* Buffer containing users errors */
static int              cc_cbuf = -1;
static int              cc_lineno = 1;
static string           cc_filename;

static list             load_list;
static int              load_index;


/*
 *  _griset_load ---
 *      Macro called if user puts a 'load:' entry in the init file. We
 *      remember the argument passed to us and then call load to load up any
 *      private startup macros.
 */
void
_griset_load(string arg)
{
    load_list += arg;
    load(arg);
    execute_macro(arg);
}


/*
 *  _griget_load ---
 *      Macro called on exit to put back the list of startup macros
 *      we were passed on startup.
 */
string
_griget_load()
{
    return load_list[load_index++];
}


/*
 *  load ---
 *      Macro to compile and load a macro in the current buffer.
 */
void
load()
{
    string filename, ext, buf, errfile;
    string template, tmp;
    int i, r, curbuf, mtime1, mtime2;

    curbuf = inq_buffer();

    /*
     *  If we get passed a filename argument, then try and find
     *  the macro the user is referring to.
     */
    if (get_parm(0, filename) > 0) {
        tmp = find_macro(filename);
        if (tmp == "") {
            error("File %s not found.", tmp);
            return;
        }
        filename = tmp;
    } else {
        if (inq_modified()) {
            write_buffer();
        }
        inq_names(filename);
    }

    i = rindex(filename, ".");
    ext = substr(filename, i + 1);
    filename = substr(filename, 1, i - 1);

    /*
     *  Search set database, for user override
     */
    template = trim(inq_env("BC" + upper(ext)));
    if (template != "") {
        sprintf(tmp, template, filename);
        make(tmp);
        return;
    }

    /*
     *  If its a .m file we can load it directly in source code form
     */
    cc_lineno = 1;
    switch (ext) {
    case "cm": case "gm":
        load_macro(filename);
        return;
    case "m":
        template = "grunch -e %s %s.m";
        break;
    case "cr":
        template = "grunch -e %s %s.cr";
        break;
    default:
        error("Dont know how to compile .%s files.", ext);
        return;
    }

    /*
     *  Look to see if we have an up to date compiled file and
     *  load that instead.
     */
    tmp = filename + GREXTENSION;
    if (exist(tmp)) {
        file_pattern(filename + "." + ext);
        find_file(NULL, NULL, mtime1);
        file_pattern(tmp);
        find_file(NULL, NULL, mtime2);
        if (mtime1 < mtime2) {
            load_macro(filename);
            message("Macro successfully loaded.");
            return;
        }
    }

    /*
     *  Build the image
     */
    tmp = inq_tmpdir();
    sprintf(errfile, "%s/grx%05d.err", tmp, getpid());
    errfile = fixslash(errfile);
    if (edit_file(errfile) != -1) {
        int newbuf;

        newbuf = inq_buffer();
        set_buffer(curbuf);
        attach_buffer(curbuf);
        delete_buffer(newbuf);

        sprintf(buf, template, errfile, filename);
        message(buf);

        r = shell(buf, 0);
        if (edit_file(errfile) != -1) {
            top_of_buffer();
            newbuf = inq_buffer();
            i = inq_lines();
            if (r == 0 && i < 1) {
                delete_buffer(newbuf);
                newbuf = -1;
            } else {
                cc_buffer_switch(newbuf);
            }

            /*
             *  Tidy windows & buffers up
             */
            remove(errfile);
            set_buffer(curbuf);
            attach_buffer(curbuf);

            /*
             *  If any errors occured, display first one screen.
             */
            assign_to_key("<Ctrl-N>", "::cc_default_find 0");
            assign_to_key("<Ctrl-P>", "::cc_default_find 1");
            if (newbuf != -1) {
                cc_default_find(0);
            } else {
                load_macro(filename);
                message("Macro successfully loaded.");
            }
        }
    }
}


/*
 *  make ---
 *      Execute a 'make' command and load the resuling compiler output.
 */
void
make(string arg)
{
    int    curwin, curbuf, wait_stat;
    int    win, line, col;
    string tmp;
#if defined(MSDOS) && !defined(WIN32)
    string errfile;
#endif

    cc_lineno = 1;
    curwin = inq_window();
    curbuf = inq_buffer();

#if defined(MSDOS) && !defined(WIN32)
    if (get_parm(0, arg, "Make: ", NULL, "gmake") < 0) {
        return;
    }

    sprintf(errfile, "crx%05d.err", getpid());  /* XXX - assume sh is available */
    wait_stat = shell("sh -c \"" + arg + ">" + errfile + " 2>&1\"\n", 0);

    if (edit_file(errfile) == -1) {
        wait_stat = 0;                          /* no error file, hmmm */
    } else {
        int newbuf, lines;

        newbuf = inq_buffer();                  /* newly created buffer */
        top_of_buffer();
        lines = inq_lines();
        if (wait_stat == 0 && lines < 1) {
            delete_buffer(newbuf);
            wait_stat = 0;
        } else {                                /* new compile buffer */
            cc_buffer_switch(newbuf);
            wait_stat = 1;
        }
    }

    remove(errfile);

    refresh();                                  /* refresh the screen */
    get_parm(NULL, tmp, "Make completed - press <return> to continue: ");

#else    /*!MSDOS*/
    if (get_parm(0, arg, "Make: ", NULL, "make") < 0) {
        return;
    }

    curwin = inq_window();
    curbuf = inq_buffer();

    if (curbuf != cc_cbuf) {
        int slines, scols;

        inq_screen_size(slines, scols);
        win = sized_window(slines/3, scols-8, "");
        set_window(win);
        cc_buffer_switch(-1);
        attach_buffer(cc_cbuf);
    } else {
        win = -1;
    }

#if defined(MSDOS)
    connect(PF_WAIT|PF_ECHO);
    refresh();
    wait_for(10, "*\\>");                       /* wait for the prompt */
    message(arg);
    insert_process(arg + "\n");
    sleep(1);

#else
    connect(PF_WAIT|PF_ECHO, "/bin/sh");
    refresh();
    wait_for(10, "*[:$#%>\\]] ");               /* wait for the prompt */
    tmp = "exec " + arg;
    message(tmp);
    insert_process(tmp + "\n");
#endif

    refresh();
    inq_position(line, col);
    set_process_position(line, col);
    insert_process(tmp + "exit\n" );
    refresh();

    /*
     * Wait for process to die.
     */
    wait(wait_stat);

    /*
     * Let user see end of make before blasting it away.
     */
    get_parm(NULL, arg, "Make completed - <return> to continue: ");

    /*
     * Tidy windows & buffers up
     */
    if (win >= 0) {
        delete_window();
    }
#endif   /*!MSDOS*/

    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);

    /*
     * If any errors occured, display first one screen.
     */
    assign_to_key("<Ctrl-N>", "::cc_default_find 0");
    assign_to_key("<Ctrl-P>", "::cc_default_find 1");
    if (wait_stat) {
        cc_default_find(0);
    } else {
        message("Make completed successfully.");
    }
}


void
gmake(string arg)
{
    make("gmake " + arg);
}


void
dmake(string arg)
{
    make("dmake " + arg);
}


void
xmake(string arg)
{
    make("xmake " + arg);
}


/*
 *  cc_buffer_switch ---
 *      Create a compile buffer if its not already been created,
 *      and clear it if necessary.
 */
static void
cc_buffer_switch(int newbuf)
{
    if (cc_cbuf >= 0) {
        set_buffer(cc_cbuf);
        clear_buffer();
        return;
    }

    if (newbuf == -1) {
        cc_cbuf = create_buffer("Compile-Buffer", NULL, TRUE);
        set_buffer(cc_cbuf);
    } else {
        cc_cbuf = newbuf;
    }
    set_buffer(cc_cbuf);
    set_buffer_flags(NULL, BF_NO_UNDO);
}


/*
 *  Object routines for finding errors.
 */
static list cc_errors = {
    /* "fred.c", line 49: ... */
    { "<\"\\c[^():,\"]+\"",     1, "[0-9]+:",   1 },

    /* fred.c: 49: ... */
    { "<[A-Z]:[\\/][^():]+:",   1, "[0-9]+:",   1 },

    /* fred.c(49): ... */
    { "<[^():]+:",              1, "[0-9]+:",   1 },
    { "<[A-Z]:[\\/][^():]+:",   1, "[0-9]+:",   1 },

    { "<[^(]+(",                1, "[0-9]+):",  2 },
    { "<[A-Z]:[\\/][^(]+(",     1, "[0-9]+):",  2 },

    { "<[^(]+(",                1, "[0-9]+) :", 3 },
    { "<[A-Z]:[\\/][^(]+(",     1, "[0-9]+) :", 3 }
    };


static void
cc_default_find(int reverse)
{
    cc_find_error(
        "<{\"*\", line [0-9]+:}|{*:[0-9]+:[ \t]}|{*([0-9]+)[ ]@:}", cc_errors, reverse);
}


/*
 *  errors ---
 *      Given a buffer containing the output of a compiler (error messages and
 *      warnings), move cursor to next error in file and display error message
 *      on bottom of screen. When called the first time, assumes the current
 *      buffer contains error messages and saves the buffer number for this
 *      one. Subsequent calls to this function go back to that old buffer
 */
void
errors(int lineno = 1)
{
    int oldcc_buf = cc_cbuf;

    if (-1 == cc_ubuf) {
        cc_ubuf = inq_buffer();
        assign_to_key("<Ctrl-N>", "::cc_user_find 0");
        assign_to_key("<Ctrl-P>", "::cc_user_find 1");
    }

    if (lineno >= 1) {
        cc_lineno = lineno;                     // 27/05/10
    } else {
        inq_position(cc_lineno);                // otherwise current
    }

    cc_cbuf = cc_ubuf;
    cc_default_find(0);
    cc_cbuf = oldcc_buf;
}


static void
cc_user_find(int reverse)
{
    if (cc_ubuf >= 0) {
        int oldcc_buf = cc_cbuf;
        int curbuf = inq_buffer();

        /*
         *  Validate current 'error'..
         */
        if (curbuf == cc_ubuf) {                /* User within 'error' buffer resync. */
            inq_position(cc_lineno);
        } else if (set_buffer(cc_ubuf) == -1) { /* User buffer no longer exists. */
            cc_ubuf = -1;
        } else {                                /* User buffer validated, rstore. */
            set_buffer(curbuf);
        }

        /*
         *  select next/prev error
         */
        if (cc_ubuf != -1) {
            cc_cbuf = cc_ubuf;
            cc_default_find( reverse );
            cc_cbuf = oldcc_buf;
        }
    }
}


/*
 *  lint --
 *      This macro expects there to be a makefile entry for lint, so that we
 *      can automatically generate the output via a 'make lint'. The output is
 *      then massaged so that the output from lint looks like the normal C
 *      compiler error messages, which allows the display error macro to then
 *      be used to skip from one error to the next.
 */
void
lint()
{
    string filename, warning;

    /*
     *  Do a make lint
     */
    make("lint");
    set_buffer(cc_cbuf);

    /*
     *  Convert all lines of the form: 28) (31) (45) ... have each '(..)'
     *  start on a separate line so we can then prefix each line
     *  with the file name.
     */
    message("Putting errors in separate lines.");
    top_of_buffer();
    while (re_search(NULL, "^[ \t]+([0-9]+)  \t") > 0) {
        drop_anchor(MK_LINE);
        re_translate(SF_GLOBAL | SF_BLOCK, ")", ")\n");
        raise_anchor();
        delete_line();
    }

    /*
     *  Now take lines of the form: alarm llib-lc(15) :: clock.c(45) and move the
     *  file-name/line-no to the start of the ine, append the lint error occuring
     *  before this block.
     */
    message("Massaging lint summary errors.");

    top_of_buffer();
    while (re_search(NULL, " :: ") > 0) {
        up();
        beginning_of_line();
        warning = trim(read());
        down();
        while (read(1) == " ") {
            re_translate(NULL, "{<*}\t*:: {*$}", "\\1: \\0 - ");
            end_of_line();
            insert(warning);
            next_char();
        }
        down();
        beginning_of_line();
    }

    /*
     *  Delete all leading spaces/tabs
     */
    message("Removing leading white space.");
    top_of_buffer();
    re_translate(SF_GLOBAL, "^[ \t]+", "");

    /*
     *  Now suffix all of the above lines [ (..) ] with the warning message that preceeds them.
     */
    message("Putting error messages on each line.");
    top_of_buffer();
    while (re_search(NULL, "^warning") > 0) {
        /*
         *  Look at following lines for lines beginning with '('
         */
        warning = trim(read());
        down();
        while (read(1) == "(") {
            end_of_line();
            insert(warning);
            beginning_of_line();
            down();
        }
    }

    /*
     *  Ensure all lines with errors/warnings put the filename at the beginning of the line.
     */
    message("Making errors into a normalised format.");
    top_of_buffer();
    while (re_search(NULL, "^==============$") > 0) {
        up();
        filename = trim(compress(read()));
        move_rel(2, 0);
        if (index(filename, " ")) {
            filename = substr(filename, 1, index(filename, " ") - 1);
        }
        save_position();
        drop_anchor(MK_LINE);
        if (re_search(NULL, "^==============$") <= 0) {
            end_of_buffer();
        }
        re_translate(SF_GLOBAL | SF_BLOCK, "<{([0-9]+)}", filename + "\\0:");
        raise_anchor();
        restore_position();
    }

    /*
     *  Trim down some of the error messages.
     */
    message("Tidying errors up.");
    top_of_buffer();
    re_translate(SF_GLOBAL, "( number ) ", "");
    top_of_buffer();
    re_translate(SF_GLOBAL, "( arg {[0-9]} )", "(\\0)");

    /*
     *  Display first error.
     */
    cc_lineno = 1;
    cc_default_find(0);
}


static void
cc_find_error(string regexp, list token_list, int reverse)
{
    cc_display_error(regexp, token_list, reverse);
}


static void
cc_display_error(string regexp, list token_list, int reverse)
{
    int curbuf = inq_buffer();
    string errfile, filename_pat, lineno_pat, line;
    int filename_trail, lineno_trail, lineno;
    int foffset, loffset, flength, llength;
    int idx;

    /* Find next/prev */
    if (cc_cbuf == -1 || set_buffer(cc_cbuf) == -1) {
        cc_cbuf = -1;                           /* user deleted ! */
        return;
    }

    if (reverse && cc_lineno > 2) {
        cc_lineno -= 2;
    }
    goto_line(cc_lineno);
    inq_names(NULL, NULL, errfile);

    if (re_search((reverse ? SF_BACKWARDS : 0), regexp) <= 0) {
        message("No more errors (%s).", errfile);
        set_buffer(curbuf);
        cc_lineno = 1;
        return;
    }
    inq_position(cc_lineno);
    ++cc_lineno;

    beginning_of_line();
    line = trim(compress(read()));

    /*
     *  Scan token_list trying to find an entry which matches the filename.
     */
    idx = 0;
    foffset = -1, loffset = -1;
    while (idx < length_of_list(token_list)) {
        list re_list = token_list[idx];

        filename_pat    = re_list[0];           // 1
        filename_trail  = re_list[1];           // 2
        lineno_pat      = re_list[2];           // 3
        lineno_trail    = re_list[3];           // 4

        foffset = re_search(0, filename_pat, line, NULL, flength);
        if (foffset > 0 &&
                (loffset = re_search(0, lineno_pat, line, NULL, llength)) > 0) {
            break;
        }
        ++idx;
    }
    set_buffer(curbuf);

    /* Extract */
    if (filename_trail >= 0) {
        if (foffset <= 0) {
            error("error: unable to locate file name");
            return;
        }
        cc_filename = substr(line, foffset, flength - filename_trail);
    }
    if (loffset <= 0) {
        error("error: unable to locate line no");
        return;
    }
    lineno = atoi(substr(line, loffset, llength - lineno_trail));
    line = substr(line, loffset + llength);

    /* Display and position */
    if (! exist(cc_filename)) {
        error("%s: %s", cc_filename, ltrim(line));
        return;
    }

    if (-1 != edit_file(cc_filename)) {
        error("%d: %s", lineno, ltrim(line));
        beginning_of_line();
        goto_old_line(lineno);
        search_hilite(strlen(read()));

        if (cc_cbuf == cc_ubuf) {
            assign_to_key("<Ctrl-N>", "::cc_user_find 0");
            assign_to_key("<Ctrl-P>", "::cc_user_find 1");
        } else {
            assign_to_key("<Ctrl-N>", "::cc_default_find 0");
            assign_to_key("<Ctrl-P>", "::cc_default_find 1");
        }
    }
}

/*eof*/
