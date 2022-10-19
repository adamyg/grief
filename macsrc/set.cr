/* $Id: set.cr,v 1.22 2022/08/10 15:44:58 cvsuser Exp $
 * Command level environment variables.
 *
 *
 */

#include "grief.h"
#include "wp.h"

#if defined(DEBUG)
#define  TRACELOG(x)    dprintf x ;
#else
#define  TRACELOG(x)
#endif

#define  SET_TITLE      "set database"

#if defined(MSDOS)
#define  SET_FILE       "_grset"
#else
#define  SET_FILE       ".grset"
#endif

#if defined(__PROTOTYPES__)
static void             sf(string flag, int val);
static void             setall(void);
static void             setbufferflags(void);
static void             setdisplayflags(void);
static void             settermflags(void);
static void             settermchars(void);
static void             setfilemagic(void);

static void             _set_test(~string);
static void             _set_view(void);
static void             _set_update(string var, string value);
static string           _set_file(void);
static int              _set_open(~int, ~int);
#endif

#define BF1(__x)        (__x)
#define BF2(__x)        (100 + __x)
#define BF3(__x)        (200 + __x)
#define BF4(__x)        (300 + __x)

static list             std_variables = {
    "ansi",                 "",
    "autoindent",           "",
    "autosave",             "",
    "autowrap",             0,
    "autowrite",            "",                 // automaticly write the buffer if modified
    "paste",                "::sf \"paste_mode\"",
    "autowrap",             0,
    "backup",               "",
    "blinkmatch",           0,                  // not supported
    "block",                0,                  // not supported
    "bufhidden",            "::sf \"hidden\"",  // hidden from view
    "color",                BF_SYNTAX,          // buffer colorization
    "colormatching",        BF_SYNTAX_MATCH,    // hilite matching brackets; "color" also required.
//  "diff",                 BF_MOD_LINES,
    "eof",                  "",                 // <EOF> marker
    "justify",              "::set_justify",    // format, justify right margin
    "ragged",               "::set_ragged",     // format, unjustified/ragged right margin
//  "tabs",
    "howmatch",             0,
    "ignorecase",           0,
//  "linecolor",            BF_LINE_XXX,
//  "linecolor2",           0,
    "lmargin",              "::set_lmargin",
    "magic",                0,
    "man",                  "",
    "mmap",                 0,
    "modified",             "",
    "number",               "::sf \"line_numbers\"",
    "oldlines",             "::sf \"line_oldnumbers\"",
    "readonly",             "",
    "rmargin",              "::set_rmargin",
    "ruler",                "",
    "spell",                "::sf \"spell\"",
    "shiftwidth",           0,                  // indent
    "showall",              0,
    "showmatch",            0,
    "showmode",             0,
    "syntax",               "::set_syntax",     // syntax=on|off|mode
    "tabstop",              0,                  // tabs
    "tags",                 0,
    "undo",                 -BF_NO_UNDO,
    "volatile",             BF_VOLATILE,
    "vtab",                 "::sf \"hiwhitespace\"",
    "whitespace",           0,
    "wrapscan",             0,

#if (XXX)
    "autochdir",                                // automaticly change to the directory of the active buffer
    autoread                                    // automaticly re-read buffer if the underlying file changes
    autowrite                                   // automaticly write the buffer if modified
    autowriteall                                // global, write-all buffers
//  background
//  backspace
    backup                                      // backup buffer on modification
    bufhidden                                   // hidden shall be hidden from view
    expandtab

    "term"                                      // name of the terminal
    "termbidi,tbidi",                           // terminal takes care of bi-directionality
    "termencoding,tenc",                        // character encoding used by the terminal

    "numberwidth",                              // width of 'number' column
    "textwidth",                                // right margin
    "cindent",                                  // indentation mode=C
    "colorcolumn",                              //

    "timeout,to"                                // time out on mappings and key codes
    "timeoutlen,tm"                             // time out time in milliseconds
    "title"                                     // let Vim set the title of the window
    "titlelen"                                  // percentage of "columns" used for window title
    "titleold"                                  // old title, restored when exiting
    "titlestring"                               // string to use for the Vim window title
    "ttimeout"                                  // time out on mappings
    "ttimeoutlen,ttm"                           // time out time for key codes in milliseconds
    "ttybuiltin,tbi"                            // use built-in termcap before external termcap
    "ttyfast,tf"                                // indicates a fast terminal connection
    "ttymouse,ttym"                             // type of mouse codes generated
    "ttyscroll,tsl"                             // maximum number of lines for a scroll
    "ttytype,tty"                               // alias for "term"

    "cursorcolumn,cuc",                         // highlight the screen column of the cursor
    "cursorline,cul",                           // highlight the screen line of the cursor

    "softtabstop,sts",                          // number of spaces that <Tab> uses while editing
    "shiftwidth,sw"                             // number of spaces to use for (auto)indent step
    "tabstop,ts",                               // number of spaces that <Tab> in file uses

    "ignorecase,ic",                            // ignore case in search patterns
    "smartcase,scs",                            // no ignore case when pattern has uppercase
    "wildignorecase,wic",                       // ignore case when completing file names
#endif
    };


