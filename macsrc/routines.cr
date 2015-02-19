/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: routines.cr,v 1.8 2014/10/27 23:28:26 ayoung Exp $
 * Routine search.
 *
 *
 */

#include "grief.h"
#include "mode.h"

static void     routines_list(void);
extern void     routine_keys(void);
static void     routines_detailed(void);


/*
 *  If TRUE get previous line to function definition. For C files
 *  this usually means get the type info as well.
 */
static int      routines_prev_line = FALSE;


/*
 *  Variable needed to keep track of Ctrl-G nesting.
 */
static int      rtn_nested = FALSE;


void
routines(void)
{
    string macro_name, ext;

    ext = _mode_pkg_get();
    macro_name = ext + "_routines";             /* handler */
    if (inq_macro(macro_name) > 0) {
        execute_macro(macro_name);
    } else {
        routines_list();                        /* list available */
    }
}


/*
 *  routines_list ---
 *      If no routines available for current file, then generate a
 *      list of all routines macros and let him have a go at these.
 */
void
routines_list(void)
{
    list macs = macro_list();
    list rtn_list;
    int entry = 0;
    int cnt = 0;

    while (1) {
        entry = re_search(NULL, "_routines$", macs, entry);
        if (entry < 0)
            break;

        if (macs[entry] != "default_routines") {
            rtn_list[cnt] = macs[entry];
            cnt++;
        }
        entry++;
    }

    entry = select_list( "Routine macros",
                "Select function", 1, rtn_list, TRUE, NULL );
    if (entry < 0)
        return;

    execute_macro(rtn_list[entry - 1]);
}


/*
 *  s_routines, asm_routines ---
 *      Routines for Intel assembler files
 */
void
asm_routines()
{
    routines_search( "<*PROC[ \t]", SF_IGNORE_CASE,
        "Assembler Subroutines", "asm_routines_trim" );
}


void
s_routines()
{
    asm_routines();
}


string
asm_routines_trim(string name)
{
    if (substr(name, 1, 1) == "_")
        return substr(name, 2);
    return name;
}


/*
 *  ps_routines ---
 *      Routines for PostScript files.
 */
void
ps_routines()
{
    routines_search( "</*\\{", 0,
        "PostScript Definitions", "ps_routines_trim" );
}


string
ps_routines_trim(string name)
{
    return name;
}


/*
 *  f_routines ---
 *      Routines for Fortran files.
 */
void
f_routines()
{
    routines_search( "<[\t ]@{subroutine}|{function}\\c*+$", SF_IGNORE_CASE,
        "Routines", "f_routines_trim" );
}


string
f_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, "[#(", name);
    if (spos > 0)
        name = substr(name, 1, spos - 1);
    return trim(name);
}


/*
 *  pas_routines ---
 *      Routines for Pascal files.
 */
void
pas_routines()
{
    routines_search( "<{PROCEDURE}|{FUNCTION}*+$", SF_IGNORE_CASE,
        "Procs & Funcs", "pas_routines_trim" );
}


string
pas_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, "[;/{]", name);
    if (spos > 0) {
        name = substr(name, 1, spos - 1);
    }
    return trim(name);
}


/*
 *  y_routines, y_routines_trim ---
 *      Routines for Yacc source files.
 */
void
y_routines()
{
    routines_search( "<[_a-z0-9]+[ \t]@:", SF_IGNORE_CASE,
        "Yacc Rules", "y_routines_trim" );
}


string
y_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, ":", name);
    if (spos > 0) {
        name = substr(name, 1, spos - 1);
    }
    return trim(name);
}


/*
 *  cr_routines ---
 *      GRIEF/CRISP/BRIEF style routines
 */
void
cr_routines()
{
    routines_search( "<[_a-z0-9]+[ \t]@*([^)\"]@)[^,;]@>",
        SF_IGNORE_CASE, "Functions", "c_routines_trim" );
}


/*
 *  c_routines, cb_routines, cc_routines, cpp_routines,
 *  cxx_routines, C_routines and c_routines_trim ---
 *      C/C++ style routine
 */
void
c_routines()
{
    routines_search( "<[_a-z0-9:~]+[ \t]@*(", SF_IGNORE_CASE,
        "Functions", "c_routines_trim" );
}


void
cb_routines()
{
    c_routines();
}


void
cc_routines()
{
    c_routines();
}


void
cpp_routines()
{
    c_routines();
}


void
cxx_routines()
{
    c_routines();
}


