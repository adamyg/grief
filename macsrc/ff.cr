/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: ff.cr,v 1.39 2020/05/04 20:11:58 cvsuser Exp $
 * Find find and other assorted directory/file searches.
 *
 *  ff      File find.
 *  dir     Directory list.
 *  tree    Directory tree.
 *  treecd  Change directory.
 *  bs      Buffer search.
 *  find    Current buffer search.
 *  ts      Text search.
 *
 */

#include "grief.h"
#include "dialog.h"

#if !(defined(OS2) || defined(MSDOS)) || defined(HPFS)
#define TREE_FILE           ".grtree"
#else
#define TREE_FILE           "_grtree"
#endif
#define TREE_WIDTH          100

#define FORWARD_SLASH       '/'
#define BACKWARD_SLASH      '\\'
#define WILD                "?*"
#define CURRENTDIR          "."

#define FF_MINWIDTH         68

#if defined(DEBUG)
#define Debug(x)            message x ;         /* local debug diag's */
#else
#define Debug(x)
#endif

/* Flags */
#define F_SPECIALS          0x0001
#define F_RECURSIVE         0x0002
#define F_DIRTREE           0x0004
#define F_AUTOLOAD          0x0008
#define F_IGNORECASE        0x0010
#define F_BINARY            0x0020
#define F_NOBINARY          0x0040
#define F_SELECTONE         0x0080              /* find, single selection */

#define F_BUFFER            0x0100
#define F_SYSBUFFERS        0x0200
#define F_PIPE              0x0400
#define F_DIR               0x0800
#define F_CURRBUFFER        0x1000

/* Globals */
static string               ffslashc;
static int                  ffrescan;
static int                  tstotals;
static int                  ffwidth;

static int                  tskeep;
static int                  tsbuf = -1;
static int                  tsline = -1;

/* Functions */
#if defined(__PROTOTYPES__)
static void                 tsmain(int flags, string pattern, string file, string caller);
static int                  tsedit(int direction, int line);
static int                  tslist(void);
static void                 tsnext(void);
static void                 tsprev(void);

int                         _ff_act(int type);
static string               _ff(string rootdir, string fmask, string pattern, int flags);
static string               _ff2(int dirtree, int dirmask);

static void                 setwidth(string pattern);
static string               sysslash(void);
static string               shortname(string name, int add);
static int                  validdir(string dir, int restore);
static string               searchfile(string file);
static string               searchbuffer(int dest);
static string               readline(string file);
static int                  aborted_pressed(void);
static string               getpathname(string);
#endif


/*
 *  ff ---
 *    Findfile, user interface
 *
 *       ff [pattern] [file-path] [flags]
 *    or ff <file-name
 */
string
ff(~string, ~string, ~int)
{
    string  alldirs, allfiles, curdir, dir, file;
    int     old_buf = inq_buffer(),
            ff_buf, flags,
            pos;

    /* Process the callers parameters */
    if (! get_parm(0, alldirs, "Directory: ", NULL, CURRENTDIR)) {
        return "";
    }
    if (! get_parm(1, allfiles, "Filename: ", NULL, WILD)) {
        return "";
    }
    if (! get_parm(2, flags)) {
        flags = F_SPECIALS|F_RECURSIVE;
    }

    /* Generate the file list */
    if ((ff_buf = create_buffer((flags & F_DIR ? "Directory" : "File Find"), NULL, 1)) == -1) {
        return "";
    }
    set_buffer(ff_buf);
    tabs(7, 13);

    if (alldirs == "<") {
        /* Use output of a previous file-find */
        if (read_file(allfiles) <= 0) {
            error("%s: File not found.", allfiles);
            set_buffer(old_buf);
            delete_buffer(ff_buf);
            return "";
        }

    } else {
        /* Save current working directory */
        getwd("", curdir);

        /* Generate using the current pattern */
        while (alldirs != "") {                 /* For each dir contained within the 'alldirs' parameter */
            if (pos = index(alldirs, ",")) {
                dir = substr(alldirs, 1, pos - 1);
                alldirs = substr(alldirs, pos + 1);
            } else {
                dir = alldirs;
                alldirs = "";
            }
            dir = getpathname(dir);

            if (validdir(dir, 0)) {             /* For each pattern contained within the 'allfiles' parameter */
                do {
                    if (pos = index(allfiles, ",")) {
                        file = trim(substr(allfiles, 1, pos - 1));
                        allfiles = trim(substr(allfiles, pos + 1));
                    } else {
                        file = trim(allfiles);
                        allfiles = "";
                    }
                    _ff(dir, (file == "" ? WILD : file), "", flags);
                } while (allfiles != "");
            }
        }

        /* Restore current working directory */
        cd(curdir);
    }
    delete_line();

    /* Display the result */
    if (inq_lines(ff_buf) > 0) {
        int slines, scols;

        set_buffer(old_buf);
        inq_screen_size(slines, scols);

        _dialog_menu(ffwidth, (slines/3)*2, 0, 0,
            "File-List", "<Enter> to select, <F10/Esc> to exit>",
               NULL, ff_buf, "_ff_act", TRUE);

        set_buffer(ff_buf);
        pos = (inq_called() == "");             /* Called from command line ? */
        allfiles = alldirs = "";

        end_of_buffer();
        do {
            beginning_of_line();
            if (read(1) == "*") {
                file = substr(rtrim(read()), 2);
                file = substr(file, rindex(file, ";") + 1);
                if (index("\\/", substr(file, strlen(file), 1))) {
                    /* directory */
                    if (alldirs == "") {
                        if (! get_parm(NULL, alldirs, "Command: ", NULL, "dir %s *.*")) {
                            top_of_buffer();
                        }
                    }
                    sprintf(dir, alldirs, substr(file, 1, strlen(file)-1));
                } else {
                    /* file name */
                    if (allfiles == "") {
                        if (! get_parm(NULL, allfiles, "Command: ", NULL, "edit_file %s")) {
                            top_of_buffer();
                        }
                    }
                    sprintf(dir, allfiles, file);
                }

                if (pos) {
                    message(dir);
                    set_buffer(old_buf);
                    save_position();
                    set_calling_name("");
                    execute_macro(dir);
                    set_calling_name("ff");
                    restore_position();
                    old_buf = inq_buffer();
                    set_buffer(ff_buf);
                }
            }
        } while (up());
        message("");

    } else {
        message("No file found.");
    }
    set_buffer(old_buf);
    delete_buffer(ff_buf);
    return (file);
}


