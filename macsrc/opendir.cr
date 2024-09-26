/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: opendir.cr,v 1.3 2024/07/28 12:27:44 cvsuser Exp $
 * opendir interface
 *
 *
 */

#include "grief.h"
#include "dialog.h"                             /* dialog manager */

#if defined(DEBUG)
#define Debug(x)            debug x;            /* local debug diag's */
#else
#define Debug(x)
#endif

/* Constants */
#define SYNTAX              "opendir_coloriser"

#define FORWARD_SLASH       '/'
#define BACKWARD_SLASH      '\\'
#define WILD                "?*"
#define SMINCOLUMNS         62                  /* screen width */
#define SFILECOLUMNS        (opendir_fwidth)
#define SINFOCOLUMNS        38
#define TYPEPOS(__base)     (__base + 1)
#define NAMEPOS(__base)     (__base + 2)

#if defined(MSDOS)
#define MARKER              "\xfe"
#else
#define MARKER              "="
#endif

/* Anchor types (See drop_anchor()) */
#define ANCHOR_LINE         3

/* Directory movement  */
#define DIR_UP              ".."
#define DIR_DOWN            "./"

/* Search modes */
#define SEL_NEXT            1
#define SEL_TOP             2
#define SEL_PREV            0

/* Multiple file selection mode */
#define MARK_SELECT         1
#define MARK_UNSELECT       2
#define MARK_INVERT         3


/* Functions */
#if defined(__PROTOTYPES__)
int                         opendir(~ string);

static void                 OdSize(void);
static int                  OdPosition(int line);
static void                 OdPrompt(void);
static void                 OdCd(~ string);
static void                 OdRead(string cwd);
static int                  sort_versions(int i1, string s1, int l2, string s2);
static void                 OdMessage(void);
static void                 OdSpace(void);
static void                 OdBack(void);
static void                 OdMatch(string);
static int                  OdNext(string fname, int dir);
static int                  OdSearch(int next, string pattern);
static void                 OdChdir(void);
static void                 OdUpdir(void);
static void                 OdEscape(void);
static void                 OdAction(void);
static int                  OdPosition(int line);
static void                 OdPrompt(void);
static void                 OdHome(void);
static void                 OdUp(void);
static void                 OdDown(void);
static void                 OdEnd(void);
static void                 OdPgup(void);
static void                 OdPgdn(void);
static int                  OdToggle(void);
static void                 OdSelect(void);
static void                 OdUnselect(void);
static void                 OdInverse(void);
static void                 OdMultiple(int);
static void                 OdSort(~ string);
#endif   /*__PROTOTYPES__*/

static int                  odkeymap;           /* keyboard map */

static int                  odrows, odcols;     /* screen rows/cols */
static int                  odwidth, odfwidth;

static int                  odlineno;           /* current line count */
static int                  odactiveno;         /* selected count */
static string               odexitname;         /* file buffer */
static string               odpattern;          /* selection pattern */

static list                 oddirlist = {       /* Mkdir/Chdir */
    "Name: ",      ""
    };

static list                 odpatlist = {       /* Select/unselect */
    "Pattern: ",   ""
    };

static list                 odsortlist = {      /* Sort order */
    "Sort by: ",    {"Name", "Extension", "Unsorted"}
    };

static string               odsortorder = "Name";


/*
 *  Run-time initialisation.
 */