void
C_routines()
{
    c_routines();
}


string
c_routines_trim(string name)
{
    int lines, whitespace, backets;
    string c;
    int loc;

    search_fwd("(");
    inq_position(NULL, loc);
    beginning_of_line();
    name = trim(read(loc));                     // read 'function name' including opening bracket
    next_char(loc);                             // reposition past name

    lines = whitespace = 0;                     // cursors
    backets = 1;                                // 0 when end of parameters

    while (1) {
        c = read(1); next_char();

        if (index(c, "\n") && lines++ > 20) {   // fuse search to lines
            break;                              // ... completion
        }

        c = substr(c, 1, 1);                    // first character, remove trailing newline (if any)

        // terminators
        if (c == ";") {                         // prototype
            return ("");                        // ... ignore

        } else if (c == "{") {                  // function body
            break;                              // ... completion

        } else if (backets <= 0 && c == ":") {  // C++ initialisation
            break;                              // ... completion

        // comments
        } else if (c == "/") {                  // possible comment

            c = read(1), next_char();

            if (substr(c,1,1) == "/") {         // comment

                if (index(c, "\n") == 0) {
                    down();
                    beginning_of_line();        // move down one line
                }

            } else if (substr(c,1,1) == "*") {  // /* comment */

                while (1) {                     // find end of comment
                    if ((loc = search_fwd("[*]")) ==0 ||
                                substr(read(2), 1, 2) == "*/") {
                        break;
                    }
                    next_char();                // eat '*'
                }

                if (loc) {
                    next_char();                // eat '/'
                }
            }

        // text
        } else if (index("\n\t ", c)) {
            ++whitespace;                       // whitespace

        } else {                                // append to routine

            if (c == "(") {                     // maintain backet nesting
                ++backets;
            } else if (c == ")") {
                --backets;
            }

            if (whitespace == 0 || index("(),", c)) {
                name += c;
            } else {
                name += " " + c;
            }

            whitespace = 0;
        }
    }

    if (strlen(name) > 60) {
        name = substr(name,1,54) + " ...";
    }

    return name;
}


/*
 *  pm_routines, pl_routines ---
 *      Perl style routines
 */
void
pl_routines()
{
    routines_search( "<sub [_a-zA-Z0-9]+[ \t]*+$", 0,
        "Subroutines", "pl_routines_trim" );
}


void
pm_routines()
{
    pl_routines();
}


string
pl_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, "[#(", name);
    if (spos > 0) {
        name = substr(name, 1, spos - 1);
    }
    return trim(name);
}


void
news_routines()
{
    routines_search( "<Subject:", 0,
        "Subjects", "news_routines_trim" );
}


string
news_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, ":", name);
    if (spos > 0) {
        name = substr(name, spos + 1);
    }
    return trim(name);
}


void
fs_routines()
{
    routines_search(
        "<{BEFORE}|{ON}|{CHOOSE}|{AFTER}|{FIELD}|{INPUT}|{[a-zA-Z ]+ FUNCTION [a-zA-Z ]@*([^)\"]@)$}",
        0, "Sections", "fs_routines_trim" );
}


string
fs_routines_trim(string name)
{
    return substr(name, 1);
}


void
as_routines()
{
    fs_routines();
}


void
h_routines()
{
    routines_search( "<{typedef}|{enum}|{struct}|{class}|{union}[^;]+$", 0,
        "Structures", "h_routines_trim" );
}


string
h_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, "[;/{]", name);
    if (spos > 0) {
        name = substr(name, 1, spos - 1);
    }

    /* If we have 'typedef struct ...' then remove the typedef part. */
    if (substr(name, 1, 1) == "t" && index(name, "struct")) {
        name = substr(name, 9);
    }
    spos = re_search(NULL, "[ \t]", name);

    if (spos > 0) {
        name = trim(substr(name, spos + 1)) + " : " + substr(name, 1, spos - 1);
    }

    return trim(name);
}


void
hlp_routines()
{
    routines_search( "<\\> ", 0,
        "Sections", "hlp_routines_trim" );
}


string
hlp_routines_trim(string name)
{
    return substr(name, 3);
}


void
m_routines()
{
    routines_search( "<({macro}|{replacement}\\c", 0,
        "Macros", "m_routines_trim" );
}


string
m_routines_trim(string name)
{
    int spos;

    spos = re_search(NULL, "[ \t;]", name);
    if (spos > 0) {
        return substr(name, 1, spos - 1);
    }
    return name;
}