//  Function:           set
//      The set command enables/diables the specificed option, for the compatible with
//      the equivalent options within the 'vi' editor.
//
//      Most of the options are boolean where are either set (on) or not-set (off).
//      Theses the format as follows:
//
//          set readonly
//          set noreadonly
//
//      Other options which take a value are specified by using an equal (=) followed
//      by the value, as follows:
//
//          set tabs=9
//
//      The following options are available.
//
//          lmargin -   Left margin.
//          rmargin -   Right margin.
//          justify -
//          ragged  -
//
void
set(~ string, ~string)
{
    string bufname, var, val;
    int no, i;

    if (! get_parm(0, var, "option: ")) {
        return;
    }

    if ("all" == var) {
        setall();
        return;
    } else if (var == "buffer-flags"  || var == "bufferflags") {
        setbufferflags();
        return;
    } else if (var == "display-flags" || var == "displayflags") {
        setdisplayflags();
        return;
    } else if (var == "term-flags"    || var == "termflags") {
        settermflags();
        return;
    } else if (var == "term-chars"    || var == "termchars") {
        settermchars();
        return;
    } else if (var == "file-magic"    || var == "filemagic") {
        setfilemagic();
        return;
    }

    if ((i = index(var, '=')) > 0) {            /* var=[value] */
        val = substr(var, i + 1);
        var = substr(var, 1, i - 1);
    } else {
        get_parm(1, val);
    }

    if (substr(var, 1, 2) == "no") {            /* strip off 'no' specification */
        var = substr(var, 3);
        no = TRUE;
    }

    inq_names(NULL, NULL, bufname);
    if (var != "") {
        if ((i = re_search(NULL, "<" + var + ">", std_variables)) >= 0) {

            if (no) {
                val = "0";                      /* no --- assumed zero value */
            } else {
                if ("" == val || "y" == val || "yes" == val || "on" == val) {
                    val = "1";
                } else if (atoi(val) < 1) {
                    val = "0";
                }
            }

            declare arg = std_variables[i + 1];
            if (is_string(arg)) {
                //
                //  macro interface
                //
                if (strlen(arg)) {              // e.g. "::sf <flag-name>"
                    execute_macro(arg + " " + val);
                } else {
                    sf(var, atoi(val));
                }
                message("set(%s): %s=%s", bufname, var, val);

            } else if (is_integer(arg)) {
                //
                //  toggle buffer flags
                //
#if (XXX)
                val = lower(val);
                if (atoi(val) >= 1) {
                    if (arg > 0) {
                        message("set-flag1(%s-%d): %s=%s", bufname, arg, var, val);
                    } else {
                        message("set-flag2(%s-%d): %s=%s", bufname, arg, var, val);
                    }
                } else {
                    if (arg > 0) {
                        message("clr-flag1(%s-%d): %s=%s", bufname, arg, var, val);
                    } else {
                        message("clr-flag2(%s-%d): %s=%s", bufname, arg, var, val);
                    }
                }
#endif
            }

        } else {                                // unknown
            message("set(%s): unknown option '%s'", bufname, var);
        }
    }
}


static void
setall(void)
{
    //TODO
}


static void
sf(string flag, int val)
{
    if (val) {
        set_buffer_flags(NULL, flag);
    } else {
        set_buffer_flags(NULL, NULL, flag);
    }
}


static void
setbufferflags(void)
{
    int curbuf = inq_buffer(), buf, win;

    save_position();
    if ((buf = create_buffer("buffer-flags", NULL, 1)) >= 0) {
        string spec, flag;
        list flags;

        set_buffer(buf);
        inq_buffer_flags(curbuf, -1, spec);
        flags = split(spec, ",");
        while (list_each(flags, flag) >= 0) {
            insertf("%s\n", flag);
        }
        sort_buffer(NULL, NULL, 1, inq_lines()-1);
        win = sized_window(inq_lines() + 1, (inq_line_length() * 2) + 1, "<Esc> exit");
        select_buffer(buf, win, NULL, NULL);
        delete_buffer(buf);
    }
    restore_position(2);
}