void
main()
{
    /* syntax */
    create_syntax(SYNTAX);
    syntax_rule("^[ =][/~][^ ]+",   "lsdirectory");
    syntax_rule("^[ =]\\*[^ ]+",    "lsexecute");
    syntax_rule("^[ =]\\@[^ ]+",    "lssymlink");
    syntax_rule("^[ =]\\|[^ ]+",    "lspipe");
    syntax_rule("^[ =]\\![^ ]+",    "lserror"); /* broken link */
    syntax_rule("^[ =][-=S+][^ ]+", "lsspecial");
    syntax_rule("^[ =][^-/~@|!*=$+][^ ]+", "lsnormal");
    syntax_build(__COMPILETIME__);

    /* build keyboard definition */
    keyboard_push();
    keyboard_typeables();                       /* keys 0...127 */

    assign_to_key("<Enter>",        "::OdEnter");
    assign_to_key("<Backspace>",    "::OdBack");
    assign_to_key("<Space>",        "::OdSpace");
    assign_to_key("<Home>",         "::OdHome");
    assign_to_key("<End>",          "::OdEnd");
    assign_to_key("<Up>",           "::OdUp");
    assign_to_key("<Down>",         "::OdDown");
    assign_to_key("<Wheel-Up>",     "::OdUp");
    assign_to_key("<Wheel-Down>",   "::OdDown");
    assign_to_key("<PgUp>",         "::OdPgup");
    assign_to_key("<PgDn>",         "::OdPgdn");
    assign_to_key("<Tab>",          "nothing");
    assign_to_key("<F1>",           "help_display features/opendir.hlp");
    assign_to_key("<Alt-H>",        "help_display features/opendir.hlp");
    assign_to_key("<Esc>",          "::OdEscape");
    assign_to_key("<F4>",           "::OdAction");
    assign_to_key("<F10>",          "::OdAction");
    assign_to_key("<Ctrl-D>",       "::OdChdir");
    assign_to_key("<Ctrl-R>",       "::OdCd");
    assign_to_key("<Ctrl-PgUp>",    "::OdUpdir");
#if defined(MSDOS)
    assign_to_key("<Alt-F1>",       "::OdDrive");
#endif
    assign_to_key("<Alt-F>",        "ff");
    assign_to_key("<Ins>",          "::OdToggle");
    assign_to_key("<Keypad-Plus>",  "::OdSelect");
    assign_to_key("<Keypad-Minus>", "::OdUnselect");
    assign_to_key("<Keypad-Star>",  "::OdInverse");
    assign_to_key("<Ctrl-F2>",      "::OdSort Unsorted");
    assign_to_key("<Ctrl-F3>",      "::OdSort Name");
    assign_to_key("<Ctrl-F4>",      "::OdSort Extension");
    assign_to_key("<F9>",           "::OdSort");

    odkeymap = inq_keyboard();
    keyboard_pop(1);                            /* save keyboard */
    OdSize();
}


/*
 *  Open directory interface, invoked on start upon a directory being presented for edit.
 */
int
opendir(~ string)
{
    string dir, cwd;                            /* saved directory */
    int old_buffer, old_window;
    int new_buffer = -1;
    int dir_buffer;
    int base;

    /* Initialise directory */
    old_buffer = inq_buffer();
    old_window = inq_window();
    if ((dir_buffer = create_buffer("opendir", NULL, 1)) < 0) {
        return -1;
    }

    getwd(NULL, cwd);                           /* current working dir */
    if (! get_parm(0, dir) || 0 == strlen(dir) || dir == ".")
        dir = cwd;

    OdSize();
    set_buffer(dir_buffer);
    attach_syntax(SYNTAX);
    set_buffer_type(NULL, BFTYP_UTF8);
    OdRead(dir);

    create_window(odcols, odrows + 4, odcols + odwidth, 3,
            "<F1> Help, <Ins/+/-/Enter> select>, <F10 edit>, <Esc> to exit");
    attach_buffer(dir_buffer);
    OdPosition(1);
    odpattern = "";

    /* User process */
    keyboard_push(odkeymap);
    register_macro(0, "::OdBadkey", 1);         /* local Bad key */
    process();
    unregister_macro(0, "::OdBadkey", 1);
    keyboard_pop(1);                            /* pop, but retain */
    delete_window();

    /* Load selected files */
    if (strlen(odexitname)) {                    /* exit condition */
        if (odexitname == MARKER) {
            /* Load all marked files */
            string linebuf;

            top_of_buffer();
            while (search_fwd("<\\c" + MARKER)) {
                linebuf = read();
                base = index(linebuf, '\t');  /* delimiter */
                odexitname = "./" + trim(substr(linebuf, NAMEPOS(base)));
                down();

                message("Loading \"%s\" for editing...", odexitname);
                if (edit_file(odexitname) > 0) {
                    if (new_buffer == -1) {     /* first */
                        new_buffer = inq_buffer();
                    }
                }
                set_buffer(dir_buffer);         /* for next search */
            }
        } else {
            /* Load specified file */
            message("Loading \"%s\" for editing...", odexitname);
            if (edit_file(odexitname) > 0) {
                if (new_buffer == -1) {
                    new_buffer = inq_buffer();
                }
            }
        }
        display_file_name();
    }

    cd(cwd);                                    /* restore cwd */

    delete_buffer(dir_buffer);
    if (new_buffer == -1)
        new_buffer = old_buffer;
    set_buffer(new_buffer);
    set_window(old_window);
    attach_buffer(new_buffer);
    return (new_buffer == -1 ? 0 : 1);
}