void
mm_routines()
{
    list levels;
    int i;

    for (i = 0; i < 5;) {
        levels[i++] = 0;
    }
    routines_search( "<\\.{TH??}|{H}|{SH}", 0,
        "Sections", "mm_routines_trim" );
}


string
mm_routines_trim(string name)
{
    extern list levels;
    string s, s1;
    int i, j;

                                                /* headings */
    if (substr(name, 1, 2) != ".H" || routines_prev_line) {
        return name;
    }

    s = substr(name, 4);
    j = atoi(s) - 1;
    if (j < 0 || j > 5) {
        return name;
    }
    for (i = j; i < 5;) {
        levels[++i] = 0;
    }
    levels[j] = levels[j] + 1;
    s = substr(s, index(s, " "));
    s1 = "";
    for (i = 0; levels[i]; i++) {
        s1 += levels[i] + ".";
    }
    return s1 + s;
}


/*
 * routines_search ---
 *    Routine to select language sepecific entities from a buffer.
 *    (macro routines_search
 *        sstr  search-string to find matching line.
 *        name  name of things we are looking for.
 *    )
 */
void
routines_search(string sstr, int sflags, string desc, string trim_func)
{
    list    line_no_list;                       /* List of line-numbers so we know where to go to when the user makes a selection */
    int     curbuf,                             /* Current buffer */
            curline,                            /* Current line -- so we can home in on the popup */
            macbuf,                             /* Buffer to put macro names in */
            mac_cnt,                            /* Count of macros encountered so far */
            line, endline,                      /* Temporary to contain line number of of matched macro-name */
            win,                                /* Window to display macros in */
            selection,                          /* Users selection */
            mac_line,                           /* Line to place cursor on to start with */
            width;                              /* Maximum width so far */
    string  name,                               /* Name of currently matched macro */
            msg;

    inq_position(curline);
    curbuf = inq_buffer();
    save_position();
    macbuf = create_buffer(name, NULL, 1);
    top_of_buffer();
    message("Scanning for %s...", desc);

    width = 10;
    mac_cnt = 0;
    mac_line = 0;

    while (re_search(sflags, sstr))
    {
        /* Pattern match */
        inq_position(line);
        name = trim(compress(read()));
        name = execute_macro(trim_func, name);
        if (name == "")
        continue;

        inq_position(endline);

        /* If user wants previous line to function definition -- get that too. */
        if (routines_prev_line) {
            move_abs(line-1, 1);
            if (index(name, "(")) {
                name = substr(name, 1, index(name, "(") - 1) + "()";
            }
            name = ltrim(trim(compress(read()))) + "\t" + name + ";";
            move_abs(endline, 1);
        }

        /* Locate closest function to 'curline' */
        if (line == curline && mac_line == 0) {
            mac_line = mac_cnt + 1;
        } else if (line > curline && mac_line == 0) {
            mac_line = mac_cnt;
        }

        /* Build list entry */
        line_no_list[mac_cnt] = line;
        set_buffer(macbuf);

        if (mac_cnt++) {
            insert("\n");
        }

        insert(name);
        if (strlen(name) > width) {
            width = strlen(name);
        }

        set_buffer(curbuf);
        next_char();

        message("Scanning for routines %s [#%d]...", desc, mac_cnt);
    }
    restore_position();

    /* If no macros found just tell the user and exit. */
    if (mac_cnt == 0) {
        message("No %s found.", desc);
        delete_buffer(macbuf);
        return;
    }

    /* We found some macros -- display them */
    if (++width < 26) {
        width = 26;
    }
    msg = int_to_key(ALT_C) + " - copy to scrap. ";
    win = sized_window(mac_cnt + 1, width, msg);
    message("Use arrow keys to make a selection.");
    selection = select_buffer(macbuf, win, SEL_NORMAL, "routine_keys", NULL,
                    "help_display \"features/routines.hlp\"", mac_line);
    delete_buffer(macbuf);
    message("");
    if (selection < 0) {
        if (routines_prev_line && !rtn_nested) {
            rtn_nested = TRUE;
            routines_search(sstr, sflags, desc, trim_func);
            routines_prev_line = FALSE;
            rtn_nested = FALSE;
        }
        return;
    }
    goto_line( line_no_list[selection-1] );
}


void
routine_keys(void)
{
    assign_to_key("<Ctrl-G>", "::routines_detailed");
}


static void
routines_detailed(void)
{
    routines_prev_line = TRUE;
    sel_esc();
}

/*end*/
