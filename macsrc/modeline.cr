/* $Id: modeline.cr,v 1.31 2014/10/22 02:34:19 ayoung Exp $
 * modelines a'la vi and Emac's.
 *
 *  This set of macros give grief a modelines feature a'la (the true BSD) vi. You can
 *  embed commands in either the first TOPCHECK or last BOTTOMCHECK lines. These
 *  commands are executed whenever the file is edited, the trigger being the REG_EDIT
 *  registry.
 *
 *  The supported format of the commands are:
 *
 *  o Native:
 *
 *      gr:<command-1>:<command-2>:...:<command-n>:".
 *
 *      It is important to remember the final colon. The modeline can be on the line by
 *      itself or surrounded by comments; the key is to start and end with a single
 *      colon. It is permissible to have multiple modelines, both at the top and the
 *      bottom, up to the number in the LINES_TO_CHECK constant.
 *
 *  o vi style:
 *
 *      vi:<command-1>:<command-2>:...:<command-n>:".
 *
 *  o Emac style:
 *
 *      You can specify the major mode for editing a given file by
 *      the following special text in the first nonblank line of the
 *      file. The modeline can be on the line by itself or surrounded
 *      by comments. For example:
 *
 *          ; -*-Lisp-*
 *
 *      If the first line is the name of a recongnised style, such as 'perl' or 'lisp',
 *      GRIEF uses a editor style approapriate to the mode. The above tells GRIEF to use
 *      Lisp mode. Note how the semicolon is used to make Lisp treat this line as a
 *      comment. Such an explicit specification overrides any defaulting based on the
 *      file name.
 *
 *      Another format of the mode specification following a Emacs style is;
 *
 *          -*- mode: modename; -*-
 *
 *      which allows you to specify local variables as well, using the
 *      following syntax:
 *
 *          -*- mode: modename; var: value; .... -*-
 *
 *      the following local variables are supported:
 *
 *          hard-tabs:          Tab vs Spaces
 *                  Explicity set the hard or soft tab setting. Value should be either
 *                  'yes' or 'no'.
 *
 *          show-tabs:          Hilight tabs
 *                  Explicity set the hard tab display setting. Value should be either
 *                  'yes' or 'no'.
 *
 *          tab-width:          Display tab setting,
 *                  Normally a tab character witin a buffer is displayed as whitespace
 *                  which extends to the next display tab stop position, and the display
 *                  tab stops come at an interval equal to eight spaces. 'tab-width'
 *                  controls the tab width, being a value between 1 and 999 inclusive.
 *                  Note, that how the tab character is displayed in the buffer has
 *                  nothing to do with the definition of the 'indent-width' setting.
 *
 *          indent-tabs-mode:   Tab vs Spaces,
 *                  Normally used both tabs and spaces to indent lines. If preferred,
 *                  all indentation can be made from spaces only by setting the value as
 *                  'nil'.
 *
 *          indent-width:       Indentation setting
 *          c-basic-offset:
 *                  Typing a tab moves the cursor to the next tab stop, padding either
 *                  spaces or tabs, based on the hard-tabs settings, normally at an
 *                  internal of eight. 'tabs' controls the width of the indentation,
 *                  being a value between 1 and 999 inclusive. A value of zero shall
 *                  clear the current indentation setting and use the current tab-width
 *                  setting.
 *
 *          tabs:               Global tabs setting
 *                  Sets both the 'tab-width' and 'indent-width'.
 *
 *          line-numbers:
 *                  Enable/disable line number display in the left column(s).
 *
 *          left-margin-width:
 *                  Specifies the width of the left margin.
 *
 *          right-margin-width:
 *                  Specifies the width of the right margin.
 *
 *      Examples:
 *
 *          -*- mode: C++; tab-width 8; indent-tabs-mode: nil; tabs: 4; -*-
 *
 *      In addition a local varoiables list may be specified near the end of the file,
 *      in the last 25 lines (a page). The declaration list starts with a line
 *      containing the string 'Local Variables:', and ends with a line containing the
 *      string 'End:'. In between come the variables names and values, one set per line.
 *      as 'variable: value'. If the file has both a local varaiables list and a -*-
 *      line, GRIEF processes everything in the -*- line first, and everything in the
 *      local variables list secondly. For example;
 *
 *          ## Local Variables: ***
 *          ## mode: perl ***
 *          ## End: ***
 *
 *      In the above example, each line starts with the prefix ## and each line ends
 *      with the suffix '***'. These are recongised as the prefix and suffix based on
 *      the first line, by finding them located surronding the magic string 'Local
 *      Variables:', then it automaticlly discards them from the other lines of the
 *      declaration. The usual reason for using prefix/suffix markers, is to embed the
 *      declaration into a comment construct as supported by language file of type.
 */

#include "grief.h"
#include "mode.h"