static void
setdisplayflags(void)
{
    int buf, win;

    save_position();
    if ((buf = create_buffer("display-flags", NULL, 1)) >= 0) {
        string spec, flag;
        list flags;

        set_buffer(buf);
        inq_display_mode(NULL, spec);
        flags = split(spec, ",");
        while (list_each(flags, flag) >= 0) {
            insertf("%s\n", flag);
        }
        win = sized_window(inq_lines() + 1, (inq_line_length() * 2) + 1, "<Esc> exit");
        if (select_buffer(buf, win, NULL, NULL) >= 0) {
            string line, var, val;
            int eq;

            beginning_of_line();
            line = read();
            if ((eq = index(line, '=')) > 0) {  /* var=value] */
                var = substr(line, 1, eq - 1);
                val = trim(substr(line, eq + 1));

                if ("no" == val) {
                    message("toggled %s to yes", var);
                    display_mode(var, NULL);

                } else if ("yes" == val) {
                    message("toggled %s to no", var);
                    display_mode(NULL, var);

                } else if (get_parm(0, val, "override value: ")) {
                    if (atoi(val) > 0) {
                        message("set %s as %s", var, val);
                        sprintf(line, "%s=%s", var, val);
                        display_mode(line, NULL);

                    } else {
                        message("cleared %s", var);
                        display_mode(NULL, var);
                    }
                }
            }
        }
        delete_buffer(buf);
    }
    restore_position(2);
}


static void
settermflags(void)
{
    int buf, win;

    save_position();
    if ((buf = create_buffer("term-flags", NULL, 1)) >= 0) {
        string flag;
        list flags;

        set_buffer(buf);                        /* FIXME - character map, expose ESC */
        flags = get_term_features();
        while (list_each(flags, flag) >= 0) {
            insertf("%s\n", flag);
        }
        sort_buffer(NULL, NULL, 1, inq_lines()-1);
        win = sized_window(inq_lines() + 1, (inq_line_length() * 2) + 1, "<Esc> exit");
        select_buffer(buf, win, NULL, NULL);
        delete_buffer(buf);
    }
    restore_position(2);
}


static void
settermchars(void)
{
    int buf, win;

    save_position();
    if ((buf = create_buffer("term-chars", NULL, 1)) >= 0) {
        string flag;
        list flags;

        set_buffer(buf);
        flags = get_term_characters();          /* FIXME - character map, expose graphics */
        while (list_each(flags, flag) >= 0) {
            insertf("%s\n", flag);
        }
        sort_buffer(NULL, NULL, 1, inq_lines()-1);
        win = sized_window(inq_lines() + 1, (inq_line_length() * 2) + 1, "<Esc> exit");
        select_buffer(buf, win, NULL, NULL);
        delete_buffer(buf);
    }
    restore_position(2);
}


static void
setfilemagic(void)
{
    int buf, win;

    save_position();
    if ((buf = create_buffer("file-magic", NULL, 1)) >= 0) {
        string flag;
        list flags;

        set_buffer(buf);
        flags = split(inq_file_magic(), ",");
        while (list_each(flags, flag) >= 0) {
            insertf("%s\n", flag);
        }
        win = sized_window(inq_lines() + 1, (inq_line_length() * 2) + 1, "<Esc> exit");
        select_buffer(buf, win, NULL, NULL);
        delete_buffer(buf);
    }
    restore_position(2);
}


//  Function:           set_lmargin
//      Left margin.
//
//  Parameters:
//      val - Optional specification.
//
//  Returns:
//      nothing
//
static void
set_lmargin(string val)
{
    set_margins(NULL, atoi(val));
}


//  Function:           set_rmargin
//      Right margin.
//
//  Parameters:
//      val - Optional specification.
//
//  Returns:
//      nothing
//
static void
set_rmargin(string val)
{
    set_margins(NULL, NULL, atoi(val));
}


//  Function:           set_justify
//      Justification.
//
//  Parameters:
//      val - Optional specification.
//
//  Returns:
//      nothing
//
static void
set_justify(string val)
{
    set_margins(NULL, NULL, NULL, atoi(val) ? JUSTIFIED : RAGGED);
}

static void
set_ragged(string val)
{
    set_margins(NULL, NULL, NULL, atoi(val) ? RAGGED : JUSTIFIED);
}


