/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: feature.cr,v 1.30 2015/02/19 00:17:40 ayoung Exp $
 * Features menu.
 *
 *
 */

#include "grief.h"


/* Following list defines the entries to appear in the features
 * menu. First item is the window title, and the remaining items
 * are grouped in pairs -- the label and the macro/keystroke to call
 */
#if defined(OS2) || defined(MSDOS)
#define LOCALSHELL      "cmd"
#else
#define LOCALSHELL      "sh"
#endif

static list             feature_list = {
                "Features", "",                 /*title, help topic*/
    "ASCII/UNICODE Chart",          "ascii",
    "Ansi mode toggle",             "ansi",
    "Auto indent",                  "autoindent",
    "Auto wrap",                    "autowrap",
    "Buffer List",                  "buffer_list 1",
    "Buffer information",           "bufinfo",
    "Buffer search",                "bs",
    "C Hierarchy chart",            "chier",
    "Calculator (infix)",           "calc",
    "Center text",                  "center",
    "Change Log",                   "change",
    "Color labels",                 "colorlabels",
    "Color mappings",               "setcolor",
    "Color scheme",                 "colorscheme",
    "Current file name",            "display_file_name",
    "Define abbreviation",          "abbrev",
    "Delete blank lines",           "delete_blank_lines",
    "Delete trailing space",        "delete_trailing_spaces",
    "Detab buffer",                 "detab_buffer",
    "Detab region",                 "detab_region",
    "Display Information",          "dispinfo",
    "Document options",             "options",
    "Edit file again",              "edit_again",
    "Error number (errno) chart",   "errnos",
    "Execute macro/command",        "execute_macro",
    "Explain an macro",             "explain",
    "Extra features",               "extra",
    "File find",                    "ff",
    "Find function",                "tag",
    "Find manual entry",            "apropos",
    "Format Region",                "format_block",
    "GREP",                         "grep",
    "GRIEF Demonstration",          "demo",
    "Game of Invaders",             "invaders",
    "Game of Mine-Sweeper",         "sweeper",
    "Game of Snake",                "snake",
    "Game of Tetris",               "tetris",
    "HEX mode editor",              "hexmode",
    "HP Calculator (RPN)",          "hpcalc",
    "Help",                         "help",
    "Hierarchy charts",             "hier",
    "Indent region",                "shiftr",
    "Insert date",                  "insert_date",
    "Interactive Search",           "isearch",
    "Join next line",               "<Ctrl-A><Ctrl-J>",
    "Keyboard Summary",             "kbd_summary",
    "Keystroke Library",            "keylib",
    "Lint",                         "lint",
    "List functions/sections",      "routines",
    "Literal display mode",         "literal",
    "Lower case region",            "block_lower_case",
    "Mail",                         "mail",
    "Make buffer writeable",        "make_writeable",
    "Make",                         "make",
    "Manual page",                  "man",
    "Match brackets",               "find_matching_brace",
    "Maximize the console",         "fullscreen",
    "Norton Commander Style GUI",   "nc",
    "Outdent region",               "shiftl",
    "Pipe region",                  "<Ctrl-A><Ctrl-P>",
    "Print buffer",                 "print_buffer",
    "Save window layout",           "save_state",
    "Scroll lock window",           "scroll",
    "Set right hand margin",        "margin",
    "Shell buffer",                 LOCALSHELL,
    "Signal number chart",          "signos",
    "Sort",                         "sort",
    "Spell checker",                "spell",
    "Sub-shell",                    "shell",
    "Sum column of numbers",        "sum",
    "Syntax mode",                  "mode",
    "System information",           "sysinfo",
    "Texinfo",                      "texinfo",
    "Text search",                  "ts",
    "Tower of Hanoi",               "hanoi",
    "Unix spell",                   "uspell",
    "Upper case region",            "block_upper_case",
    "Use tabs or spaces",           "use_tab_char",
    "Wildcard file erase",          "wildcard_erase",
    "Word count",                   "wc",
    "Write all buffers",            "write_buffers",
    "Write buffer as ..",           "write_buffer_as",
    "Zoom/unzoom window",           "zoom"
    };

static string           printkeys(string keys);


void
feature()
{
    message("Generating features menu...");
    select_feature(feature_list, 30, FALSE);
}


void
feature_help()
{
    message("Generating features menu...");
    select_feature(feature_list, 30, TRUE);
}