static void
OdSize(void)
{
    inq_screen_size(odrows, odcols);
    if (odcols > SMINCOLUMNS + 2) {
        odwidth = (odcols / 8) * 7;             /* 7/8 width */
    } else {
        odwidth = SMINCOLUMNS;                  /* center window */
    }
    odfwidth = odwidth - SINFOCOLUMNS;
    odcols -= (odwidth + 2);                    /* center window */
    odcols /= 2;
    odrows -= 10;                               /* rows-10 screen size */
}


static string
OdBadkey()
{
    string c;

    left();
    c = read(1);
    delete_char();
    if (c == "/" || c == "\\") {
        OdCd("/");
    } else {
        OdMatch(lower(c));
    }
    return "";
}


static void
OdMessage(void)
{
    message("File: %s", odpattern);
}


static void
OdSpace(void)
{
    if (odpattern == "") {
        if (OdToggle() != 0) {
            OdDown();
        }
    } else {
        if (OdNext(odpattern, SEL_NEXT) == -1) {
            OdNext(odpattern, SEL_TOP);
        }
    }
}


static void
OdBack(void)
{
    int len;

    if ((len = strlen(odpattern)) > 0) {
        odpattern = substr(odpattern, 1, len - 1);
        if (strlen(odpattern) == 0) {
            OdPosition(1);
        } else {
            if (OdNext(odpattern, SEL_TOP) == -1) {
                beep();
            }
        }
        OdMessage();
    }
}


static void
OdMatch(string new_key)
{
    string new_pattern;

    new_pattern = odpattern;
    new_pattern += new_key;
    if (strlen(new_pattern) == 1 || OdNext(new_pattern, SEL_NEXT) == -1) {
        if (OdNext(new_pattern, SEL_TOP) == -1) {
            beep();
            return;
        }
    }
    odpattern = new_pattern;
    OdMessage();
}


/*
 *  Finds the next matching file name.
 */
static int
OdNext(string fname, int dir)
{
    save_position();                            /* Save the current position. */

    switch(dir) {
    case SEL_TOP:                               /* From top of buffer. */
        top_of_buffer();
        break;
    case SEL_NEXT:                              /* Next match. */
        down();
        break;
    case SEL_PREV:                              /* Prev match. */
        up();
        break;
    }

    if (OdSearch(dir, "<" + MARKER + "|  "+fname) <= 0) {
        /* not found, restore position */
        restore_position();
        return -1;
    }
    restore_position(0);                        /* found, eat position. */
    return OdPosition(0);
}


/*
 *  Searches forward and back depending on first parameter if that parameter is not zero it searches forward else back.
 */
static int
OdSearch(int next, string pattern)
{
    return (next ? search_fwd(pattern, 1, 0, 0) : search_back(pattern, 1, 0, 0));
}


#if defined(MSDOS)
/*
 *  Change-drive
 */