/*
 *  dir ---
 *      Dir list, performs 'user specified' command on selection list
 */
string
dir(~string, ~string)
{
    string dir, file;
    int flags = F_DIR;
    int param = 0;

    while (1) {
        if (! get_parm(param++, dir)) {         // directory
            dir = CURRENTDIR;
            break;
        }

        if (dir == "/a" || dir == "-a")  {
            flags |= F_SPECIALS;                // all (include specials)

        } else if (dir == "/r" || dir == "-r") {
            flags |= F_RECURSIVE;               // recursive

        } else if (dir == "/i" || dir == "-i") {
            flags |= F_IGNORECASE;              // ignore case

        } else {
            break;
        }
    }

    if (! get_parm(param++, file)) {            // pattern
        file = WILD;
    }

    set_calling_name(inq_called());
    return ff(dir, file, flags);
}


/*
 *  tree ---
 *      Directory tree
 */
string
tree(~string)
{
    string  directory, tmpstr;
    int     old_buf = inq_buffer(),
               msg_level = inq_msg_level();
    int     tree_buf, picked_no;

    /* Retrieve base directory */
    if (! get_parm(0, directory, "Directory: ", NULL, CURRENTDIR)) {
        return "";
    }

    /* Open the Dir-Tree file */
    if (directory == "~") {
        directory = inq_home();
    }
    if (! validdir(directory, 2)) {
        message("tree: Invalid directory.");
        return "";
    }

    tmpstr = directory;
    if (rindex(tmpstr, sysslash()) != strlen(tmpstr)) {
        tmpstr += sysslash();
    }

    if ((tree_buf =
            create_buffer("Directory Tree", tmpstr + TREE_FILE, FALSE)) < 0) {
        message("tree: Unable to access directory buffer.");
        return "";
    }

    set_buffer(tree_buf);
    set_buffer_flags(tree_buf, BF_SYSBUF|BF_NO_UNDO, ~BF_BACKUP);

    if (inq_lines() <= 1) ffrescan = 1;         /* new buffer; scan */

    ffwidth = TREE_WIDTH;

    while (1) {                                 /* Generate the file list */
        if (ffrescan) {
            /* Purge the current buffer */
            top_of_buffer(); drop_anchor();
            end_of_buffer(); delete_block();
            write_buffer();

            /* Insert BASE directory */
            getwd("", tmpstr);
            cd(directory);
            getwd("", directory);
            insert(directory);
            move_abs(0, TREE_WIDTH);
            insert(";" + directory);

            /* Create the TREE from the base directory */
                                                /* recursive, tree */
            _ff(directory, "", "", F_RECURSIVE|F_SPECIALS|F_DIRTREE);
            cd(tmpstr);
            write_buffer();                     /* flush result */
            message("");
            ffrescan = 0;
        }

        /* Display the tree */
        {
            int sline, col, width;

            set_buffer(old_buf);
            inq_screen_size(sline, col);

            if ((width = col) > TREE_WIDTH) {
                col = (col - TREE_WIDTH)/2;     /* center image */
                width = TREE_WIDTH;
            } else {
                width = col - 4, col = 1;       /* maximise image */
            }
                                                /* Left,bottom,right,top */
            _dialog_menu(col, (sline / 3) * 2, col+width, 3,
                "DirTree", "<Esc>, <Enter>, <F2> Re-Scan, <F5> Search",
                NULL, tree_buf, "_tree_act", TRUE, picked_no, tmpstr);

            set_buffer(tree_buf);
        }

        /* Why did the user exit */
        if (ffrescan == 0) {
            if (picked_no == 0) {
                directory = "";
            } else {
                top_of_buffer();
                move_abs(0, TREE_WIDTH + 1);
                directory = trim(read());
                directory += substr(tmpstr, index(tmpstr, ";")+1);
            }
            break;
        }
    }

    set_buffer(old_buf);
    delete_buffer(tree_buf);
    set_msg_level(msg_level);
    return directory;
}