#define DEBUG 0
#define TOPCHECK 5
#define BOTTOMCHECK 25

#if DEBUG
#define Debug(x)        message x ;             /* local debug diag's */
#else
#define Debug(x)
#endif

#if defined(__PROTOTYPES__)
static string           ml_read(void);
static void             ml_gr(string cmdline);
static void             ml_vi(string cmdline, string prefix);
static void             ml_emacs(string cmdline);
#endif

void
main(void)
{
    load_package();                             /* force module load */
    register_macro(REG_EDIT, "modeline");       /* processing edit events */
}


/*
 *  mode ---
 *      Manually set the file 'mode'.
 */
void
mode(~string)
{
    string mode;

    if (get_parm(0, mode, "Mode: ", NULL, NULL) < 0) {
        return;
    }

    mode = trim(mode, "\t\n\r ");               /* remove trailing whitespace */
    if (strlen(mode)) {
        string mac, tmp;

        mode = _mode_alias(mode, mode);         /* map aliases (returns alias if not matched) */
        mac = "_" + mode + "_mode";             /* package interface */

        load_macro("mode", FALSE);              /* only load once */
        if (inq_macro(mac) > 0 &&
                (tmp = execute_macro(mac)) != "") {
            _package_call("_first", tmp);       /* see language.cr */
        } else {
            inq_names(NULL, NULL, tmp);
            message("%s: unknown mode '%s' - ignored", tmp, mode);
        }
    }
}


/*
 *  modeline ---
 *      Automaticlly set the file 'mode', using the embedded
 *      modeline specification(s).
 */
void
modeline(void)
{
    string curline;
    int dflag, i, j;

    /* ignore well-known specials */
    if (inq_system() ||                         /* ignore system */
            (inq_buffer_flags() & BF_BINARY)) { /* and binary buffer */
        return;
    }
    inq_names(NULL, NULL, curline);
    if (GRLOG_FILE == curline) {
        return;
    }

    /* process the first few lines */
    dflag = debug(0, FALSE);                    /* disable debug, result is far 2 verbose */
    save_position();
    top_of_buffer();
    for (i = 0; i < TOPCHECK; ++i) {
        curline = ml_read();

        if (index(curline, "gr:") ||            /* GRIEF native mode */
                    index(curline, "cr:")) {
            ml_gr(curline);

        } else if (index(curline, "vi:")) {     /* vi emulation mode */
            ml_vi(curline, "vi:");

        } else if (index(curline, "vim:")) {    /* vim emulation mode */
            ml_vi(curline, "vim:");

        } else if (index(curline, "-*-")) {     /* emacs style */
            curline = substr(curline, index(curline, "-*-")+3);
            if ((j = index(curline, "-*-")) > 0) {
                curline = substr(curline, 1, j-1);
                curline = compress(curline, 1);
                ml_emacs(curline);
            }
        }
        down();
    }

    /* process the final few lines */
    end_of_buffer();
    beginning_of_line();
    for (i = 0; i < BOTTOMCHECK; ++i) {
        curline = ml_read();

        if (index(curline, "gr:") ||            /* GRIEF native mode */
                    index(curline, "cr:")) {
            ml_gr(curline);

        } else if (index(curline, "vi:")) {     /* vi[m] emulation mode */
            ml_vi(curline, "vi:");

        } else if (index(curline, "vim:")) {    /* vi[m] emulation mode */
            ml_vi(curline, "vim:");
                                                /* emac, variables section */
        } else if ((j = index(curline, "Local Variables:")) > 0) {
            string prefix, postfix;

            prefix = trim(substr(curline, 1, j - 1));
            postfix = trim(substr(curline, j + 17));

            Debug(("LocalVars(%s,%s)", prefix, postfix));

            while (--i > 0 &&                   /* !EOF and !End: marker */
                        re_search(SF_IGNORE_CASE|SF_NOT_REGEXP, "End:", curline) <= 0) {

                if (prefix != "") {             /* handle prefix */
                    if ((j = index(curline, prefix)) > 0 && j < index(curline, ":")) {
                        curline = ltrim(substr(curline, j + strlen(prefix)));
                    } else {
                        continue;               /* .. not prefix, ignore line ?? */
                    }
                }

                if (postfix != "") {            // handle postfix
                    if ((j = rindex(curline, postfix)) > 0 && j > index(curline, ":")) {
                        curline = rtrim(substr(curline, 1, j - 1));
                    }
                }

                Debug(("var(%s%s%s%s%s)",
                    prefix, prefix ? " " : "", curline, postfix ? " " : "", postfix));

                ml_emacs(curline);              // emacs emulation

                down();
                curline = ml_read();            // read line
            }
            break;                              // no additional processing
        }
        up();
    }

    /* shbash */
    if (! inq_symbol("_bvar_package")) {        // local mode
        top_of_buffer();
        curline = ml_read();
        if (substr(curline, 1, 2) == "#!") {
            //
            //  #!/bin/ksh
            //  #!/usr/bin/perl -w
            //
            curline = substr(curline, rindex(curline, "/") + 1);
            if (index(curline, " ")) {          // remove arguments
                curline = substr(curline, 1, index(curline, " "));
            }
            mode(curline);
        }
    }

    restore_position();
    debug(dflag, FALSE);                        // restore debug
}


