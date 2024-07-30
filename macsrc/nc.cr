/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: nc.cr,v 1.37 2024/07/29 16:15:54 cvsuser Exp $
 * Norton Commander (NC)/Midnight Commander style directory services
 *
 * TODO: retain marked/position post command.
 */

#include "grief.h"
#include "dialog.h"                             /* dialog manager */

#if defined(DEBUG)
#define Debug(x)            debug x;            /* local debug diag's */
#else
#define Debug(x)
#endif

/* Constants */
#define SYNTAX              "nc_coloriser"

#define FORWARD_SLASH       '/'
#define BACKWARD_SLASH      '\\'
#define WILD                "?*"
#define SMINCOLUMNS         62                  /* screen width */
#define SMAXCOLUMNS         142
#define SFILECOLUMNS        (nc_fwidth)
#define SINFOCOLUMNS        38
#define TYPEPOS(__base)     (__base + 1)

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
int                         mc(~ string);
int                         nc(~ string);

static void                 NcSize(void);
static string               NcParts(string line, int &type, int &size, int &mtime);
static void                 NcCd(~ string);
static void                 NcRead(string cwd);
static void                 NcMessage(void);
static void                 NcSpace(void);
static void                 NcBack(void);
static void                 NcMatch(string);
static int                  NcNext(string fname, int dir);
static int                  NcSearch(int next, string pattern);

static void                 NcRename(void);
static void                 NcMkdir(void);
static void                 NcChdir(void);
static void                 NcDelete(void);
static void                 NcUpdir(void);
static void                 NcEscape(void);
static void                 NcAction(void);

static int                  NcPosition(int line);
static void                 NcDiagnostics(void);
static void                 NcPrompt(void);

static void                 NcHome(void);
static void                 NcUp(void);
static void                 NcDown(void);
static void                 NcEnd(void);
static void                 NcPgup(void);
static void                 NcPgdn(void);

static int                  NcToggle(void);
static void                 NcSelect(void);
static void                 NcUnselect(void);
static void                 NcInverse(void);
static void                 NcMultiple(int);

static void                 NcSort(~ string);
#endif /*__PROTOTYPES__*/

static string               ncsortorder = "Name";
static int                  nckeymap;           /* keyboard map */

static int                  ncrows, nccols;     /* screen rows/cols */
static int                  ncwidth, ncfwidth;

static int                  nclineno;           /* current line count */
static int                  ncactiveno;         /* selected count */
static string               ncexitname;         /* file buffer */
static string               ncpattern;          /* selection pattern */

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

    assign_to_key("<Enter>",        "::NcEnter");
    assign_to_key("<Backspace>",    "::NcBack");
    assign_to_key("<Space>",        "::NcSpace");
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
    assign_to_key("<F6>",           "::NcRename");
    assign_to_key("<F7>",           "::NcMkdir");
    assign_to_key("<F8>",           "::NcDelete");
    assign_to_key("<Del>",          "::NcDelete");
    assign_to_key("<F9>",           "::NcSort");
    assign_to_key("<F10>",          "::NcAction");
    assign_to_key("<Esc>",          "::NcEscape");
    assign_to_key("<Ctrl-D>",       "::NcChdir");
    assign_to_key("<Ctrl-R>",       "::NcCd");
    assign_to_key("<Ctrl-PgUp>",    "::NcUpdir");
    assign_to_key("<Ctrl-F3>",      "::NcSort Name");
    assign_to_key("<Ctrl-F4>",      "::NcSort Extension");
    assign_to_key("<Ctrl-F5>",      "::NcSort Time");
    assign_to_key("<Ctrl-F6>",      "::NcSort Size");
    assign_to_key("<Ctrl-F7>",      "::NcSort Unsorted");
#if defined(MSDOS)
    assign_to_key("<Alt-F1>",       "::NcDrive");
#endif
    assign_to_key("<Alt-F>",        "ff");
    assign_to_key("<Alt-F10>",      "::NcTree");
    assign_to_key("<Ins>",          "::NcToggle");
    assign_to_key("<Keypad-Plus>",  "::NcSelect");
    assign_to_key("<Keypad-Minus>", "::NcUnselect");
    assign_to_key("<Keypad-Star>",  "::NcInverse");

    nckeymap = inq_keyboard();
    keyboard_pop(1);                            /* save keyboard */
    NcSize();
}