/*
 *  treecd ---
 *      Change directory, using directory tree
 */
void
treecd(~string)
{
    string dir;

    if (get_parm(0, dir, "Directory: ", NULL, CURRENTDIR)) {
        dir = tree(dir);
        message(dir);
        cd(dir);
    }
}


/*
 *  bs --- Buffer search
 *
 *      bs [options] pattern
 */
string
bs(~string, ...)
{
    string opt, pattern, file;
    int flags, param;

    flags = F_BUFFER;                           /* buffer search */
    param = 0;

    while (1) {
        if (! get_parm(param++, opt, "Pattern/Options: ")) {
            return "";
        }

        if (opt == "--") {                      /* getopt style (--) */
            if (! get_parm(param++, pattern, "Pattern: ")) {
                return "";
            }
            break;

        } else if (opt == "/a" || opt == "-a")  {
            file = "*.*";                       /* all buffers */

        } else if (opt == "/s" || opt == "-s") {
            flags |= F_SYSBUFFERS;              /* system buffers */

        } else if (opt == "/i" || opt == "-i") {
            flags |= F_IGNORECASE;              /* ignore case */

        } else if (opt == "/1" || opt == "-1") {
            flags |= F_SELECTONE;               /* single select */
            break;

        } else {    /*otherwise pattern */
            pattern = opt;
            break;
        }
    }

    if ("" == file) {
        if (! get_parm(/*1+*/param++, file, "Filename: ", NULL, WILD)) {
            return "";
        }
    }

    tsmain(flags, pattern, file, inq_called());
}


/*
 *  find --- Find text within the current buffer
 *
 *       find [options] pattern
 */
string
find(~string, ...)
{
    string opt, pattern, file;
    int flags, param;

    flags = F_CURRBUFFER|F_SELECTONE;           /* buffer search/single selection */
    param = 0;

    while (1) {
        if (! get_parm(param++, opt, "Pattern/Options: ")) {
            return "";
        }

        if (opt == "--") {                      /* getopt style (--) */
            if (! get_parm(param++, pattern, "Pattern: ")) {
                return "";
            }
            break;

        } else if (opt == "/i" || opt == "-i") {
            flags |= F_IGNORECASE;              /* ignore case */

        } else {    /*otherwise pattern */
            pattern = opt;
            break;
        }
    }

    inq_names(NULL, NULL, file);                /* current buffer */

    tsmain(flags, pattern, file, inq_called());
}


/*
 *  ts ---- Text Search
 *
 *         ts [/ralib] pattern [dir/][pattern]
 *      or ts [options] pattern --
 *      or ts <[filename]
 */
string
ts(~string, ~string, ...)
{
    string opt, pattern, file;
    int flags, param;

    param = flags = 0;

    while (1) {
        if (! get_parm(param++, opt, "Pattern/Options: ")) {
            return "";
        }

        if (opt == "--") {                      /* getopt style (--) */
            if (! get_parm(param++, pattern, "Pattern: ")) {
                return "";
            }
            break;

        } else if (opt == "/a" || opt == "-a")  {
            flags |= F_SPECIALS;                /* all (include specials) */

        } else if (opt == "/r" || opt == "-r") {
            flags |= F_RECURSIVE;               /* recursive */

        } else if (opt == "/l" || opt == "-l") {
            flags |= F_AUTOLOAD;                /* autoload */

        } else if (opt == "/i" || opt == "-i") {
            flags |= F_IGNORECASE;              /* ignore case */

        } else if (opt == "/b" || opt == "-b") {
            flags |= F_BINARY;                  /* plus binary */

        } else if (opt == "/p" || opt == "-p") {
            flags |= F_PIPE;                    /* pipe */
            break;

        } else if (opt == "/1" || opt == "-1") {
            flags |= F_SELECTONE;               /* single select */
            break;

        } else {    /*otherwise pattern */
            pattern = opt;
            break;
        }
    }

    if ("" == file) {
        if (! get_parm(/*1+*/param++, file, "Filename: ", NULL, WILD)) {
            return "";
        }
    }

    if ("/r" == file && 0 == flags) {           /* allow trailing /r (old style) */
        if (! get_parm(/*1+*/param++, file, "Filename: ", NULL, WILD)) {
            return "";
        }
        flags |= F_RECURSIVE;
    }

    tsmain(flags, pattern, file, inq_called());
}


