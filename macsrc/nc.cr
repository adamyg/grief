/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: nc.cr,v 1.34 2021/07/05 15:01:29 cvsuser Exp $
 * Norton Commander (NC) style directory services
 *
 *
 */

#include "grief.h"
#include "dialog.h"                                 /* dialog manager */

#if defined(DEBUG)
#define Debug(x)            debug x;                /* local debug diag's */
#else
#define Debug(x)
#endif

/* Constants */
#define SYNTAX              "nc_coloriser"

#define FORWARD_SLASH       '/'
#define BACKWARD_SLASH      '\\'
#define WILD                "?*"
#define SMINCOLUMNS         62                      /* screen width */
#define SMAXCOLUMNS         90
#define SFILECOLUMNS        (nc_fwidth)
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
void                        mc(void);
void                        nc(void);

static void                 NcSize(void);
static int                  NcPosition(int line);
static void                 NcPrompt(void);
static void                 NcCd(~ string);
static void                 NcDirectory(string cwd);
static int                  sort_versions(int i1, string s1, int l2, string s2);
static void                 NcSelMsg(void);
static void                 NcSelSpace(void);
static void                 NcSelBack(void);
static void                 NcSel(string);
static int                  NcSelFind(string fname, int dir);
static int                  NcSelSearch(int next, string pattern);
static void                 NcMkdir(void);
static void                 NcChdir(void);
static void                 NcUpdir(void);
static void                 NcTree(void);
static void                 NcDelete(void);
static void                 NcRemove(void);
static void                 NcEdit(void);
static void                 NcExit(void);
static int                  NcPosition(int line);
static void                 NcPrompt(void);
static void                 NcHome(void);
static void                 NcUp(void);
static void                 NcDown(void);
static void                 NcEnd(void);
static void                 NcPgup(void);
static void                 NcPgdn(void);
static int                  NcMark(void);
static void                 NcMarkSet(void);
static void                 NcMarkClr(void);
static void                 NcMarkInv(void);
static void                 NcMarkMultiple(int);
#endif   /*__PROTOTYPES__*/

static string               ncsortorder = "Name";
static int                  nckeymap;           /* keyboard map */

static int                  ncrows, nccols;     /* screen rows/cols */
static int                  ncwidth, ncfwidth;

static int                  nclineno;           /* current line count */
static string               nclinebuf;          /* line buffer */
static string               ncfilebuf;          /* file buffer */
static string               ncselpattern;       /* selection pattern */

static list                 ncdirlist = {       /* Mkdir/Chdir */
    "Name: ",      ""
    };

static list                 ncpatlist = {       /* Select/unselect */
    "Pattern: ",   ""
    };

static list                 ncsortlist = {      /* Sort order */
    "Sort by: ",    {"Name", "Extension", "Time", "Size", "Unsorted"}
    };