/*
 *  mc ---
 *      Directory services (NC/Midnight commander style).
 */
int
mc(~ string)
{
    string dir;
    get_parm(0, dir);
    nc(dir);
}


/*
 *  nc ---
 *      Directory services (NC/Midnight commander style).
 */
int
nc(~ string)
{
    string dir, cwd;                            /* saved directory */
    int old_buffer, old_window;
    int new_buffer = -1;
    int dir_buffer;

    /* Initialise directory */
    old_buffer = inq_buffer();
    old_window = inq_window();
    if ((dir_buffer = create_buffer("nc", NULL, 1)) < 0) {
        return -1;
    }

    getwd(NULL, cwd);                           /* current working dir */
    if (! get_parm(0, dir) || 0 == strlen(dir) || dir == ".")
        dir = cwd;

    NcSize();
    set_buffer(dir_buffer);
    attach_syntax(SYNTAX);
    set_buffer_type(NULL, BFTYP_UTF8);
    NcRead(dir);

    create_window(nccols, ncrows + 4, nccols + ncwidth + 30, 3,
            "<F1> Help, <Ins/+/-/Enter> select, <F10/Esc> action/exit");
    attach_buffer(dir_buffer);
    NcPosition(1);
    ncpattern = "";

    /* User process */
    keyboard_push(nckeymap);
    register_macro(0, "::NcBadkey", 1);         /* local Bad key */
    process();
    unregister_macro(0, "::NcBadkey", 1);
    keyboard_pop(1);                            /* pop, but retain */
    delete_window();

    /* Load selected files */
    if (strlen(ncexitname)) {                    /* exit condition */
        if (ncexitname == MARKER) {
            /* Load all marked files */
            string name;
            int type, size, mtime;

            top_of_buffer();
            while (search_fwd("<\\c" + MARKER)) {
                name = NcParts(read(), type, size, mtime);
                ncexitname = "./" + name;
                down();

                message("Loading \"%s\" for editing...", ncexitname);
                if (edit_file(ncexitname) > 0) {
                    if (new_buffer == -1) {     /* first */
                        new_buffer = inq_buffer();
                    }
                }
                set_buffer(dir_buffer);         /* for next search */
            }
        } else {
            /* Load specified file */
            message("Loading \"%s\" for editing...", ncexitname);
            if (edit_file(ncexitname) > 0) {
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
NcSize(void)
{
    inq_screen_size(ncrows, nccols);
    if (nccols > SMINCOLUMNS + 2) {
        ncwidth = (nccols / 3) * 2;             /* 2/3 width */
        if (ncwidth >SMAXCOLUMNS) {
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


static string
NcParts(string line, int &type, int &size, int &mtime)
{
    string name;
    sscanf(substr(line, index(line, '\t') + 1), "%c|%[^/]/%d/%d", type, name, size, mtime);
    return trim(name);
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
    if (c == "/" || c == "\\") {
        NcCd("/");
    } else {
        NcMatch(lower(c));
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
 *  NcMessage ---
 *      Display the current selected file name.
 */
static void
NcMessage(void)
{
    message("File: %s", ncpattern);
}


/*
 *  NcSpace ---
 *      Called whenever the user presses a <Space> key.
 *      Finds the next file name match ensuring the user does not go past the end of the buffer.
 */
static void
NcSpace(void)
{
    if (ncpattern == "") {
        if (NcToggle() != 0) {
            NcDown();
        }
    } else {
        if (NcNext(ncpattern, SEL_NEXT) == -1) {
            NcNext(ncpattern, SEL_TOP);
        }
    }
}


/*
 *  NcBack ---
 *      Called whenever the user presses a <Backspace> key.
 *      Removes the last key from the buffer search filename.
 */
static void
NcBack(void)
{
    int len;

    if ((len = strlen(ncpattern)) > 0) {
        ncpattern = substr(ncpattern, 1, len - 1);
        if (strlen(ncpattern) == 0) {
            NcPosition(1);
        } else {
            if (NcNext(ncpattern, SEL_TOP) == -1) {
                beep();
            }
        }
        NcMessage();
    }
}


/*
 *  NcMatch ---
 *      Select engine.
 */
static void
NcMatch(string new_key)
{
    string new_pattern;

    new_pattern = ncpattern;
    new_pattern += new_key;
    if (strlen(new_pattern) == 1 || NcNext(new_pattern, SEL_NEXT) == -1) {
        if (NcNext(new_pattern, SEL_TOP) == -1) {
            beep();
            return;
        }
    }
    ncpattern = new_pattern;
    NcMessage();
}


/*
 *  NcNext ---
 *      Finds the next matching file name.
 */
static int
NcNext(string fname, int dir)
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

    if (NcSearch(dir, "<" + MARKER + "|  "+fname) <= 0) {
        /* not found, restore position */
        restore_position();
        return -1;
    }
    restore_position(0);                        /* found, eat position. */
    return NcPosition(0);
}


/*
 *  NcSearch ---
 *      Searches forward and back depending on first parameter if that parameter is not
 *      zero it searches forward else back.
 */
static int
NcSearch(int next, string pattern)
{
    return (next ? search_fwd(pattern, 1, 0, 0) : search_back(pattern, 1, 0, 0));
}


#if defined(MSDOS)
/*
 *  NcDrive ---
 *      Change-drive
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
 *  NcRename ---
 *      Rename directory/file command
 */
static void
NcRename(void)
{
    string name;
    int type, size, mtime;
    list result;

    name = NcParts(read(), type, size, mtime);
    result[0] = name;
    result = field_list("Rename", result, ncdirlist, TRUE);
    if (result[0] == "") {
        NcPrompt();
    } else {
        message("Renaming \"%s\" to \"%s\"...", name, result[0]);
        if (rename(name, result[0]) == -1) {
            error("Unable to rename \"%s\" : %d (%s)", name, errno, strerror(errno));
        } else {
            NcCd();
        }
    }
}

/*
 *  NcMkdir ---
 *      Make directory command.
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
 *      Change-directory command.
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
 *      Process <Ctrl-PgUp> "up directory" key request.
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
 *      Directory tree command.
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
 *      Delete file/directory command.
 */
static void
NcDelete(void)
{
    string linebuf, name, answer;
    int line, type, size, mtime;

    inq_position(line);
    raise_anchor();
    top_of_buffer();
    if (search_fwd("<\\c" + MARKER)) {
        answer = "?";
        while (answer != "Y" && answer != "N") {
            if (answer != "?") {
                beep();
            }
            answer = "";
            get_parm(NULL, answer, "Delete 'tagged' files [yn]? ", 1);
            if (answer == "") {
                answer = "N";
            } else if (answer == "?") {
                beep();
            } else {
                answer = upper(answer);
            }
        }

        if (answer == "Y") {
            top_of_buffer();
            while (search_fwd("<\\c" + MARKER)) {
                beginning_of_line();
                linebuf = read();
                down();

                name = NcParts(linebuf, type, size, mtime);
                if (type == 'D') {              /* directory */
                    if (name != "..") {
                        message("Deleting Directory \"%s\"...", name);
                        if (rmdir(name) == -1) {
                            error("Unable to rmdir \"%s\" : %d (%s)", name, errno, strerror(errno));
                        }
                    }

                } else if (type == 'F') {       /* file */
                    name = "./" + name;
                    message("Deleting \"%s\"...", name);
                    if (remove(name) == -1) {
                        error("Unable to delete \"%s\" : %d (%s) - Press any key", name, errno, strerror(errno));
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
        linebuf = read();
        beginning_of_line();

        answer = "?";
        while (answer != "Y" && answer != "N") {
            if (answer != "?")
                beep();
            answer = "";
            get_parm(NULL, answer, "Delete 'highlighted' file [yn]? ", 1);
            if (answer == "") {
                answer = "N";
            } else if (answer == "?") {
                beep();
            } else {
                answer = upper(answer);
            }
        }

        if (answer == "Y") {
            name = NcParts(linebuf, type, size, mtime);
            if (type == 'D') {                  /* directory */
                if (name != "..") {
                    message("Deleting Directory \"%s\"...", name);
                    if (rmdir(name) == -1) {
                        error("Unable to rmdir \"%s\" : %d (%s)", name, errno, strerror(errno));
                    }
                }

            } else if (type == 'F') {           /* file */
                name = "./" + name;
                message("Deleting \"%s\"...", name);
                if (remove(name) == -1) {
                    error("Unable to delete \"%s\" : %d (%s) - Press any key", name, errno, strerror(errno));
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
 *  NcEscape/ESC ---
 *      Escape, without action.
 */
static void
NcEscape(void)
{
    ncexitname = "";
    exit();
}


/*
 *  NcEdit/F4 ---
 *      Edit command.
 */
static void
NcEdit(void)
{
    string linebuf, name;
    int line, size, type, mtime;

    inq_position(line);
    linebuf = read();
    raise_anchor();
    top_of_buffer();

    if (search_fwd("<\\c" + MARKER)) {
        ncexitname = MARKER;

    } else {
        name = NcParts(linebuf, type, size, mtime);
        if (type == 'D') {                      /* directory */
            error("Cannot edit directory, press any key...");
            beep();
            NcPosition(line);
            return;

        } else if (type == 'F') {               /* file */
            ncexitname = "./" + name;

        } else {
            NcPrompt();
            return;
        }
    }
    exit();                                     /* exit 'nc' */
}


/*
 *  NcExit/F10 ---
 *      Exit.
 */
static void
NcExit(void)
{
    if (ncactiveno) {
        ncexitname = MARKER;
        exit();
    } else {
        beep();
    }
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
//  NcDiagnostics();
    return line;
}


static void
NcDiagnostics(void)
{
    string linebuf, name;
    int type, size, mtime;

    linebuf = read();
    name = NcParts(linebuf, type, size, mtime);
    message("type=%c,name=%s,size=%d,mtime=%d", type, name, size, mtime);
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
 *  NcSelect ---
 *      Multiple file marking.
 */
static void
NcSelect(void)
{
    NcMultiple(MARK_SELECT);
}


/*
 *  NcUnselect ---
 *      Multiple file unmarking.
 */
static void
NcUnselect(void)
{
    NcMultiple(MARK_UNSELECT);
}


/*
 *  NcInverse ---
 *      Invert file marking.
 */
static void
NcInverse(void)
{
    NcMultiple(MARK_INVERT);
}


/*
 *  NcToggle ---
 *      Toggle file mark of current file
 */
static int
NcToggle(void)
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
        --ncactiveno;
        insert(" ");
    } else {
        insert(MARKER);
        ++ncactiveno;
    }
    beginning_of_line();
    NcDown();
    return (0);
}


/*
 *  NcMultiple ---
 *      Multiple file marking engine
 */
static void
NcMultiple(int mark_type)
{
    list result;

    result[0] = "";
    result = field_list((mark_type == MARK_SELECT ? "Select" :
                    (mark_type == MARK_UNSELECT ? "Unselect" : "Invert")),
                result, ncpatlist, TRUE);

    if (result[0] == "") {
        NcPrompt();

    } else {
        int line_no, line;
        string linebuf, pat, name;
        int type, size, mtime;

        pat = result[0];
        message("Marking specified files... (%s)", pat);

        inq_position(line);
        line_no = nclineno;
        while (line_no) {
            move_abs(line_no, 1);
            linebuf = read();
            name = NcParts(linebuf, type, size, mtime);
            if (type == 'F')
            {
                if (re_search(NULL, pat, name) > 0) {
                    message(".. %s", name);
                    switch (mark_type) {
                    case MARK_INVERT:
                        delete_char();
                        if (substr(linebuf,1,1) == MARKER) {
                            --ncactiveno;
                            insert(" ");
                        } else {
                            ++ncactiveno;
                            insert(MARKER);
                        }
                        break;
                    case MARK_SELECT:
                        if (substr(linebuf,1,1) != MARKER) {
                            delete_char();
                            ++ncactiveno;
                            insert(MARKER);
                        }
                        break;
                    case MARK_UNSELECT:
                        if (substr(linebuf,1,1) == MARKER) {
                            delete_char();
                            --ncactiveno;
                            insert(" ");
                        }
                        break;
                    }
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
    string linebuf, name;
    int type, size, mtime;

    linebuf = read();
    name = NcParts(linebuf, type, size, mtime);
    if (type == 'D') {                          /* directory */
        if (name == "..") {
            NcCd(DIR_UP);
        } else {
            NcCd(DIR_DOWN + name);
        }

    } else if (type == 'F') {                   /* file */
        if (0 == ncactiveno) {
            ncexitname = "./" + name;
            exit();
        } else {
            NcToggle();
        }
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
    ncpattern = "";
    NcRead(cwd);

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
 *  NcRead ---
 *      Read the current directory.
 */
static void
NcRead(string cwd)
{
    int cyear, year, day, hour, min;
    int bytes, mtime, mode;
    int pass, dirs, lines;
    string name, mon, size;
    string buffer;

    /*
     *    read directory
     */
    date(cyear, NULL, NULL);
    message("Reading disk directory...");

    nclineno = 0;                               /* line count */
    ncactiveno = 0;
    clear_buffer();
    for (pass = 1; pass <= 2; pass++) {
        if (ncsortorder == "Unsorted" && 2 == pass) {
            break;                              /* unsorted is single pass */
        }

        file_pattern(WILD);
        while (find_file(name, bytes, mtime, NULL, mode)) {
            if (S_IFDIR & mode) {
                /*
                 *  Directories
                 */
                if ((ncsortorder != "Unsorted" && 2 == pass) ||
                        name == "." ) {         /* skip on 2nd sort pass */
                    continue;                   /* or if '.' directory */
                }
                sprintf(buffer, " /%-*.*W|  <DIR>|", ncfwidth, ncfwidth, name);
                bytes = 0;
                ++dirs;

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
                    substr(mode_string(mode, cwd + "/" + name, TRUE), 1, 1), ncfwidth, ncfwidth, name, size);
            }

            buffer += mode_string(mode);        /* decode mode */

            localtime(mtime, year, NULL, day, mon, NULL, hour, min, NULL);
            if (cyear == year) {
                sprintf(buffer, "%s|%s %2d %02d:%02d ", buffer, substr(mon,1,3), day, hour, min);
            } else {
                sprintf(buffer, "%s|%s %2d  %04d ", buffer, substr(mon,1,3), day, year);
            }

            insertf("%s\t%c|%s/%d/%d\n",        /* <delimiter><type>|<name>/<size>/<mtime><nl> */
                buffer, ((mode & S_IFDIR) ? 'D' : 'F'), name, bytes, mtime);
            ++nclineno;
        }

        delete_line();
        if (ncsortorder != "Unsorted") {
            string sortby;
            if (dirs > 1)
                sortby = "::sort_names";
            if (ncsortorder == "Name") {
                sortby = "::sort_names";
            } else if (ncsortorder == "Extension") {
                sortby = "::sort_extensions";
            } else if (ncsortorder == "Time") {
                sortby = "::sort_times";
            } else if (ncsortorder == "Size") {
                sortby = "::sort_sizes";
            }
            if (sortby) {
                sort_buffer(NULL, sortby, 1, dirs);
                sort_buffer(NULL, sortby, dirs + 1);
            }
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
sort_names(int l1, string s1, int l2, string s2)
{
    int type, size, mtime;

    UNUSED(l1, l2);
    s1 = NcParts(s1, type, size, mtime);
    s2 = NcParts(s2, type, size, mtime);
    return strfilecmp(s1, s2);
}


static int
sort_extensions(int l1, string s1, int l2, string s2)
{
    int type, size, mtime, ext;

    UNUSED(l1, l2);
    s1 = NcParts(s1, type, size, mtime);
    ext = rindex(s1, '.');
    if (ext) s1 = substr(s1, ext + 1);

    s2 = NcParts(s2, type, size, mtime);
    ext = rindex(s2, '.');
    if (ext) s2 = substr(s2, ext + 1);

    return strfilecmp(s1, s2);
}


static int
sort_times(int l1, string s1, int l2, string s2)
{
    int type, size, mtime1, mtime2;

    UNUSED(l1, l2);
    NcParts(s1, type, size, mtime1);
    NcParts(s2, type, size, mtime2);
    return mtime1 <=> mtime2;
}


static int
sort_sizes(int l1, string s1, int l2, string s2)
{
    int type, size1, size2, mtime;

    UNUSED(l1, l2);
    NcParts(s1, type, size1, mtime);
    NcParts(s2, type, size2, mtime);
    return size1 <=> size2;
}


/*
 *  <sort>
 */
static void
NcSort(~ string)
{
    string sortorder;

    if (! get_parm(0, sortorder)) {
        list result;

        result[0] = ncsortorder;
        result = field_list("Sort Order", result, ncsortlist, TRUE);
        if (result[0] != "" && result[0] != ncsortorder) {
            sortorder = result[0];
        }
    }

    if (sortorder) {
        message("sorting by %s...", sortorder);
        ncsortorder = sortorder;
        NcCd();
    }
}

/*end*/