static void
tsmain(int flags, string pattern, string file, string caller)
{
    int curbuf = inq_buffer();

    /* prime search results buffer */
    tstotals = 0;
    curbuf = inq_buffer();
    if (tsbuf == -1)  {
        if ((tsbuf = create_buffer("Text Search", NULL, 1)) == -1) {
            return;
        }
    }

    set_buffer(tsbuf);
    set_buffer_flags(NULL, BF_NO_UNDO, ~BF_BACKUP);

    /* search */
    if (flags & F_PIPE) {                       /* piping search results */
        if (read_file(file) <= 0) {
            error("%s: File not found.", file);
            set_buffer(curbuf);
            delete_buffer(tsbuf);
            tsbuf = -1;
            return;
        }

    } else if (flags & (F_BUFFER|F_CURRBUFFER)) {
        string name, _ff_pattern = pattern;     /* open files (buffers) */
        int _ff_flags = flags;

        UNUSED(_ff_pattern, _ff_flags);

        clear_buffer();                         /* clear previous results */

        setwidth(pattern);                      /* setup display width */

        if (flags & F_CURRBUFFER) {
            set_buffer(curbuf);
            searchbuffer(tsbuf);

        } else {
            do {                                /* walk buffers */
                inq_names(NULL, NULL, name);

                /*
                *  Search if/
                *      a) Not the result buffer
                *      b) If not a system buffer or F_SYSBUFFERS
                *      c) 'file' matches the buffer name
                */
                if (inq_buffer() != tsbuf &&
                        ((flags & F_SYSBUFFERS) || !inq_system()) && file_match(file, name)) {
                    searchbuffer(tsbuf);
                }
                set_buffer(next_buffer(TRUE));  /* next */
            } while (inq_buffer() != tsbuf);
        }

    } else {
        string curdir, dir;
        int pos;

        getwd("", curdir);                      /* save current working directory */

        clear_buffer();                         /* clear previous results */

        if ((pos = rindex(file, "/")) == 0 && (pos = rindex(file, "\\")) == 0) {
            dir = ".";
        } else {
            dir = substr(file, 1, pos);
            file = substr(file, pos+1);
        }

        dir = getpathname(trim(dir));
        if (validdir(dir, 0)) {
            _ff(dir, (trim(file) == "" ? WILD : file), pattern, flags);
        }

        cd(curdir);                             /* restore cwd */
    }
    delete_line();

    /* Process search results */
    tskeep = 0;

    if (inq_lines(tsbuf) > 0) {
        set_buffer(curbuf);

        if (flags & F_AUTOLOAD) {
            tskeep = 1;                         /* keep results */
        } else {
            int slines, scols;
            int t_buf = tsbuf;                  /* XXX - scoping bug (fixed??) */

            inq_screen_size(slines, scols);

            if (flags & F_SELECTONE) {          /* single selection */
               _dialog_menu(ffwidth, (slines / 3) * 2, 0, 0,
                  "Search-List", "<Esc> to exit, <Enter> to select",
                        NULL, t_buf, "_ts_act_one", TRUE);
            } else {
               _dialog_menu(ffwidth, (slines / 3) * 2, 0, 0,
                  "Search-List", "<Esc/F10> to exit, <Enter> to select",
                        NULL, t_buf, "_ts_act_mul", TRUE);
            }
        }

        if (caller == "") {                     /* invoked from command line */
            int loaded = 0;

            set_buffer(tsbuf);
            end_of_buffer();

            do {
                beginning_of_line();
                if (read(1) == "*" || (flags & F_AUTOLOAD)) {
                    int buf;

                    if ((buf = tsedit(0, -1)) == 0) {
                        curbuf = inq_buffer();
                        set_buffer(tsbuf);
                        loaded++;
                    }
                }
            } while (up());

            if (loaded) {
                message("Loaded %d matches.", loaded);
            }
        }
    } else {
        message("No match found.");
    }

    /* Delete ONLY if user didn't exit using F10 */
    set_buffer(curbuf);
    attach_buffer(curbuf);

    if (tskeep == 0) {
        delete_buffer(tsbuf);
        tsbuf = -1;
    } else {
        assign_to_key("<Ctrl-N>", "::tsnext");
        assign_to_key("<Ctrl-P>", "::tsprev");
        assign_to_key("<Ctrl-B>", "::tslist");
    }
}