/*
 *  main ---
 *      Run-time initialisation.
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
    syntax_rule("^[ =]\\![^ ]+",    "lserror"); // broken link
    syntax_rule("^[ =][-=S+][^ ]+", "lsspecial");
    syntax_rule("^[ =][^-/~@|!*=$+][^ ]+", "lsnormal");
    syntax_build(__COMPILETIME__);

    /* build keyboard definition */
    keyboard_push();
    keyboard_typeables();                       // keys 0...127

    assign_to_key("<Enter>",        "::NcEnter");
    assign_to_key("<Backspace>",    "::NcSelBack");
    assign_to_key("<Space>",        "::NcSelSpace");
 /* assign_to_key("<Esc>",          "::NcSelEsc");      -- exit. */
    assign_to_key("<Home>",         "::NcHome");
    assign_to_key("<End>",          "::NcEnd");
    assign_to_key("<Up>",           "::NcUp");
    assign_to_key("<Down>",         "::NcDown");
    assign_to_key("<Wheel-Up>",     "::NcUp");
    assign_to_key("<Wheel-Down>",   "::NcDown");
    assign_to_key("<PgUp>",         "::NcPgup");
    assign_to_key("<PgDn>",         "::NcPgdn");
    assign_to_key("<Tab>",          "nothing");
    assign_to_key("<F1>",           "help_display features/nc.hlp");
    assign_to_key("<Alt-H>",        "help_display features/nc.hlp");
 /* assign_to_key("<F2>",           "::NcMenu");        -- User menu. */
 /* assign_to_key("<F3>",           "::NcView");        -- Viewer. */
    assign_to_key("<F4>",           "::NcEdit");
 /* assign_to_key("<F5>",           "::NcCopy");        -- Copy. */
 /* assign_to_key("<F6>",           "::NcRename");      -- Rename/move. */
    assign_to_key("<F7>",           "::NcMkdir");
    assign_to_key("<F8>",           "::NcDelete");
    assign_to_key("<F9>",           "::NcSort");        /* Sort order. */
    assign_to_key("<F10>",          "::NcExit");
    assign_to_key("<Esc>",          "::NcExit");
    assign_to_key("<Ctrl-D>",       "::NcChdir");       /* Change directory. */
    assign_to_key("<Ctrl-R>",       "::NcCd");          /* Reread. */
    assign_to_key("<Ctrl-PgUp>",    "::NcUpdir");
    assign_to_key("<Ctrl-F3>",      "::NcSort F");      /* Filename. */
    assign_to_key("<Ctrl-F4>",      "::NcSort E");      /* Extension. */
    assign_to_key("<Ctrl-F5>",      "::NcSort T");      /* Time. */
    assign_to_key("<Ctrl-F6>",      "::NcSort S");      /* Size. */
    assign_to_key("<Ctrl-F7>",      "::NcSort U");      /* Unsorted. */
#if defined(MSDOS)
    assign_to_key("<Alt-F1>",       "::NcDrive");       /* DOS special. */
#endif
    assign_to_key("<Alt-F7>",       "ff");
    assign_to_key("<Alt-F10>",      "::NcTree");
    assign_to_key("<Del>",          "::NcDelete");
    assign_to_key("<Ins>",          "::NcMark");
    assign_to_key("<Keypad-Minus>", "::NcMarkClr");
    assign_to_key("<Keypad-Plus>",  "::NcMarkSet");
    assign_to_key("<Keypad-Star>",  "::NcMarkInv");

    nckeymap = inq_keyboard();
    keyboard_pop(1);                            /* save keyboard */
    NcSize();
}


static void
NcSize(void)
{
    inq_screen_size(ncrows, nccols);
    if (nccols > SMINCOLUMNS + 2) {
        if ((ncwidth = nccols - 6) > SMAXCOLUMNS) {
            ncwidth = SMAXCOLUMNS;
        }
    } else {
        ncwidth = SMINCOLUMNS;                  /* center window */
    }
    ncfwidth = ncwidth - SINFOCOLUMNS;
    nccols -= (ncwidth + 2);                    /* center window */
    nccols /= 2;
    ncrows -= 10;                               /* rows-10 screen size */
}


/*
 *  mc ---
 *      Directory services (NC/Midnight commander style).
 */
void
mc(void)
{
    nc();
}


/*
 *  nc ---
 *      Directory services (NC/Midnight commander style).
 */