static void
OdDrive(void)
{
    string drive;

    drive = "?";
    while (drive != "" && (drive < "A" || drive > "Z")) {
        if (drive != "?")
            beep();
        drive = "";
        get_parm(NULL, drive, "New Drive Letter? ", 1);
        if (drive == "?")
            beep();
        drive = upper(drive);
    }

    if (drive >= "A" && drive <= "Z") {
        OdCd(drive + ":");
    } else {
        beep();
        OdPrompt();
    }
}
#endif  /*MSDOS*/


/*
 *  Change-directory
 */
static void
OdChdir(void)
{
    list result;

    result[0] = "";
    result = field_list("Chdir", result, oddirlist, TRUE);
    if (result[0] == "") {
        OdPrompt();
    } else {
        OdCd(result[0]);
    }
}


/*
 *  Up-directory
 */
static void
OdUpdir(void)
{
    string curdir;

    getwd(NULL, curdir);
    if (strlen(curdir) > 3) {
        OdCd(DIR_UP);
    } else {
        beep();
    }
}


/*
 *  <ESC> escape.
 */
static void
OdEscape(void)
{
    odexitname = "";
    exit();
}


/*
 *  <F4/F10>, action selection.
 */
static void
OdAction(void)
{
    if (odactiveno) {
        odexitname = MARKER;
        exit();
    } else {
        beep();
    }
}


/*
 *  Position the cursor.
 */
static int
OdPosition(int line)
{
    raise_anchor();
    if (line == 0)
        inq_position(line);
    if (line > odlineno)
        line = odlineno;
    move_abs(line, 1);
    drop_anchor(ANCHOR_LINE);
    refresh();
    return line;
}


/*
 *  Update command prompt.
 */
static void
OdPrompt(void)
{
    string currdir;

    getwd(NULL, currdir);
    message("%s", currdir);
}


/*
 *  Cursor controls.
 */
static void
OdHome(void)
{
    OdPosition(1);
}


static void
OdUp(void)
{
    up();
    OdPosition(0);
}


static void
OdDown(void)
{
    int line;

    inq_position(line);
    if (line < odlineno) {
        beginning_of_line();
        down();
        OdPosition(0);
    }
}


static void
OdEnd(void)
{
    OdPosition(odlineno);
}


static void
OdPgup(void)
{
    int count, line;

    inq_position(line);
    count = odrows;
    line -= count;
    if (line <= 1)
        line = 1;                               /* HOME */
    else while (count)
        --count;
    OdPosition(line);
}


static void
OdPgdn(void)
{
    int count, line;

    inq_position(line);
    count = odrows;
    line += count;
    if (line <= odlineno) {
        while (count) {
            --count;
        }
    } else {
        line = odlineno;                        /* END */
    }
    OdPosition(line);
}


/*
 *  Multiple file marking.
 */
static void
OdSelect(void)
{
    OdMultiple(MARK_SELECT);
}


/*
 *  Multiple file unmarking.
 */
static void
OdUnselect(void)
{
    OdMultiple(MARK_UNSELECT);
}


/*
 *  Invert file marking.
 */
static void
OdInverse(void)
{
    OdMultiple(MARK_INVERT);
}


/*
 *  Toggle file mark of current file
 */
static int
OdToggle(void)
{
    string linebuf;
    int base;

    linebuf = read();
    base = index(linebuf, '\t');                /* delimiter */
    if (substr(linebuf, TYPEPOS(base), 1) != "F") {
        return (-1);
    }
    delete_char();
    if (substr(linebuf,1,1) == MARKER) {
        --odactiveno;
        insert(" ");
    } else {
        insert(MARKER);
        ++odactiveno;
    }
    beginning_of_line();
    OdDown();
    return (0);
}


/*
 *  Multiple file marking engine
 */