static int
tsedit(int direction, int line)
{
    int curbuf = inq_buffer();
    string file, pattern;
    int result = 0;
    int row = 0;

    set_buffer(tsbuf);
    if (line > 0) {                             /* read line, if a File: skip */
        move_abs(line, 1);
    }

    file = substr(rtrim(read()), 2);
    if (substr(file, 1, 6) == "File: ") {
        if (direction == 0) {                   /* ignore */
            result = -1;

        } else if (direction > 0) {             /* next */
            if (down()) {
                file = substr(rtrim(read()), 2);
            } else {
                result = -1;
            }

        } else if (direction < 0) {             /* previous */
            if (up()) {
                file = substr(rtrim(read()), 2);
            } else {
                result = -1;
            }
        }
    }

    /* decode line */
    if (result == 0) {
        int omsglevel = set_msg_level(3);

        inq_position(tsline);                   /* match line */

        row = atoi(file);
        file = substr(file, rindex(file, ";") + 1);
        search_back("< |*File: ", 1, 1);
        pattern = read();
        pattern = substr(pattern, index(pattern, "Pattern: ") + 9);
        pattern = substr(pattern, 1, strlen(pattern) - 1);
        if ("~" == substr(file, 1, 1)) {
            file = inq_home() + substr(file, 2);
        }
        set_msg_level(omsglevel);

        if (edit_file(file) != 1) {
            result = -1;
        } else {
            move_abs(row, 1);
            search_fwd(pattern, 1);
        }
    }

    if (result != 0) {
        set_buffer(curbuf);
    }

    return result;
}


static int
tslist(void)
{
    ts("<");
}


static void
tsnext(void)
{
    int result = -1;

    if (tsbuf >= 0 && tsline < inq_lines(tsbuf)) {
        result = tsedit(1, tsline+1);
    }
    if (result) {
        beep();
    }
}


static void
tsprev(void)
{
    int result = -1;

    if (tsbuf >= 0 && tsline > 2) {
        result = tsedit(-1, tsline-1);
    }
    if (result) {
        beep();
    }
}


/*
 *  _ff_act ---
 *      file find engine dialog action interface
 */
int
_ff_act(int type)
{
    string front;
    int pos;

    switch (type) {
    case DIALOG_BACK:
    case DIALOG_PICK_MENU: {
            get_parm(1, pos);
            move_abs(pos, 1);
            if (read(1) == "*") {
                front = " ";
            } else {
                front = "*";
            }
            delete_char();
            insert(front);
            beginning_of_line();
            push_back(key_to_int("<Down>"));
        }
        break;

    case DIALOG_FUNC_KEY:
        get_parm(1, pos);
        switch(pos) {
        case DIALOG_KEY_F4:                     /* <F4> Edit & Exit */
            beginning_of_line();
            delete_char();
            insert("*");
            _dialog_esc();
            return (FALSE);

        case DIALOG_KEY_F10:                    /* <F10> Exit */
            _dialog_esc();
            return (FALSE);
        }
    }
    return (TRUE);                              /* perform default action */
}


/*
 *  _tree_act ---
 *      Directory tree engine dialog action interface
 */
int
_tree_act(int type)
{
    int key;

    switch(type) {
    case DIALOG_PICK_MENU:                      /* <Enter> Select */
        _dialog_esc();
        break;

    case DIALOG_FUNC_KEY:
        get_parm(1, key);
        switch (key) {
        case DIALOG_KEY_F2:                     /* <F2> Rescan */
            ffrescan = 1;
            _dialog_esc();
            break;

        case DIALOG_KEY_F5:                     /* <F5> Search */
            _dialog_menu_search(0, TRUE);
            break;

        case DIALOG_KEY_SF5:                    /* <Shift-F5> Search-again */
            _dialog_menu_search(1, TRUE);
            break;
        }
        return (FALSE);                         /* don't perform default action */
    }
    return (TRUE);                              /* perform default action */
}


/*
 *  _ts_act_one ---
 *      Text search engine dialog action interface, single selection.
 */
int
_ts_act_one(int type)
{
    string text;
    int pos;

    switch(type) {
    case DIALOG_PICK_MENU: {                    /* <Enter>, select */
            get_parm(1, pos);
            move_abs(pos, 1);
            if (substr(text = read(5), 2, 4) != "File") {
                text = substr(text, 1, 1);
            } else {
                down();
                text = read(1);
            }
            delete_char();
            insert(text == " " ? "*" : " ");
            left();
            tskeep = 1;
            _dialog_esc();
        }
        break;
    }
    return (TRUE);                              /* perform default action */
}



/*
 *  _ts_act_mul ---
 *      Text search engine dialog action interface, multiple selection.
 */