void
nc(void)
{
    string cwd;                                 /* saved directory */
    int old_buffer, old_window;
    int dir_buffer;
    int base;

    /* Initialise directory */
    old_buffer = inq_buffer();
    old_window = inq_window();
    dir_buffer = create_buffer("NC", NULL, 1);
    if (dir_buffer == -1) {
        return;                                 /* template missing */
    }
    NcSize();

    getwd(NULL, cwd);                           /* current working dir */
    set_buffer(dir_buffer);
    attach_syntax(SYNTAX);
    set_buffer_type(NULL, BFTYP_UTF8);
    NcDirectory(cwd);

    create_window(nccols, ncrows + 4, nccols + ncwidth, 3,
            "<F1> Help, <Enter> to select, <F10/Esc> to exit");
    attach_buffer(dir_buffer);
    NcPosition(1);
    ncselpattern = "";

    /* User process */
    keyboard_push(nckeymap);
    register_macro(0, "::NcBadkey", 1);         /* local Bad key */
    process();
    unregister_macro(0, "::NcBadkey", 1);
    keyboard_pop(1);                            /* pop, but retain */
    delete_window();

    /* Load selected files */
    if (strlen(ncfilebuf)) {                    /* F10 or Esc ? */
        if (ncfilebuf == MARKER) {
            /* Load all marked files */
            top_of_buffer();
            while (search_fwd("<\\c" + MARKER)) {
                nclinebuf = read();
                base = index(nclinebuf, '\t');  /* delimiter */
                ncfilebuf = "./" + trim(substr(nclinebuf, NAMEPOS(base)));
                down();

                message("Loading \"%s\" for editing...", ncfilebuf);
                if (edit_file(ncfilebuf) > 0) {
                    old_buffer = inq_buffer();
                }
                set_buffer(dir_buffer);         /* for next search */
            }
        } else {
            /* Load specified file */
            message("Loading \"%s\" for editing...", ncfilebuf);
            if (edit_file(ncfilebuf) > 0) {
                old_buffer = inq_buffer();
            }
        }
        display_file_name();
    }

    cd(cwd);                                    /* restore cwd */

    delete_buffer(dir_buffer);

    set_buffer(old_buffer);
    set_window(old_window);
    attach_buffer(old_buffer);
}


/*
 *  NcBadkey ---
 *      This macro keeps tabs on the "illegal" keys pressed at the prompt.
 */
static string
NcBadkey()
{
    string c;

    left();
    c = read(1);
    delete_char();
    if (c == "/") {
        NcCd("/");
    } else {
        NcSel(lower(c));
    }
    return "";
}


/*
 *  NcForeach ---
 *      For all the lines that are tagged, performs another macro that is passed to it
 *      as a parameter. If no lines are tagged, performs the macro on the current line.
 *-
void
NcForeach(string funcname)
{
    save_position();
    top_of_buffer();
    if (search_fwd("<\\c" + MARKER)) {
        do {
            execute_macro(funcname, read());
        } while(search_fwd("<\\c" + MARKER));
    } else {
        execute_macro(funcname, read());
    }
    restore_position();
}
-*/


/*
 *  NcSelMsg ---
 *      Display the current selected file name, APY 06/12/90
 */
static void
NcSelMsg(void)
{
    message("File: %s", ncselpattern);
}


/*
 *  NcSelSpace ---
 *      called whenever the user presses a <Space> key. Finds the next file name match
 *      ensuring the user does not go past the end of the buffer.
 */
static void
NcSelSpace(void)
{
    if (ncselpattern == "") {
        if (NcMark() != 0) {
            NcDown();
        }
    } else {
        if (NcSelFind(ncselpattern, SEL_NEXT) == -1) {
            NcSelFind(ncselpattern, SEL_TOP);
        }
    }
}


/*
 *  NcSelBack ---
 *      called whenever the user presses a <Backspace> key. Removes the last key from the
 *      buf search fname.
 */
static void
NcSelBack(void)
{
    int len;

    if ((len = strlen(ncselpattern)) > 0) {
        ncselpattern = substr(ncselpattern, 1, len - 1);
        if (strlen(ncselpattern) == 0) {
            NcPosition(1);
        } else {
            if (NcSelFind(ncselpattern, SEL_TOP) == -1) {
                beep();
            }
        }
        NcSelMsg();
    }
}


/*
 *  NcSelEsc ---
 *      called whenever the user presses a <Esc> (key. Clears the search file-name
 *
    static void
    NcSelEsc(void)
    {
        if (ncselpattern != "") {
            ncselpattern = "";
            NcSelMsg();
        }
    }
 */