/*
 *  _completion_Mode ---
 *      Command line completion.
 */
string
_completion_Mode(string word)
{
    list cmds, modes;
    string pat, mode;
    int matchs, idx;
    int ret;

    matchs = -1;
    cmds = command_list();                      /* inquiry command list */
    pat = "<_" + quote_regexp(word) + "[a-zA-Z]@_mode>";
    idx = re_search(NULL, pat, cmds);
    while (idx >= 0) {
        mode = substr(cmds[idx], 2);
        modes[++matchs] = substr(mode, 1, index(mode, "_")-1);
        idx = re_search(NULL, pat, cmds, idx+1);
    }

    if (matchs < 0) {
        beep();
        return "";
    }

    if (0 == matchs) {
        ret = 0;
    } else {
        ret = select_list("Modes", "", 1, modes, SEL_CENTER,
                "help_display \"features/mode.hlp\" \"Help on Modes\"");
        refresh();
    }

    if (ret < 0) {
        return "";
    }
    return modes[ret - 1];
}


/*
 *  ml_read ---
 *      Read a line converting tabs and compressing white-space, allowing the
 *      modeline processing to be a little forgiving.
 */
static string
ml_read(void)
{
    return compress(re_translate(SF_GLOBAL, "\t", " ", read()), 1);
}


/*
 *  ml_gr ---
 *      grief style modelines. Check for the following mode statements:
 *
 *  -*- gr: command: -*-
 */
static void
ml_gr(string cmdline)
{
    list cmdlist;
    int i;

    cmdline = substr(cmdline, index(cmdline, ":gr:"));
    cmdlist = split(cmdline, ":");
    for (i = 1; i < length_of_list(cmdlist) - 1; i++) {
        execute_macro(cmdlist[i]);
    }
}


/*
 *  ml_vi ---
 *      vi's style modelines. Check for the following mode statements:
 *
 *  -*- vi: set var=value: -*-

    Modelines are a fancy but dangerous vi feature. Therefore, they are usually turned off by
    default in every self-respecting vi version and may have only partial support or no support
    at all.

<GRIEF DOCUMENTATION>
    Some clones intentionally only support selected commands in modelines to avoid the security
    problems.

    Modelines are lines in text files which are specially interpreted by vi when such a text file
    is opened. When the modeline (ml) (in some version of vi also called modelines) option is
    turned on (e.g. in the users .exrc file), vi scans the first and last five lines of each
    opened file for text of the form

        unrelated text vi:command: more unrelated text

            or

        unrelated text ex:command: more unrelated text

    Each command from such lines is taken and executed as it would have been typed by the user.
    Any text in front of the modeline-marker (vi: or ex:) or behind the closing : is ignored for
    the modeline interpretation. This can be used to place modelines in comments if they are used
    in some programming source code.

    Here is an example Java source code file. It contains a modeline on the second and third line,
    in a Java comment:

    //
    // vi:set sw=4 ai:
    // vi:set showmatch:
    //
    package gnu.freesoftware;
    public class Interpreter {
        public Interpreter() ...
            ...

    When modelines are turned on, and this file is opened, shiftwidth (sw) is set to 4,
    autoindent (ai) is turned on, and the showmatch (sm) option is turned on, too. There is no
    particular reason why two set commands on two modelines are used other than to demonstrate
    that all modeline commands found in the first and last five lines are executed, and not just
    the first.

**/
static void
ml_vi(string cmdline, string prefix)
{
    int idx, set = 0;
    list cmdlist;

    idx = index(cmdline, prefix) + strlen(prefix);
    cmdline = substr(cmdline, idx);
    if ((idx = index(cmdline, ":")) > 0) {
        cmdline = substr(cmdline, 0, idx - 1);
        cmdlist = split(cmdline, " ");
        for (idx = 0; idx < length_of_list(cmdlist); ++idx) {
            if (cmdlist[idx] == "set") {
                set = 1;
            } else if (set) {
                set(cmdlist[idx]);
            }
        }
    }
}