//  Function:           set_syntax
//      Syntax control.
//
//  Parameters:
//      val - Optional specification.
//
//  Returns:
//      nothing
//
//  Examples:
//      :syntax on          Turn on syntax highlighting.
//      :syntax off         Turn off syntax highlighting.
//      :set syntax=perl    Force syntax highlighting.
//
static void
set_syntax(string val)
{
    switch(val) {
    case "on":
    case "1":
        modeline();
        break;
    case "off":
    case "0":
        mode("none");
        break;
    default:
        mode(val);
        break;
    }
}


//  Function:           setenv
//      The setenv command configures general environment settings.
//
//  Parameters:
//      xxx
//
//  Returns:
//      xxx
//
void
setenv(~string)
{
    string var, val, oval;
    int ismenu, i;

    if (! get_parm(0, var, "variable: ")) {
        return;
    }

    if ((i = index(var, '=')) > 0) {            /* var=[value] */
        val = substr(var, i+1);
        var = substr(var, 1, i-1);
    }

    if (var == "") {
        _set_view();                            /* no specific, load configuration file */
        return;
    }

    oval = inq_env(var, ismenu);

    if (val == "" && ! get_parm(1, val, var + "=", NULL, oval)) {
        return;                                 /* aborted */
    }

    if (ismenu == 0 && oval != val) {
        _set_update(var, val);
    }
}


//  Function:           inq_env
//      string
//      inq_env(~string name, ~int source)
//
//  Description:
//      Lookup 'name' in the enviroment and returns the value of the name.
//      It is similar to the function getenv().
//
//  Returns:
//      Returns a string which is the contents of the named enviroment variable, or the
//      null string if the variable does not exist.
//
static void
_set_test(~string)
{
    inq_env("BCC");                             /* local test interface */
    refresh();
}


string
inq_env(~string, ~int)
{
    string varname, var;                        /* variable setting */
    int oldbuf,                                 /* buffer save/restore */
        setbuf;                                 /* inq_env buffer */

    var = getenv(varname);

    /* Does the SET database exist, if not exit */
    if (! get_parm(0, varname)) {
        return "";
    }
    TRACELOG(("look: %s", varname));
    put_parm(1, 0);                             /* clear menu source flag */

    if (! exist(_set_file())) {
        TRACELOG(("WARNING: %s not found", _set_file()));
        return "";
    }

    setbuf = _set_open(TRUE, 1);                /* retrieve/open system image */
    if (setbuf == -1) {
        TRACELOG(("WARNING: error opening %s", _set_file()));
        return "";
    }

    /* Menu */
    oldbuf = inq_buffer();
    set_buffer(setbuf);
    save_position();
    top_of_buffer();

    if (search_fwd("<#menu " + varname + "\\c", 1, 1, 0) > 0) {
        string line;                            /* read line buffer */
        list set_desc,                          /* set list, for select */
            set_vars;                           /* set list, for select */
        int idx;

        /* Create menu */
        line = trim(read());                    /* current setting */

        beginning_of_line();
        save_position();

        if (search_fwd("\\c<#end " + varname + ">", 1, 1, 0) <= 0) {
            restore_position();
            error("Mismatched #menu....");

        } else {
            restore_position();

            /* Build selection list */
            do {
                if ((idx = index(line, ";")) > 0) {
                    set_desc += trim(substr(line, 1, idx - 1));
                    set_vars += trim(substr(line, idx + 1));
                }
                down();                         /* next line */
            } while (substr(line = trim(read()), 1, 5) != "#end ");

            /* Save selection */
            idx = select_slim_list(varname + "", "", set_desc, 0, NULL, NULL);
            if (idx <= 0) {
                idx = 0;                        /* return previous/first */
            } else {
                idx--;                          /* convert to list index */
            }
            var = set_vars[ idx ];              /* selection */

            /* Update history */
            set_buffer(setbuf);
            top_of_buffer();
            if (search_fwd("<#menu " + varname + "\\c", 1, 1, 0) > 0) {
                TRACELOG(("new: %s", var));
                delete_to_eol();
                if (substr(set_desc[idx], 1, 1) != ">") {
                    insert(" >" + set_desc[ idx ] + "<;");
                } else {
                    insert(" " + set_desc[ idx ] + ";");
                }
                insert(var);
                write_buffer();
            }

            TRACELOG(("%s: %s", varname, var));
            put_parm(1, 1);                     /* set menu source flag */
        }

    } else if (search_fwd("<#define " + varname + " \\c", 1, 1, 0) > 0) {
        var = rtrim(read());
    }
    restore_position();

    /* Restore status */
    set_buffer(oldbuf);
    attach_buffer(oldbuf);
    refresh();

    return (var);
}