int
_ts_act_mul(int type)
{
    string text;
    int pos;

    switch(type) {
    case DIALOG_PICK_MENU: {                    /* <Enter>, select */
            get_parm(1, pos);
            move_abs(pos, 1);
            if (substr(text = read(5), 2, 4) != "File") {
                text = substr(text, 1, 1);
            } else {
                down();
                text = read(1);
            }
            delete_char();
            insert(text == " " ? "*" : " ");
            left();
            push_back(key_to_int("<down>"));
        }
        break;

    case DIALOG_FUNC_KEY:
        get_parm(1, pos);
        switch(pos) {
        case DIALOG_KEY_F10:                    /* <F10> Exit and save */
            tskeep = 1;
            _dialog_esc();
            return (FALSE);                     /* don't perform default action */
        }
        break;
    }
    return (TRUE);                              /* perform default action */
}


/*
 *  setwidth ---
 *      set the current display width
 */
static void
setwidth(string pattern)
{
    inq_screen_size(NULL, ffwidth);

    if (pattern) {
        ffwidth -= 5;                           /* max */
    } else {
        ffwidth = (ffwidth/4)*3;                /* 75% */
    }

    if (ffwidth < FF_MINWIDTH) {
        ffwidth = FF_MINWIDTH;
    }
}


/*
 *  _ff --
 *      File find engine front end, with optional text search and
 *      tree creation
 *
 *      rootdir:        String which the root/search directory
 *
 *      fmask:          Search file pattern
 *
 *      flags:
 *          SPECIAL     Specifies whether the all files, including
 *                      specials should be matched, during file
 *                      scans (fmask required).
 *
 *          RECURSIVE   Specifies whether the search should
 *                      recurse down the directory tree.
 *
 *          DIRTREE     Specifies whether a directory tree should
 *                      be created.
 *
 *          IGNORECASE  Ignore case during pattern searches.
 *
 *      pattern:        Text search pattern (fmask required)
 */
static string
_ff(string rootdir, string fmask, string pattern, int flags)
{
    string _ff_fmask, _ff_pattern, _ff_rootdir;
    int _ff_lines, _ff_flags;

    setwidth(pattern);                          /* set display width */

    _ff_lines = 0;
    _ff_rootdir = rootdir;
    _ff_fmask = fmask;
    _ff_pattern = pattern;
    _ff_flags = flags;

    return _ff2((flags & F_DIRTREE ? 1 : 0), 0);
}