/*
 *  NcSel ---
 *      Select engine.
 */
static void
NcSel(string new_key)
{
    string new_pattern;

    new_pattern = ncselpattern;
    new_pattern += new_key;
    if (strlen(new_pattern) == 1 || NcSelFind(new_pattern, SEL_NEXT) == -1) {
        if (NcSelFind(new_pattern, SEL_TOP) == -1) {
            beep();
            return;
        }
    }
    ncselpattern = new_pattern;
    NcSelMsg();
}


/*
 *  NcSelFind ---
 *      Finds the next matching file name.
 */
static int
NcSelFind(string fname, int dir)
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

    if (NcSelSearch(dir, "<" + MARKER + "|  "+fname) <= 0) {
        /* not found, restore position */
        restore_position();
        return -1;
    }
    restore_position(0);                        /* found, eat position. */
    return NcPosition(0);
}


/*
 *  NcSelSearch ---
 *      Searches forward and back depending on first parameter if that parameter is not
 *      zero it searches forward else back.
 */
static int
NcSelSearch(int next, string pattern)
{
    return (next ? search_fwd(pattern, 1, 0, 0) : search_back(pattern, 1, 0, 0));
}


#if defined(MSDOS)
/*
 *  NcDrive ---
 *      called whenever the user presses the <Alt-F1> key, "change current working drive".
 */
static void
NcDrive(void)
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
        NcCd(drive + ":");
    } else {
        beep();
        NcPrompt();
    }
}
#endif  /*MSDOS*/


/*
 *  NcMkdir ---
 *      called whenever the user presses the <F7> key, "make directory".
 */
static void
NcMkdir(void)
{
    list result;

    result[0] = "";
    result = field_list("Mkdir", result, ncdirlist, TRUE);
    if (result[0] == "") {
        NcPrompt();
    } else {
        message("Creating \"%s\"...", result[0]);
        if (mkdir(result[0]) == -1) {
            error("Unable to create \"%s\" : %d (%s)", result[0], errno, strerror(errno));
        } else {
            NcCd();
        }
    }
}


/*
 *  NcChdir ---
 *      called whenever the user presses the <Ctrl-d> key, "change directory".
 */
static void
NcChdir(void)
{
    list result;

    result[0] = "";
    result = field_list("Chdir", result, ncdirlist, TRUE);
    if (result[0] == "") {
        NcPrompt();
    } else {
        NcCd(result[0]);
    }
}


/*
 *  NcUpdir ---
 *      Process <Ctrl-PgUp> "up directory" key request
 */
static void
NcUpdir(void)
{
    string curdir;

    getwd(NULL, curdir);
    if (strlen(curdir) > 3) {
        NcCd(DIR_UP);
    } else {
        beep();
    }
}


/*
 *  NcTree ---
 *      called whenever the user presses the <Alt-F10> key, "directory tree".
 */
static void
NcTree(void)
{
    string newdir;

#if defined(MSDOS)
    newdir = tree("\\");
#else
    newdir = tree("~");
#endif
    if (newdir != "") {
        NcCd(newdir);
    }
}


/*
 *  NcDelete ---
 *      called whenever the user presses the <F8> or <Del> key, "delete".
 */