static void
OdMultiple(int mark_type)
{
    list result;
    int base;

    result[0] = "";
    result = field_list((mark_type == MARK_SELECT ? "Select" :
                    (mark_type == MARK_UNSELECT ? "Unselect" : "Invert")),
                result, odpatlist, TRUE);

    if (result[0] == "") {
        OdPrompt();

    } else {
        int line_no, line;
        string linebuf, pat;

        pat = result[0];
        message("Marking specified files... (%s)", pat);

        inq_position(line);
        line_no = odlineno;
        while (line_no) {
            move_abs(line_no, 1);
            linebuf = read();
            base = index(linebuf, '\t');      /* delimiter */
            if (substr(linebuf, TYPEPOS(base), 1) == "F") 
            {
                string name = trim(substr(linebuf, NAMEPOS(base)));
                if (re_search(NULL, pat, name) > 0) {
                    message(".. %s", name);
                    switch (mark_type) {
                    case MARK_INVERT:
                        delete_char();
                        if (substr(linebuf,1,1) == MARKER) {
                            --odactiveno;
                            insert(" ");
                        } else {
                            ++odactiveno;
                            insert(MARKER);
                        }
                        break;
                    case MARK_SELECT:
                        if (substr(linebuf,1,1) != MARKER) {
                            delete_char();
                            ++odactiveno;
                            insert(MARKER);
                        }
                        break;
                    case MARK_UNSELECT:
                        if (substr(linebuf,1,1) == MARKER) {
                            delete_char();
                            --odactiveno;
                            insert(" ");
                        }
                        break;
                    }
                }
                beginning_of_line();
            }
            --line_no;
        }
        OdPosition(line);
    }
    OdPrompt();
}


/*
 *  <Enter> processing
 */
static void
OdEnter(void)
{
    string linebuf;
    int base;

    linebuf = read();
    base = index(linebuf, '\t');                /* delimiter */

    if (substr(linebuf, TYPEPOS(base), 1) == "D") {
        /*
         *  Directory, move up/down.
         */
        string name = trim(substr(linebuf, NAMEPOS(base)));
        if (name == "..") {
            OdCd(DIR_UP);
        } else {
            OdCd(DIR_DOWN + name);
        }

    } else if (substr(linebuf, TYPEPOS(base), 1) == "F") {
        /*
         *  File, either action or toggle.
         */
        if (0 == odactiveno) {
            odexitname = "./" + trim(substr(linebuf, NAMEPOS(base)));
            exit();
        } else {
            OdToggle();
        }
    }
}


/*
 *   DIR selection.
 */
static void
OdCd(~ string)
{
    string cwd, olddir;
    int idx, pos;

    /* Retrieve parameters (option path) */
    if (! get_parm(0, cwd)) {
        cwd = "";
    }

    /* Change DIRECTORY */
    getwd(NULL, olddir);
    if (strlen(cwd)) {
        cd(cwd);
    } else {
        cwd = olddir;
    }

    /* Refresh directory list */
    odpattern = "";
    OdRead(cwd);

    /* Position cursor at parent directory */
    pos = 1;
    getwd(NULL, cwd);
    idx = strlen(cwd);
    if (idx < strlen(olddir)) {
        int dot;

        if (idx > 3) {
            ++idx;
        }
        ++idx;
        olddir = substr(olddir, idx);
        dot = index(olddir, ".");
        if (dot) {
            olddir = substr(olddir, 1, dot-1) + " +" + substr(olddir, dot+1);
        }
        if (search_fwd("< /"+olddir+"[ ]+") > 0) {
            inq_position(pos);
        }
    }

    /* directory position restore */
    OdPosition(pos);
    OdPrompt();
}


/*
 *  Read the current directory.
 */