/*
 *  ml_emacs ---
 *    emac's style modelines. Check for the following mode statements:
 *
 *    -*- NAME -*-              set mode to NAME
 *    -*- mode: NAME -*-        set mode to NAME
 *    -*- evalfile: FILE -*-    evaluate file FILE
 *    -*- eval: expression -*-  evaluate expression
 *    -*- VAR: VALUE -*-        set VAR = VALUE
 *
 *  these statements may be combined, seperated by a semicolon:
 *
 *    -*- mode: NAME; evalfile: FILE; VAR: VALUE -*-
 *
 *  plus a mode shorthand form, where the first argument is taken as the
 *  mode type (compat with older crisp format);
 *
 *    -*- mak: tabs: 8; -*-
 */
static int
ml_boolean(string value)
{
    value = lower(value);
    switch(value) {
    case "yes":  case "y":
        return 1;
    case "no":   case "n":
        return 0;
    }
    return -1;
}


static void
ml_emacs(string cmdline)
{
    list cmdlist;
    string token, value;
    int i, j;

    /*
     *  mode short hand
     *      C, C++, Cpp, pl and Mak
     */
    i = 0;
    cmdline = trim(cmdline);
    token = lower(cmdline);
    if      (substr(token, 1, 2) == "c:")   { i = 2; }
    else if (substr(token, 1, 4) == "c++:") { i = 4; }
    else if (substr(token, 1, 4) == "cpp:") { i = 4; }
    else if (substr(token, 1, 3) == "pl:")  { i = 3; }
    else if (substr(token, 1, 4) == "mak:") { i = 4; }

    /*
     *  bust line
     */
    if (0 == i) {
        cmdlist = split(cmdline, ";");          /* standard */
    } else {                                    /* shorthand */
        cmdlist[0] = "mode: " + substr(cmdline, 1, i-1);
        cmdlist += split(substr(cmdline, i+1), ";");
    }

    /*
     *  parse each "token: value" component
     */
    for (i = 0; i < length_of_list(cmdlist); ++i) {
        token = compress(cmdlist[i]);           /* current token */

        if ((j = index(token, ":")) > 0) {
            /*
             *  token: value[;]
             */
            value = trim(substr(token, j+1));
            token = lower(trim(substr(token, 1, j-1)));
            if ((j = index(value, ";")) > 0) {
                value = substr(value, 1, j-1);  /* strip terminator */
            }
            value = lower(trim(value));

            dprintf("modeline(%s,%s)\n", token, value);
            switch(token) {
            case "mode":                        /* file mode (Major) */
                if (value != "fold") {
                    mode(value);                /* fold'ing ignored */
                }
                break;

            case "evalfile:":
                break;
            case "eval:":
                break;

            /*
             *  Encode 'standard' variables
             */
            case "encoding:":                   /* consumed by file-type guessor */
            case "coding:":
                break;

            case "indent-tabs-mode":
                if ("nil" == value) {
                    use_tab_char("no");         /* disable hard tabs */

                } else if (value == substr("t", 1, 1)) {
                    use_tab_char("yes");        /* enable hard tabs */
                }
                break;

            case "hard-tabs": {                 /* hard or soft tabs (yes or no) */
                    int use_tab = ml_boolean(value);
                    if (-1 != use_tab) {
                        use_tab_char(use_tab);
                    }
                }
                break;

            case "show-tabs":                   /* show hard tabs (yes or no) */
                switch(ml_boolean(value)) {
                case 1:
                    set_buffer_flags(NULL, "hiwhitespace");
                    break;
                case 0:
                    set_buffer_flags(NULL, NULL, "hiwhitespace");
                    break;
                }
                break;

            case "line-numbers":                /* show line numbers (yes or no) */
                switch(ml_boolean(value)) {
                case 1:
                    set_buffer_flags(NULL, "line_numbers");
                    break;
                case 0:
                    set_buffer_flags(NULL, NULL, "line_numbers");
                    break;
                }
                break;

            case "tab-width":                   /* buffer display tab size */
                set_tab(atoi(value));
                break;

            case "left-margin-width":
                set_margins(NULL, atoi(value));
                break;

            case "right-margin-width":
                set_margins(NULL, NULL, 80 - atoi(value));
                break;

            case "scroll-bar-mode":
                set_ctrl_state(WCTRLO_VERT_SCROLL, (value == "nil" ? WCTRLS_DISABLE : WCTRLS_ENABLE));
                break;

            case "tabs":                        /* display and indentation tab setting */
                set_indent(atoi(value));
                tabs(atoi(value)+1);
                break;

            case "indent-width":
                set_indent(atoi(value));
                break;

            /*
             *  CC mode variables (http://www.gnu.org/software/emacs/manual/ccmode.html)
             */
            case "c-basic-offset":              /* indentation tab size */
                set_indent(atoi(value));
                break;
            }

        } else {                                /* value */
            if (i != 0) {                       /* only allowed as first parameter, stop */
                break;                          /* eg.  -*- perl -*- */
            }
            mode(token);
        }
    }
}

/*
 *  Local Variables: ***
 *  mode: cr ***
 *  tabs: 4 ***
 *  End: ***
 */