static void
NcDelete(void)
{
    string answer;
    int line, base;

    inq_position(line);
    raise_anchor();
    top_of_buffer();
    if (search_fwd("<\\c" + MARKER)) {
        answer = "?";
        while (upper(answer) != "Y" && upper (answer) != "N") {
            if (answer != "?") {
                beep();
            }
            answer = "";
            get_parm(NULL, answer, "Delete 'tagged' files [yn]? ", 1);
            if (answer == "") {
                answer = "N";
            } else if (answer == "?") {
                beep();
            }
        }

        if (upper(answer) == "Y") {
            top_of_buffer();
            while (search_fwd("<\\c" + MARKER)) {
                nclinebuf = read();
                beginning_of_line();
                down();

                base = index(nclinebuf, '\t');  /* delimiter */

                if (substr(nclinebuf, TYPEPOS(base), 1) == "D") {
                                                /* directory */
                    ncfilebuf = trim(substr(nclinebuf, NAMEPOS(base)));
                    if (ncfilebuf != "..") {
                        error("%s", nclinebuf);
                        message("Deleting Directory \"%s\"...", ncfilebuf);
                        if (-1 == rmdir(ncfilebuf)) {
                            error("Unable to rmdir \"%s\" : %d (%s)",
                                ncfilebuf, errno, strerror(errno));
                        }
                    }

                } else if (substr(nclinebuf, TYPEPOS(base), 1) == "F") {
                                                /* file */
                    ncfilebuf = "./" + trim(substr(nclinebuf, NAMEPOS(base)));
                    message("Deleting \"%s\"...", ncfilebuf);
                    if (remove(ncfilebuf) == -1) {
                        error("Unable to delete \"%s\" : %d (%s) - Press any key",
                                ncfilebuf, errno, strerror(errno));
                        beep();
                        while (!inq_kbd_char())
                            ;
                        read_char();
                    }
                }
            }
        } else {
            NcPosition(line);
            NcPrompt();
            return;
        }

    } else {
        NcPosition(line);
        nclinebuf = read();
        beginning_of_line();

        answer = "?";
        while (upper(answer) != "Y" && upper(answer) != "N") {
            if (answer != "?")
                beep();
            answer = "";
            get_parm(NULL, answer,
                "Delete 'highlighted' file [yn]? ", 1);
            if (answer == "") {
                answer = "N";
            } else if (answer == "?") {
                beep();
            }
        }

        if (upper(answer) == "Y") {
            base = index(nclinebuf, '\t');      /* delimiter */

            if (substr(nclinebuf, TYPEPOS(base), 1) == "D") {
                                                /* directory */
                ncfilebuf = trim(substr(nclinebuf, NAMEPOS(base)));
                if (ncfilebuf != "..") {
                    message("Deleting Directory \"%s\"...", ncfilebuf);
                    if (-1 == rmdir(ncfilebuf)) {
                        error("Unable to rmdir \"%s\" : %d (%s)",
                                ncfilebuf, errno, strerror(errno));
                    }
                }

            } else if (substr(nclinebuf, TYPEPOS(base), 1) == "F") {
                                                /* file */
                ncfilebuf = "./" + trim(substr(nclinebuf, NAMEPOS(base)));
                message("Deleting \"%s\"...", ncfilebuf);
                if (remove(ncfilebuf) == -1) {
                    error("Unable to delete \"%s\" : %d (%s) - Press any key",
                            ncfilebuf, errno, strerror(errno));
                    beep();
                    while(!inq_kbd_char())
                        ;
                    read_char();
                }
            }
        } else {
            NcPrompt();
            return;
        }
    }
    NcCd();
}


/*
 *  NcEdit ---
 *      called whenever the user presses the <F4>, "edit".
 */
static void
NcEdit(void)
{
    int line, base;

    inq_position(line);
    nclinebuf = read();
    raise_anchor();
    top_of_buffer();

    if (search_fwd("<\\c" + MARKER)) {
        ncfilebuf = MARKER;

    } else {
        base = index(nclinebuf, '\t');          /* delimiter */

                                                /* directory */
        if (substr(nclinebuf, TYPEPOS(base), 1) == "D") {
            error("Cannot edit directory, press any key...");
            beep();
            while(!inq_kbd_char())
                ;
            read_char();
            NcPosition(line);
            return;

                                                /* file */
        } else if (substr(nclinebuf, TYPEPOS(base), 1) == "F") {
            ncfilebuf = "./" + trim(substr(nclinebuf, NAMEPOS(base)));
        }
    }
    exit();                                     /* exit 'nc' */
}