static string
_ff2(int dirtree, int dirmask)
{
    extern string _ff_fmask, _ff_pattern, _ff_rootdir;
    extern int _ff_lines, _ff_flags;
    string curdir, subdir, buffer, name;
    int dircnt, diridx;
    list dirlst;
    int mode, i;

    /* Process files within the directory */
    getwd("", curdir);
    if (rindex(curdir, sysslash()) != strlen(curdir)) {
        curdir += sysslash();
    }

    if ("" == _ff_pattern) {                    /* !ts */
        message("%s%s", shortname(curdir, 0), _ff_fmask);
    }

    /* Search the directory (for matching files) */
    if (_ff_fmask != "") {
        int     cyear, year, day, hour, min;
        int     size, mtime, flen = (ffwidth - 24);
        int     smode = 0;
        string  mon;

        if (F_DIR & _ff_flags) {               /* dir, mode selection */
            if (flen > 30) {
                flen -= 11;
                smode = 1;
            }
        }

        date(cyear, NULL, NULL);
        file_pattern(_ff_fmask);
        while (find_file(name, size, mtime, NULL, mode)) {
                                                /* append to directory list */
            if (mode & S_IFDIR) {
                if (_ff_fmask == WILD)
                    if (substr(name, 1, 1) != ".")
                    {/* Save the need the scan directory twice */
                        dirlst += name;
                        dircnt++;
                    }
                continue;
            }

            /* Filter specials */
            if (0 == (mode & S_IFDIR) && 0 == (_ff_flags & F_SPECIALS)) {
                if (name == GRRESTORE_FILE || name == GRSTATE_FILE) {
                    continue;                   /* grief internal */
                }

                if (substr(name, 1, 1) == ".") {
                    continue;                   /* special name */
                }
            }

            /* If 'pattern' was specified, perform text-search */
            if (_ff_pattern != "") {
                if ((mode & S_IFDIR) == 0 && searchfile(curdir + name) == ".") {
                    return ".";                 /* <ESC> during search */
                }
                continue;
            }

            /* Default action, build directory list */
            buffer = curdir+name;
            if ((i = strlen(buffer)) > flen) {
                buffer = substr(buffer, i-flen);
                if ((i = index(buffer, sysslash()))) {
                    buffer = substr(buffer, i);
                }
                buffer = "..." + buffer;
            }

            if (mode & S_IFDIR) {
                sprintf(buffer,  " %-" + flen + "s <DIR>  ",
                    buffer+sysslash());
            } else {
                sprintf(buffer, " %-" + flen + "s %7d", buffer, size);
            }
            insert(buffer);                     /* file/dir name/size */

            if (smode) {                        /* mode */
                insert(" " + mode_string(mode));
            }

                                                /* modification date/time */
            localtime(mtime, year, NULL, day, mon, NULL, hour, min, NULL);
            if (cyear == year) {
                sprintf(buffer,  " %s %2d %02d:%02d ;", substr(mon,1,3), day, hour, min);
            } else {
                sprintf(buffer, " %s %2d  %04d ;", substr(mon,1,3), day, year);
            }
            insert(buffer);

            insert(curdir + name + "\n");       /* full path */

            if (++_ff_lines % 100 == 0) {
                 if (aborted_pressed()) {
                     return ".";                /* user abort ? */
                }
            }
        }
    }

    /* Search the directory (for sub directories) */
    if (0 == (_ff_flags & F_RECURSIVE)) {       /* current directory only */
        return "";
    }

    /* Build directory list */
    if (_ff_fmask != WILD) {
        file_pattern(WILD);
        while (find_file(name, NULL, NULL, NULL, mode))
            if ((mode & S_IFDIR) && substr(name, 1, 1) != ".") {
#if defined(S_IFLNK)
                if (0 == lstat(name, NULL, NULL, NULL, NULL, mode) &&
                        (mode & S_IFLNK)) {
                    continue;                   /* dont follow links */
                }
#endif
                dirlst += name;                 /* append to directory list */
                dircnt++;
            }
    }

    if (aborted_pressed()) {
        return ".";
    }

    if (dirtree) {
        ++dirtree;                              /* increment directory depth */
    }

    /* Recurse list */
    diridx = 0;                                 /* directory list, into list */
    while (diridx < dircnt) {
        name = dirlst[ diridx ];                /* Next directory name */
        subdir = curdir + name + sysslash();

        /*  Draw tree structure:
         *      Create the tree structure (previous levels), based on the dir mask.
         *      Each bit within the mask represents either the end of the tree on
         *      a given level.
         */
        if (dirtree) {
            buffer = "\n";

#if defined(MSDOS) && (0)                       // MCHAR???
            for (i=2; i<dirtree; ++i) {         // parents
                if (dirmask & (1<<i)) {
                    buffer += "     ";
                } else {
                    buffer += "\xB3    ";
                }
            }
            if (diridx+1 >= dircnt) {           // level
                dirmask |= (1 << dirtree);
                buffer += "\xC0";
            } else {
                buffer += "\xC3";
            }
            buffer += "\xC4\xC4\xC4\xC4\xC4" + upper(name);
#else
            for (i=2; i<dirtree; ++i) {         // parents
                if (dirmask & (1<<i)) {
                    buffer += "     ";
                } else {
                    buffer += "|    ";
                }
            }
            if (diridx+1 >= dircnt) {           // level
                dirmask |= (1 << dirtree);
            }
            buffer += "+----" + upper(name);
#endif
            insert(buffer);

            move_abs(0, TREE_WIDTH);            // save the whole directory path
            insert(";");
            insert(lower(substr(subdir, strlen(_ff_rootdir)+1)) );
            if (++_ff_lines % 200 == 0) {
                if (aborted_pressed()) {
                    return ".";
                }
                write_buffer();                 // flush buffer
            }
        }

        /* Change the directory, process */
        if (validdir(subdir, 0) == FALSE ||
                aborted_pressed() || _ff2(dirtree, dirmask) == ".") {
            cd(curdir);
            return ".";
        }
        cd(curdir);                             // restore directory

        /* Increment list reference */
        diridx++;
    }
    return "";
}


/*
 *  sysslash ---
 *      Return the current backslash character
 */
static string
sysslash(void)
{
    if (ffslashc == "") {
        string curdir;

        getwd("", curdir);
        if (index(curdir, "\\")) {
            sprintf(ffslashc, "%c", BACKWARD_SLASH);
        } else {
            sprintf(ffslashc, "%c", FORWARD_SLASH);
        }
    }
    return ffslashc;
}


static string
shortname( string name, int add )
{
    int len, maxlen, i;

    if (add == 0) {
        add = 10;
    }
    len = strlen(name);
    maxlen = ffwidth - (31 + add);              // 31=std status line length
    if (len < maxlen || maxlen < 0) {
        return name;
    }
    i = maxlen/2;
    return substr(name, 1, i) + "~" + substr(name, len - (maxlen - (i + 2)));
}


/*
 *  validdir ---
 *      Determine whether a 'newdir' is a valid' directory.
 *
 *      restore:     0     Dont restore.
 *                   1     Only on error.
 *                   2     Always.
 */
static int
validdir(string newdir, int restore)
{
    string savedir;

    if (restore) {
        getwd("", savedir);
    }

    if (cd(newdir) == FALSE) {
        int ctm;

        error("%s : (%s)", shortname(newdir, strlen(strerror())+4), strerror());
        ctm = time()+5;
        while (read_char(1) == -1 && ctm > time()) {
            refresh();
        }
        if (restore) {
            cd(savedir);
        }
        return (FALSE);
    }

    if (2 == restore) {
        cd(savedir);
    }
    return (TRUE);
}