/*  select_feature ---
 *      Generate and process a 'features' menu.
 *
 *      First line contains of a 'title', plus an optional 'help topic'
 *      used for the window.  If 'help topic' is omitted, then standard
 *      cshelp(features) shall be used based on the line description.
 */
int
select_feature(list lst, int width, ~int dohelp)
{
    extern int top_line;
    string help, nl, s;
    declare t;
    int curbuf, win, buf;
    int llen;
    int i, j, ret;

    curbuf = inq_buffer();

#define FIELDOFF        2                       /* start of fields */
#define FIELDLEN        2                       /* sizeof() field line */

    buf = create_buffer(lst[0], NULL, TRUE);    /* first entry title */
    set_buffer(buf);
    llen = length_of_list(lst);

    /*  First insert the left hand side of the buffer, because we don't know
     *  how wide to pad the screen until we've scanned all entries.
     */
    for (i = FIELDOFF; i < llen; i += FIELDLEN) {
        s = lst[i+0];                           /* description */
        t = lst[i+1];                           /* command */
        j = strlen(s) + 6;

        /* Check to see whats on the right of the line */
        if (is_string(t)) {
            /* We may have a pure macro name or a key binding so look for a key
             * binding first because that's morehelpful for the user.
             */
            s = inq_assignment(t, 1);
            if (s != "nothing") {
                j += strlen(printkeys(s));
            } else {
                j += strlen(t);
            }

        } else {
            /* indicate there is another popup */
            j += 2;
        }

        if (j > width) {
            width = j;                          /* track longest line length */
        }
    }

    /* Handle window help being longer ... */
    help = (dohelp ? "" : int_to_key(ALT_H) + " help, ") +
                "<Enter>, " + int_to_key(ALT_S) + " search";

    if (strlen(help) + 4 > width) {
        width = strlen(help) + 4;               /* +2 borders */
    }

    /* Now we know the width of the longest line, we can go and insert the
     * data into the buffer.
     */
    for (i = FIELDOFF; i < llen; i += FIELDLEN) {
        s = lst[i+0];                           /* description */
        t = lst[i+1];                           /* command */
        insert(nl);
        insert(s);

        /* Check to see what's on the right of the line. */
        if (is_string(t)) {
            /* We may have a pure macro name or a key binding so look for a key
             * binding first because that's more helpful for the user.
             */
            j = width - strlen(s);
            s = inq_assignment(t, 1);
            if (s != "nothing") {
                t = printkeys(s);
            }
            insert(" ", j - strlen(t));         /* right just */
            insert(t);

        } else {
            /* indicate there is another popup */
            insert(" ", width - strlen(s) - 2);
            insert("=>");
        }
        nl = "\n";
    }

    ++top_line;
    top_of_buffer();
    win = sized_window(inq_lines() + 1, width + 1, help);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_SHOW, win);
    set_buffer(curbuf);
    ret = select_buffer(buf, win, SEL_NORMAL, "feature_menukeys", NULL, lst[1]);
    --top_line;

    delete_buffer(buf);                         /* destroy local buffer */

    /* Erase any info messages at bottom of screen. */
    refresh();
    message("");

    /* If selection aborted, then return without executing the action. */
    if (ret < 0) {
        return -1;
    }

    /* If macro started with a '<' then it must be a literal keystroke so we
        need to force it into the keyboard input buffer. */
    if (! dohelp) {
        s = lst[(ret * FIELDLEN)+1];            /* command */
    } else {
        s = lst[(ret * FIELDLEN)+0];            /* description text */
    }

    if (! dohelp && substr(s, 1, 1) == "<") {
        force_input(s);                         /* force into keyboard buffer */

    } else if (dohelp) {
        cshelp("features", s);                  /* otherwise help mode */

    } else {
        execute_macro(s);                       /* exec mode */
    }
    return 0;
}


static string
printkeys(string keys)
{
    string key, t;
    int e;

    while ((e = index(keys, ">")) > 0) {
        key = substr(keys, 1, e);
        keys = substr(keys, e + 1);

        if (key == "<-also>") {                 /* or */
            t += ", ";
        } else {
            t += key;
        }
    }
    return t;
}


/*
 *  feature_menukeys ---
 *      Additional select buffer keys for help, help_primitives
 *      and help_section menus, allowing arrows to navigate.
 */
void
feature_menukeys()
{
    assign_to_key("<Backspace>",    "sel_esc");
    assign_to_key("<Space>",        "sel_esc");
    assign_to_key("<Left>",         "sel_esc");
    assign_to_key("<Right>",        "sel_list");
}

/*eof*/