/*
 *  NcExit ---
 *      Exit.
 */
static void
NcExit(void)
{
    ncfilebuf = "";
    exit();
}


/*
 *  NcPosition ---
 *      Position the cursor.
 */
static int
NcPosition(int line)
{
    raise_anchor();
    if (line == 0)
        inq_position(line);
    if (line > nclineno)
        line = nclineno;
    move_abs(line, 1);
    drop_anchor(ANCHOR_LINE);
    refresh();
    return line;
}


/*
 *  NcPrompt ---
 *      Update command prompt.
 */
static void
NcPrompt(void)
{
    string currdir;

    getwd(NULL, currdir);
    message("%s", currdir);
}


/*
 *  NcHome ---
 *      Home position.
 */
static void
NcHome(void)
{
    NcPosition(1);
}


/*
 *  NcUp ---
 *      Move up.
 */
static void
NcUp(void)
{
   up();
   NcPosition(0);
}


/*
 *  NcDown ---
 *      Move up.
 */
static void
NcDown(void)
{
    int line;

    inq_position(line);
    if (line < nclineno) {
        beginning_of_line();
        down();
        NcPosition(0);
    }
}


/*
 *  NcEnd ---
 *      Move to END of directory list.
 */
static void
NcEnd(void)
{
    NcPosition(nclineno);
}


/*
 *  NcPgup ---
 *      Move up a page.
 */
static void
NcPgup(void)
{
    int count, line;

    inq_position(line);
    count = ncrows;
    line -= count;
    if (line <= 1)
        line = 1;                               /* HOME */
    else while (count)
        --count;
    NcPosition(line);
}


/*
 *  NcPgdn ---
 *      Move down a page.
 */
static void
NcPgdn(void)
{
    int count, line;

    inq_position(line);
    count = ncrows;
    line += count;
    if (line <= nclineno) {
        while (count) {
            --count;
        }
    } else {
        line = nclineno;                        /* END */
    }
    NcPosition(line);
}


/*
 *  NcMarkSet ---
 *      Multiple file marking.
 */
static void
NcMarkSet(void)
{
    NcMarkMultiple(MARK_SELECT);
}


/*
 *  NcMarkClr ---
 *      Multiple file unmarking.
 */
static void
NcMarkClr(void)
{
    NcMarkMultiple(MARK_UNSELECT);
}


/*
 *  NcMarkInv ---
 *      Invert file marking.
 */
static void
NcMarkInv(void)
{
    NcMarkMultiple(MARK_INVERT);
}


/*
 *  NcMark ---
 *      Toggle file mark of current file
 */
static int
NcMark(void)
{
    int base;

    nclinebuf = read();
    base = index(nclinebuf, '\t');              /* delimiter */
    if (substr(nclinebuf, TYPEPOS(base), 1) != "F") {
        return (-1);
    }
    delete_char();
    if (substr(nclinebuf,1,1) == MARKER) {
        insert(" ");
    } else {
        insert(MARKER);
    }
    beginning_of_line();
    NcDown();
    return (0);
}


/*
 *  NcMarkMultiple ---
 *      Multiple file marking engine
 */
static void
NcMarkMultiple( int mark_typ )
{
    list result;
    int base;

    result[0] ="";
    result = field_list((mark_typ == MARK_SELECT ? "Select" :
                    (mark_typ == MARK_UNSELECT ? "Unselect" : "Invert")),
                result, ncpatlist, TRUE);

    if (result[0] == "") {
        NcPrompt();

    } else {
        int line_no, line;
        string pat;

        pat = result[0];
        message("Marking specified files... (%s)", pat);

        inq_position(line);
        line_no = nclineno;
        while (line_no) {
            move_abs(line_no, 1);
            nclinebuf = read();
            base = index(nclinebuf, '\t');      /* delimiter */

            if (substr(nclinebuf, TYPEPOS(base), 1) == "F" &&
                    re_search(NULL, pat, ncfilebuf) > 0) {
                message(".. %s", trim(substr(nclinebuf, NAMEPOS(base))));

                delete_char();
                switch (mark_typ) {
                case MARK_INVERT:
                    if (substr(nclinebuf,1,1) == MARKER) {
                        insert(" ");
                    } else {
                        insert(MARKER);
                    }
                    break;
                case MARK_SELECT:
                    insert(MARKER);
                    break;
                case MARK_UNSELECT:
                    insert(" ");
                    break;
                }
                beginning_of_line();
            }
            --line_no;
        }
        NcPosition(line);
    }
    NcPrompt();
}