/*
 *  searchfile --
 *      File find engine support, search the specified 'file' for 'pattern'.
 */
static string
searchfile(string file)
{
    extern string _ff_pattern;
    extern int _ff_flags;
    int ret, buffer, dest = inq_buffer();

    // possible binary?
    if (ftest("B", file)) {
        int ch;

        if (0 == (_ff_flags & (F_BINARY|F_NOBINARY))) {
            string prompt, var;
                                                // prompt action
            sprintf(prompt, "\001%s, binary [^y^n^a^i]?", shortname(file, 16));
            do {
                if (! get_parm(NULL, var, prompt, 1)) {
                    return "";
                }
            } while (1 != strlen(var) || 0 == index("yYnNaAiI", ch = characterat(var, 1)));

            switch (ch) {
            case 'a': case 'A':
                _ff_flags |= F_BINARY;          // always
                break;

            case 'i': case 'I':
                _ff_flags |= F_NOBINARY;        // ignore all
                break;
            }
        }

        if ((_ff_flags & F_NOBINARY) || 'n' == ch || 'N' == ch) {
            return "";
        }
    }

    /* search buffer,
     *      create_nested_buffer increments the open count, hence
     *      wont create a new image.
     *
     *      for new buffers open as system stopping modeline etc
     */
    if ((buffer = create_nested_buffer(file, file, TRUE)) == -1) {
        return "";
    }
    set_buffer(buffer);
    ret = searchbuffer(dest);
    set_buffer(dest);
    delete_buffer(buffer);

    return (ret == -127 ? "." : "");
}


/*
 *  searchbuffer --
 *      Search the specified 'file' associated with 'buffer'.
 */
static string
searchbuffer(int dest)
{
    extern string _ff_pattern;
    extern int _ff_flags;

    string msg, file;
    int omsglevel, source, cnt, ret;

    inq_names(file);
    source = inq_buffer();
    msg = "Matches: %d/%d " + shortname(file, 20);

    save_position();
    top_of_buffer();

    message(msg, tstotals, cnt);
    omsglevel = set_msg_level(3);

    if ((ret = search_fwd(_ff_pattern, 1, (_ff_flags & F_IGNORECASE ? 0 : 1) )) > 0) {
        string home = inq_home();
        int hlen = strlen(home);

        if (hlen && (home == substr(file, 1, hlen))) {
            file = "~"+substr(file, hlen + 1);  /* strip home dir */
        }
        set_buffer(dest);
        insert(" File: " + file + " Pattern: " + _ff_pattern + "\n");
        set_buffer(source);

        while (ret > 0) {
            string textline;

            message(msg, ++tstotals, ++cnt);
            textline = readline(file);
            set_buffer(dest);
            insert(textline);
            set_buffer(source);
            if (aborted_pressed()) {
                ret = -127;
                break;
            }
            ret = search_fwd(_ff_pattern, 1);
        }
    }

    if (aborted_pressed()) {
        ret = -127;
    }
    restore_position();
    set_msg_level(omsglevel);
    return (ret);
}


/*
 *  readline --
 *      File find engine support (for searchfile), retrieve the next line from
 *      the specified 'file' stamp with it's current position.
 */
static string
readline(string file)
{
    string text_line;
    int line, col;
    int flen = ffwidth-10;

    inq_position(line, col);
    beginning_of_line();
    text_line = rtrim(read(flen));
    beginning_of_line();
    down();
    sprintf( text_line, " %5d,%3d %-" + flen + "s;%s\n",
        line, col, text_line, file);
    return(text_line);
}


/*
 *  aborted_pressed ---
 *      Determine whether the user has pressed ESC.
 */
static int
aborted_pressed(void)
{
    if (inq_kbd_char()) {
        int key = read_char(-1);

        if (key == 'q' || key == 'Q' || key == key_to_int("<ESC>")) {
            while (inq_kbd_char()) {
                read_char(-1);
            }
            return 1;
        }
    }
    return 0;
}


/*
 *  getpathname ---
 *      Returns a valid pathname for the specified path.
 */
static string
getpathname( string newdir )
{
    string drive, curdir;

    if ((newdir = trim(newdir)) == "") {
        newdir = ".";
#if defined(MSDOS)
    } else if (substr(newdir, 2, 1) == ":" && strlen(drive = newdir) == 2) {
        newdir += ".";
#endif
    }

    if (index("\\/", substr(newdir, strlen(newdir), 1))) {
        newdir += ".";
    }

    if (getwd(drive, curdir) && cd(newdir) == TRUE) {
        getwd(drive, newdir);
        cd(curdir);
        return newdir;
    }
    return "";
}

/*eof*/