static void
OdRead(string cwd)
{
    int cyear, year, day, hour, min;
    int bytes, mtime, mode;
    int pass, lines;
    string name, mon, size;
    string buffer;

    /*
     *    read directory
     */
    date(cyear, NULL, NULL);
    message("Reading disk directory...");

    odlineno = 0;                               /* line count */
    odactiveno = 0;
    clear_buffer();
    for (pass = 1; pass <= 2; pass++) {
        int start = odlineno;                   /* first line-index of section 0... */

        if (odsortorder == "Unsorted" && 2 == pass) {
            break;                              /* unsorted is single pass */
        }

        file_pattern(WILD);
        while (find_file(name, bytes, mtime, NULL, mode)) {
            if  (S_IFDIR & mode) {
                /*
                 *  Directories
                 */
                if ((odsortorder != "Unsorted" && 2 == pass) ||
                        name == "." ) {         /* skip on 2nd sort pass */
                    continue;                   /* or if '.' directory */
                }
                sprintf(buffer, " /%-*.*W|  <DIR>|", odfwidth, odfwidth, name);

            } else {
                /*
                 *  Files etc
                 */
                if (odsortorder != "Unsorted" && 1 == pass) {
                    continue;                   /* skip on 1st sort pass */
                }

                if (bytes < 9999999) {
                    sprintf(size, "%d", bytes);
                } else if ((bytes /= 1024) < 999999) {
                    sprintf(size, "%dK", bytes);
                } else if ((bytes /= 1024) < 999999) {
                    sprintf(size, "%dM", bytes);
                } else {
                    sprintf(size, "%dG", bytes/1024);
                }

                sprintf(buffer, " %s%-*.*W|%7s|",
                    substr(mode_string(mode, cwd+"/"+name, TRUE), 1, 1), odfwidth, odfwidth, name, size);
            }

            buffer += mode_string(mode);        /* decode mode */

            localtime(mtime, year, NULL, day, mon, NULL, hour, min, NULL);
            if (cyear == year) {
                sprintf(buffer, "%s|%s %2d %02d:%02d   ?", buffer, substr(mon,1,3), day, hour, min);
            } else {
                sprintf(buffer, "%s|%s %2d  %04d   ?", buffer, substr(mon,1,3), day, year);
            }
            insert(buffer);

            if (mode & S_IFDIR) {               /* <delimiter><type><name><nl> */
                insert(" \tD" + name + "\n");
            } else {
                insert(" \tF" + name + "\n");
            }
            ++odlineno;
        }

        delete_line();
        if (odsortorder != "Unsorted") {
            if (odsortorder == "Name") {
                sort_buffer(NULL, "::sort_names", start + 1);
            } else if (odsortorder == "Extension") {
                sort_buffer(NULL, "::sort_extensions", start + 1);
            }
        }
    }

    inq_screen_size(NULL, lines);               /* pad image */
    sprintf(buffer, "  %-*s|%7s|%10s|%12s\n", odfwidth, "", "", "", "");
    while (lines-- > 0) {
        insert(buffer);
    }

    top_of_buffer();
    message("done...");
}


static int
sort_names(int l1, string s1, int l2, string s2)
{
    int base = index(s1, '\t');                 /* delimiter */

    UNUSED(l1, l2);
    s1 = substr(s1, NAMEPOS(base));
    s2 = substr(s2, NAMEPOS(base));
    return strfilecmp(s1, s2);
}


static int
sort_extensions(int l1, string s1, int l2, string s2)
{
    int ext, base = index(s1, '\t');            /* delimiter */

    UNUSED(l1, l2);
    ext = rindex(s1, '.');
    if (ext > base)
        s1 = substr(s1, ext + 1);
    else
        s1 = "";

    ext = rindex(s2, '.');
    if (ext > base)
        s2 = substr(s2, ext + 1);
    else
        s2 = "";

    return strfilecmp(s1, s2);
}


/*
 *  <sort>
 */
static void
OdSort(~ string)
{
    string sortorder;

    if (! get_parm(0, sortorder)) {
        list result;

        result[0] = odsortorder;
        result = field_list("Sort Order", result, odsortlist, TRUE);
        if (result[0] != "" && result[0] != odsortorder) {
            sortorder = result[0];
        }
    }

    if (sortorder) {
        message("sorting by %s...", sortorder);
        odsortorder = sortorder;
        OdCd();
    }
}

/*end*/