/*
 *  Ncenter ---
 *      Process <Enter>.
 */
static void
NcEnter(void)
{
    int base;

    nclinebuf = read();
    base = index(nclinebuf, '\t');              /* delimiter */

    if (substr(nclinebuf, TYPEPOS(base), 1) == "D") { /* Directory selection */
        ncfilebuf = trim(substr(nclinebuf, NAMEPOS(base)));
        if (ncfilebuf == "..") {
            NcCd(DIR_UP);

        } else {
            NcCd(DIR_DOWN + ncfilebuf);
        }
                                                /* File selection */
    } else if (substr(nclinebuf, TYPEPOS(base), 1) == "F") {
        ncfilebuf = "./" + trim(substr(nclinebuf, NAMEPOS(base)));
        exit();
    }
}


/*
 *  NcCd ---
 *      DIR selection.
 */
static void
NcCd(~ string)
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
    ncselpattern = "";
    NcDirectory(cwd);

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
    NcPosition(pos);
    NcPrompt();
}


/*
 *  NcDirectory ---
 *      Read the current.
 */
static void
NcDirectory(string cwd)
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

    nclineno = 0;                               /* line count */
    clear_buffer();
    for (pass = 1; pass <= 2; pass++) {
        int start = nclineno;                   /* first line-index of section 0... */

        if (ncsortorder == "Unsorted" && 2 == pass) {
            break;                              /* unsorted is single pass */
        }

        file_pattern(WILD);
        while (find_file(name, bytes, mtime, NULL, mode)) {
            if  (S_IFDIR & mode) {
                /*
                 *  Directories
                 */
                if ((ncsortorder != "Unsorted" && 2 == pass) ||
                        name == "." ) {         /* skip on 2nd sort pass */
                    continue;                   /* or if '.' directory */
                }

                sprintf(buffer, " /%-*.*W|  <DIR>|", ncfwidth, ncfwidth, name);

            } else {
                /*
                 *  Files etc
                 */
                if (ncsortorder != "Unsorted" && 1 == pass) {
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
                    substr(mode_string(mode, cwd+"/"+name, TRUE), 1, 1), ncfwidth, ncfwidth, name, size);
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
            ++nclineno;
        }

        /*
         *  Sort current section
         *      TODO - sort order
         */
        if (nclineno > start) {                 /* sort section to end-of-buffer */
            delete_line();
            sort_buffer(NULL, "::sort_versions", start + 1);
        }
    }

    inq_screen_size(NULL, lines);               /* pad image */
    sprintf(buffer, "  %-*s|%7s|%10s|%12s\n", ncfwidth, "", "", "", "");
    while (lines-- > 0) {
        insert(buffer);
    }

    top_of_buffer();
    message("done...");
}


static int
sort_versions(int l1, string s1, int l2, string s2)
{
    UNUSED(l1, l2);
    return strverscmp(substr(s1, 3), substr(s2, 3));
}


/*
 *  NcSort ---
 *      This routine is called whenever the user presses the <F9>, "sort".
 */
static void
NcSort(...)
{
    list result;

    result[0] = ncsortorder;
    result = field_list("Sort Order", result, ncsortlist, TRUE);
    if (result[0] != "" && result[0] !=  ncsortorder) {
        ncsortorder = result[0];
        NcCd();
    }
}

/*end*/