/*
 *  _set_view ---
 *      View environment.
 */
static void
_set_view(void)
{
    int setbuf;

    setbuf = _set_open(FALSE);                  /* retrieve existing setbuf */
    if (setbuf != -1) {
        /* Buffer exists, force reload unless a user buffer */
        set_buffer(setbuf);
        if (inq_views() == 0 && inq_system()) {
            delete_buffer(setbuf);              /* not displayed and system */

        } else {
            if (inq_views() == 0) {
                attach_buffer(setbuf);          /* display */
                redraw();
            }
            return;
        }
    }

    setbuf = _set_open(TRUE, 0);                /* open as user buffer */
    if (setbuf != -1) {
        attach_buffer(setbuf);                  /* display */
        redraw();
    }
}


/*
 *  _set_update --
 *      Update a variable value.
 */
static void
_set_update(string var, string value)
{
    int setbuf, orgbuf;

    setbuf = _set_open(FALSE);                  /* retrieve existing setbuf */
    if (setbuf != -1) {
        /* Scan for an existing entry */
        orgbuf = inq_buffer();
        set_buffer(setbuf);
        save_position();
        top_of_buffer();

        if (search_fwd("<#define " + var + " \\c", 1, 1, 0) > 0) {
            /* define entry, either delete or update */
            if (value == "" || value == " ") {
                delete_line();
            } else {
                delete_to_eol();
                insert(value);
            }
        } else {
            /* does not exist, append */
            if (strlen(value)) {
                end_of_buffer();
                insert("\n#define " + var + " " + value);
            }
        }

        if (inq_modified()) {
            write_buffer();
        }
        restore_position();
        set_buffer(orgbuf);
    }
}


/*
 *  _set_file ---
 *      Return the name of set database, following the same rules as 'grinit'.
 *
 */
static string
_set_file(void)
{
    string profiledir;

    profiledir = getenv("GRPROFILE");           /* allow user override */
    if (profiledir == "") {
        profiledir = inq_home();
    }
    if (profiledir == "") {
        return ("/" + SET_FILE);                /* default, root directory */
    }
    return profiledir + "/" + SET_FILE;
}


/*
 *  _set_open ---
 *      Either retrieve the existing setenv buffer and open the system buffer.
 */
static int
_set_open(~int, ~int)
{
    int     orgbuf, sysbuf, thisbuf;
    string  setname,                            /* filename of tested buffer */
            bufname;                            /* buffer name of tested buffer */
    int     create,                             /* create new image (TRUE/FALSE */
            attr;                               /* system file (TRUE/FALSE) */

    get_parm(0, create);
    get_parm(1, attr);                          /* user or system attribute */
    setname = _set_file();

    /* Firstly scan existing buffers */
    sysbuf = -1;                                /* first system buffer located */
    orgbuf = inq_buffer();

    while (1) {
        inq_names(NULL, NULL, bufname);
        thisbuf = inq_buffer();

        if (bufname == SET_FILE) {
            if (inq_system()) {
                if (sysbuf != -1) {             /* hmmm, multiple images */
                    error("WARNING: set/inq_env() more then one DB located");
                }
                sysbuf = thisbuf;
            } else {
                if (sysbuf != -1) {
                    set_buffer(sysbuf);
                    if (0 == inq_views()) {
                        TRACELOG(("removed system (userexists)"))
                        delete_buffer(sysbuf);  /* remove system buffer, user has */
                                                /* opened manually */
                    }
                }
                set_buffer(orgbuf);
                TRACELOG(("_set_open() == %d (user)", thisbuf))
                return (thisbuf);
            }
        }

        set_buffer(next_buffer());
        if (inq_buffer() == orgbuf) {
            break;
        }
    }

    if (sysbuf != -1) {
        TRACELOG(("_set_open() == %d (system)", sysbuf))
        return (sysbuf);                        /* return system buffer */
    }

    /* Create new buffer, if required */
    if (create) {
        thisbuf = create_buffer(SET_TITLE, _set_file(), attr);
        set_buffer(orgbuf);
        TRACELOG(("_set_open() == %d (opened %d)", thisbuf, attr))
        return (thisbuf);
    }

    TRACELOG(("_set_open() == -1 (NOT FOUND)"))
    return (-1);
}

/*
 *  Local Variables: ***
 *  mode: cr ***
 *  tabs: 4 ***
 *  End: ***
 */

