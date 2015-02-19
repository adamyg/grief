/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: restore.cr,v 1.30 2014/10/27 23:28:26 ayoung Exp $
 * Save and restore editing session.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
static int              res_cleanup(string bfile);
static int              res_mini_save(void);
static int              res_full_save(void);
static string           res_db_name(int create);
static void             res_save_window(void);
static int              res_mini_restore(void);
static int              res_full_restore(void);
static int              crlogfile(string file);
#endif

/*
 *  ss_flag             Set to TRUE to save and restore the total editing state. This
 *                      is more expensive and can litter .crstate files in the current
 *                      directory, but many people like this feature.
 *
 *  ss_cache            If this variable is not set, then the restore file is stored in
 *                      the current working directory, otherwise within the specified
 *                      directory managed thru 'grstatedb'.
 *                      Note: Default value of GRRESTORE.
 *
 *  ss_file             Full-name of the state file.
 *
 *  ss_version          Imported interface version.
 *
 *  ss_age              Age at which time crstate are considered stale.
 */
static int              ss_flag = FALSE;
static string           ss_cache = "";
static string           ss_file = GRSTATE_FILE;
static int              ss_version = 0;
static int              ss_age = -1;

#define DELIMIT         "|"


/*
 *  res_file            Restore file.
 *
 *  res_path            Filename computed at load time where we load and
 *                      store the restore info.
 */
static string           res_file = GRRESTORE_FILE;
static string           res_path;


/*
 *  External referenced to saved/restored components
 */
extern string           search__pattern;
extern string           translate__pattern;
extern string           translate__replacement;


/*
 *  The following list is a list of all the objects we can save and
 *  restore.  For each property listed, a macro called
 *  '_OBJECT_save' and  '_OBJECT_restore' should be defined which
 *  understands how to get and set the attribute.
 */
static list             restorers = {
    "savehist",                                 /* history buffer */
    "scrapper"                                  /* scrap buffer */
    };


/*
 *  save_state ---
 *      This macro should be called to turn on full state saving.
 */
void
save_state(void)
{
    ss_flag = TRUE;
}


/*
 *  _griget_restore ---
 *      restore configuration
 */
void
_griset_restore(string arg)
{
    list args;
    int i;

    args = split(arg, " =", 1);
    for (i = 0; i < length_of_list(args); i += 2) {
        if (args[i] == "save") {                /* user override */
            if (args[i+1] == "full") {
                ss_flag = TRUE;

            } else if (args[i+1] == "mini") {
                ss_flag = FALSE;

            } else {
                ss_flag = (substr(args[i+1], 1, 1) == "y" ? TRUE : FALSE);
            }

        } else if (args[i] == "cache") {        /* cache directory */
            ss_cache = args[i+1];

        } else if (args[i] == "age") {          /* cache aging */
            ss_age = atoi(args[i+1]);

        } else if (args[i+1] == "yes") {        /* restorers */
            /*
             *  scrapper=yes
             */
            string restorer = args[i];
            int len, r;

            for (r = 0, len = length_of_list(restorers); r < len; ++r)
                if (restorer == restorers[r]) {
                    if (inq_macro("_" + restorer + "_save") <= 0) {
                        load_macro(restorer);
                    }
                    break;
                }

        } else if (args[i] == "restorer") {     /* alt form, restorers */
            /*
             *  restorer=scrapper
             *
             *  or restorer=scrapper [--option] // future form
             */
            string restorer = args[i+1];
            int len, r;

            for (r = 0, len = length_of_list(restorers); r < len; r++)
                if (restorer == restorers[r]) {
                    if (inq_macro("_" + restorer + "_save") <= 0) {
                        load_macro(restorer);
                    }
                    break;
                }
        }
    }
}


/*
 *  _griget_restore ---
 *      Save restore configuration
 */
string
_griget_restore(void)
{
    string state;
    int len, i;

    if (ss_flag) {
        state += "save=full";
    } else {
        state += "save=mini";
    }

    if (ss_cache != "" && ss_cache != getenv("GRRESTORE")) {
        state += " cache=" + ss_cache;
    }

    if (ss_age != -1) {
        state += " age=" + ss_age;
    }

    for (i = 0, len = length_of_list(restorers); i < len; i++) {
        if (inq_macro("_" + restorers[i] + "_save") > 0) {
            state += " " + restorers[i] + "=yes";
        }
    }

    return state;
}


/*
 *  _startup_complete ---
 *      Macro executed at load time to restore the current editing state. This macro is
 *      called after command line files have been read in and is passed a flag telling
 *      us whether we read files from the command line. If we have then don't bother
 *      restoring state.
 */
int
_startup_complete(int files_read_in)
{
    string home, grfile;
    int files;

    /*
     *  One-shot
     */
    if (! first_time()) {
        return -1;
    }

    /*
     *  Set up a trap for when we exit so we can save the state.
     */
    register_macro(REG_EXIT, "restore_exit");

    /*
     *  Determine restore path
     */
    home = inq_home();
    if (substr(res_file, 1, 1) == "/" || home == "") {
        res_path = res_file;
    } else {
        sprintf(res_path, "%s/%s", home, res_file);
    }

    /*
     *  Determine '.grstatedb' path
     */
    if (ss_cache == "") {
        ss_cache = getenv("GRRESTORE");
    }

    /*
     *  If files specified on command don't bother restoring the state.
     */
    if (files_read_in) {
        return 0;
    }

    /*
     * Try and restore from the .crstate file in current directory.
     * If that fails we'll use the restore file in the home directory.
     */
    if (ss_flag) {
        files = res_full_restore();
    }
    if (!ss_flag || files < 0) {
        files = res_mini_restore();
    }

    /*
     * GRFILE handling
     */
    grfile = getenv("GRFILE");
    if (files > 0) {
        res_cleanup(grfile);

    } else {
        int ml = set_msg_level(0);
        if (edit_file(grfile) != -1) {           /* default=newfile */
            files++;
        }
        set_msg_level(ml);
    }

    return files;
}


/*
 *  restore_exit ---
 *      Macro called on exit to save the current file and position in
 *      the restore file in the home directory.
 *
 *      Try and save full state if user requested it.
 */
void
restore_exit(void)
{
    if (ss_flag && res_full_save()) {
        return;
    }
    res_mini_save();
}


/*
 *  res_cleanup ---
 *      If 'grfile' doesn't exist and we've got it loaded and its not displayed
 *      in a window then delete it.
 */
static int
res_cleanup(string grfile)
{
    int newbuf;

    if (substr(grfile, 1, 1) != "/") {
        string cwd;

        getwd(NULL, cwd);
        grfile = cwd + "/" + grfile;
    }

    newbuf = inq_buffer(grfile);
    if (newbuf >= 0 && inq_views(newbuf) == 0 &&
            (inq_buffer_flags(newbuf) & BF_NEW_FILE) != 0) {
        delete_buffer(newbuf);
    }
}


/*
 *  res_mini_save ---
 *      Macro to save the current buffer list.
 */
static int
res_mini_save(void)
{
    string cwd, name, buf_name, echofmt;
    string state_info;
    int curbuf, resbuf;
    int l, c, flags;
    int is_system;

    getwd(NULL, cwd);
    curbuf = inq_buffer();
    is_system = inq_system();

    /* Open restore file and make sure we don't keep a backup of the state file. */
    if (edit_file(EDIT_SYSTEM, res_path) == -1) {
        return -1;
    }
    resbuf = inq_buffer();
    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);

    /* Delete previous entries. */
    goto_line(1);
    while (re_search(NULL, "^" + quote_regexp(cwd + DELIMIT)) > 0) {
        beginning_of_line();
        delete_line();
        goto_line(1);
    }

    /* Export current buffer list. */
    set_buffer(curbuf);
    do {
        flags = inq_buffer_flags();
        inq_names(name, NULL, buf_name);
        inq_position(l, c);
        if (!inq_system() && (flags & BF_NEW_FILE) == 0 &&
                inq_buffer() != resbuf) {       /* file name and cursor */
            state_info += cwd + DELIMIT + name + DELIMIT + l + DELIMIT + c + "\n";
        }
        set_buffer(next_buffer(is_system));     /* next buffer */
    } while (inq_buffer() != curbuf);

    /*
     *  Save state of the echo line options.
     */
    set_buffer(resbuf);
    if (re_search(NULL, "^echo_line=") > 0) {
        beginning_of_line();
        delete_line();
        if (re_search(NULL, "^echo_format=") > 0) {
            beginning_of_line();
            delete_line();
        }
    }
    state_info += "echo_line=" + inq_echo_line() + "\n";
    echofmt = inq_echo_format();
    if (strlen(echofmt)) {
        state_info += "echo_format=" + echofmt + "\n";
    }
    insert(state_info);

    /*
     *  Now save the buffer.
     */
    write_buffer();
}


/*
 *  res_full_save ---
 *      Macro to save the window layout and buffer states.
 */
static int
res_full_save(void)
{
    int    curwin, curbuf;
    int    win, l, c, flags;
    string name, buf_name;
    string state_info;
    list   lst;
    int    i, len;
    int    is_system = inq_system();

    /* Save size of screen in state file because we cannot attempt
     * to restore state if we have a different sized window.
     */
    inq_screen_size(l, c);
    sprintf(state_info, "screen=%d,%d\n", l, c);

    /* Syntax version */
    state_info += "version=3\n";                /* 15/02/06 */

    /* Menubar active */
    if (create_menu_window(FALSE) >= 0) {
        state_info += "menubar=yes\n";
    }

    /* Save list of buffers being edited. */
    curbuf = inq_buffer();
    do {
        inq_names(name, NULL, buf_name);
        inq_position(l, c);
        flags = inq_buffer_flags();

                                                /* ignore system, new and restore-buffer */
        if (!inq_system() && 0 == (flags & BF_NEW_FILE) &&
                    buf_name != ss_file && buf_name != res_file) {
            //
            //  others/
            //      use_hard_tabs
            //      inq_tabs
            //      inq_margin
            //      inq_ruler
            //
            int marginl, marginr, margins;
            string f = "";
                                                /* sticky buffer flags */
            if (flags & BF_RULER)                   f += "ruler,";
            if (inq_buffer_flags(NULL, "hiwhitespace")) f += "hiwhitespace,";
            if (flags & BF_EOF_DISPLAY)             f += "eof,";

            if (inq_buffer_flags(NULL, "noundo"))   f += "noundo,";
            if (inq_buffer_flags(NULL, "spell"))    f += "spell,";
            if (inq_buffer_flags(NULL, "folding"))  f += "folding,";

            if (inq_buffer_flags(NULL, "line_numbers")) f += "line_numbers,";
            if (inq_buffer_flags(NULL, "line_oldnumbers")) f += "line_oldnumbers,";
            if (inq_buffer_flags(NULL, "line_status")) f += "line_status,";

                                                /* filename, cursor and flags */
            state_info += "buffer=" + name + DELIMIT + l + DELIMIT + c + DELIMIT + f + DELIMIT;

                                                /* buffer specific margin's */
            inq_margins(NULL, marginl, marginr, margins, NULL, FALSE);
            if (marginl > 0) state_info += "marginl=" + marginl + ",";
            if (marginr > 0) state_info += "marginr=" + marginr + ",";
            if (margins > 0) state_info += "margins=" + margins + ",";

            state_info += "\n";                 /* entry terminator */
        }

        set_buffer(next_buffer(is_system));     /* next buffer */

    } while (inq_buffer() != curbuf);

    /*
     *  Walk through all background windows and save the layout information.
     */
    curwin = inq_window();                      /* starting point */
    while (1) {
        res_save_window();
        if ((win = next_window()) < 0 || win == curwin) {
            break;                              /* looped */
        }
    }

    /*
     *  Open state image
     */
    if (ss_cache != "") {
        string file;

        if ((file = res_db_name(TRUE)) == "") {
            return FALSE;
        }
        if (ss_file != file) {
            remove(ss_file);                    /* remove old */
        }
        ss_file = file;
    }

    if (edit_file(EDIT_SYSTEM, ss_file) == -1) {
        return FALSE;
    }

    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);
    clear_buffer();
    insert("[grief-edit]\n");

    insert(state_info);

    /*
     *  Save the bookmark information.
     */
    lst = bookmark_list();
    for (i = 0, len = length_of_list(lst); i < len; i += 4) {
        inq_names(name, NULL, NULL, lst[i + 1]);
        insert("bookmark=" + lst[i] + DELIMIT +
            name + DELIMIT + lst[i + 2] + DELIMIT + lst[i + 3] + "\n");
    }

    /*
     *  Save the search/translate and replacement settings
     */
    insert("search=" + quote_regexp(search__pattern) + "\n");
    insert("translate=" + quote_regexp(translate__pattern) + "\n");
    insert("repl=" + quote_regexp(translate__replacement) + "\n");

    insert("[grief-edit-end]\n");
    write_buffer();

    /*
     *  Run any optional save processes
     */
    for (i = 0, len = length_of_list(restorers); i<len; i++) {
        name = restorers[i];
        if (inq_macro("_" + name + "_save") > 0) {
            execute_macro("_" + name + "_save");
        }
    }

    write_buffer();

    return TRUE;
}


/*
 *  res_db_name ---
 *      Retrieve and optional create a state file managed by the cache database.
 */
static string
res_db_name(int create)
{
    int curbuf, dbbuf;
    string cwd, line;
    int seq;

    getwd(NULL, cwd);
    curbuf = inq_buffer();

    if (edit_file(EDIT_SYSTEM, ss_cache + "/" + GRSTATE_DB) == -1) {
        return "";
    }

    dbbuf = inq_buffer();
    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);

    seq = 0;
    goto_line(1);
    if (re_search(NULL, "^[0-9]+=" + quote_regexp(cwd) + ">") > 0) {
        beginning_of_line();
        seq = atoi(read());                     /* current ident */
    }

    if (create && seq == 0) {
        int x_seq = 1;

        goto_line(1);
        if ((line = read()) != "\n" && line != "") {
            x_seq = atoi(line);                 /* next sequence */
        }
        seq = x_seq++;
        beginning_of_line();
        delete_line();
        insert(x_seq + "\n");
        insert(seq + "=" + cwd + "\n");
        write_buffer();
    }

    set_buffer(curbuf);
    attach_buffer(curbuf);

    if (seq == 0) {
        return "";
    }
    sprintf(line, "%s/gr%05d", ss_cache, seq);  /* state image name */
    return (line);
}


/*
 *  res_db_clean ---
 *      Clean the restore db, removing stale entries.
 */
void
res_db_clean(~int)
{
    int    curbuf, dbbuf;
    list   entries;
    string old, new;
    int    oldest, nseq;
    int    removed, i;
    int    age = ss_age;

    if (0 == age) {
        get_parm(1, age);
    }
    if (ss_cache == "" || 0 == age) {           /* aging enabled ? */
        return;
    }

    curbuf = inq_buffer();

    if (edit_file(EDIT_SYSTEM, ss_cache + "/" + GRSTATE_DB) == -1) {
        return;
    }

    dbbuf = inq_buffer();
    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);

    oldest = time() - (60 * 60 * 24 * (age <= 0 ? 90 : age));

    /*
     *  scan for missing and stale state images
     */
    goto_line(2);
    while (re_search(NULL, "^[0-9]+=*>") > 0) {
        list parts = split(rtrim(read()), "=");
        int oseq = atoi(parts[0]);
        int mod;

        sprintf(old, "%s/gr%05d", ss_cache, oseq);
        if (ss_file == old) {                   /* current */
            sprintf(new, "%s/gr%05d.sav", ss_cache, oseq);
            rename(old, new);
            entries += parts;
            ss_file = new;

        } else if (stat(old, NULL, mod) == 0 && mod > oldest && stat(parts[1]) == 0) {
            sprintf(new, "%s/gr%05d.sav", ss_cache, oseq);
            rename(old, new);
            entries += parts;                   /* younger then age days */

        } else {
            remove(old);
            removed++;
        }
        down();                                 /* next */
    }
    nseq = (length_of_list(entries)/2) + 1;     /* next sequence number */

    /*
     *  rebuild db rename state files
     */
    clear_buffer();
    insert(nseq + "\n");
    for (i = 0; i< length_of_list(entries); i += 2) {
        int oseq = atoi(entries[i]);

        --nseq;
        sprintf(old, "%s/gr%05d.sav", ss_cache, oseq);
        sprintf(new, "%s/gr%05d", ss_cache, nseq);
        rename(old, new);
        insert(nseq + "=" + entries[i+1] + "\n");
        if (ss_file == old) {
            ss_file = new;                      /* current image */
        }
    }
    write_buffer();

    set_buffer(curbuf);
    attach_buffer(curbuf);
    delete_buffer(dbbuf);

    message("removed %d entries", removed);
}


/*
 *  res_save_window ---
 *      Macro to save the information about the 'current' window.
 */
static void
res_save_window(void)
{
    extern string state_info;
    extern int curwin;
    int type, win_id, buf_id, lx, by, rx, ty;
    int line, col, top_left;
    string filename, buf;

    type = inq_window_info(win_id, buf_id, lx, by, rx, ty);
    if (type == 0) {
        /* only what tiled */
        inq_position(line, col);
        inq_top_left(top_left);
        inq_names(filename);

/*DELIMIT*/
        sprintf(buf, "window=%d|%d|%d|%d|%d|%d|%d|%s|%s\n",
            lx, by, rx, ty, line, col, top_left, filename,
                (win_id == curwin ? "CURRENT" : "0"));
        state_info += buf;
    }
}


/*
 *  res_mini_restore ---
 *      Macro to restore the buffer list from the state file.
 *
 *  Returns:
 *      Number of files restored.
 *
 */
static int
res_mini_restore(void)
{
    string cwd, line, file;
    int resbuf, firstbuf;
    int l, c, ml, files;
    list lst;

    firstbuf = -1;
    getwd(NULL, cwd);

    /*
     *  Open ',crisp' and make sure we don't keep a backup of the state file.
     */
    if (! exist(res_path) || edit_file(EDIT_SYSTEM, res_path) == -1) {
        return 0;
    }
    resbuf = inq_buffer();
    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);

    /* Restore status line mode. */
    top_of_buffer();
    if (re_search(NULL, "^echo_line=") > 0) {
        int eflags;

        line = substr(rtrim(read()), 11);
        eflags = atoi(line);
        if (re_search(NULL, "^echo_format=") > 0) {
            line = substr(rtrim(read()), 13);
            set_echo_format(line);
        }
        if (eflags >= 0) {
            echo_line(eflags);
        }
    }

    /* Restore the buffer list. */
    top_of_buffer();
    while (re_search(NULL, "^" + quote_regexp(cwd+DELIMIT)) > 0) {
        line = rtrim(read());
        lst = split(line, DELIMIT, 1);
        file = lst[1];
        l = lst[2];
        c = lst[3];

        /*
         *  Only edit file if the file exists.
         */
        if (exist(file)) {
            /*
             *  Make sure user sees progress messages
             */
            ml = set_msg_level(0);
            if (edit_file(file) != -1) {
                /*
                 *  Goto the line and column where we left off.
                 */
                if (firstbuf == -1) {
                    firstbuf = inq_buffer();
                }
                move_abs(l, c);
                files++;
            }
            set_msg_level(ml);
        }
        set_buffer(resbuf);
        end_of_line();
    }

    /* Restore previous buffer. */
    if (firstbuf >= 0) {
        /*
         *  Make first loaded active
         */
        set_buffer(firstbuf);
        attach_buffer(firstbuf);
    }

    if (inq_buffer() != resbuf) {               /* close state file */
        delete_buffer(resbuf);
    }

    return files;
}


/*
 *  res_full_restore ---
 *      Macro to restore the state of the screen from the state file. Returns TRUE if
 *      we restored the state, otherwise FALSE.
 *
 *      Here we make the restore buffer current and parse the information in the state
 *      file. This includes:
 *
 *          o size of the screen.
 *          o buffer list.
 *          o bookmarks.
 *          o search pattern.
 *          o translate pattern.
 *          o replacement string.
 *          o search configuration
 *              - direction,
 *              - regular expression setting,
 *              - search block setting.
 *              - translate direction.
 *
 *      No error checking is done on the results of the parse: if the numbers aren't
 *      right, the (atoi) call will fail and things will default to 0 (if the strings
 *      don't exist, they'll probably be null).
 *
 *  Returns:
 *      Number of files restored.
 */
static int
res_full_restore(void)
{
    int     old_curwin = -1;
    int     curbuf, curwin;
    int     state_buf;
    int     line_no;
    string  line;
    string  file;
    string  current_file;
    string  delimit;
    int     partial_restore = FALSE;
    int     ml;
    int     files, i;
    int     l, c;
    list    lst;

    curbuf = inq_buffer();
    curwin = inq_window();

    /* Cannot restore state if no file there. */
    if (exist(ss_file)) {                       /* old image */
        if (edit_file(EDIT_SYSTEM, ss_file) == -1) {
            return -1;
        }

    } else {                                    /* cache image */
        string t_file;

        if ((t_file = res_db_name(FALSE)) == "") {
            return -1;
        }

        if (edit_file(EDIT_SYSTEM, t_file) == -1) {
            return -1;
        }

        ss_file = t_file;
    }

    if (inq_macro("menuoff", 1) == 1) {
        menuoff();                              /* if loaded, disable */
    }

    set_buffer_flags(NULL, BF_SYSBUF | BF_NO_UNDO, ~BF_BACKUP);
    state_buf = inq_buffer();
    top_of_buffer();

    message("restoring session");

    if (search_fwd("<\\[grief-edit\\]>")) {
        down();
        inq_position(line_no);

        /*
         *  First line must tell us the screen size. If this is different,
         *  then we ignore the state file.
         */
        line = compress(read(), TRUE);
        if (substr(line, 1, 7) != "screen=") {
            partial_restore = TRUE;

        } else {
            lst = split(substr(line, 8), ",", 1);
            inq_screen_size(l, c);
            if (lst[0] != l || lst[1] != c) {
                partial_restore = TRUE;
            }
        }
        ++line_no;

        delimit = " ";                          /* default delimiter */

        while (substr(line = compress(read()), 1, 1) != "[") {

            if ((i = index(line, "=")) == 0) {
                break;                          /* Format error */
            }

            switch (substr(line, 1, i-1)) {
            case "version":
                /*
                 *  Version (added in version 2)
                 */
                ss_version = atoi(substr(line, 9));
                if (ss_version >= 3) {
                    delimit = DELIMIT;          /* version 3 */
                } else {
                    delimit = ",";              /* version 2 */
                }
                break;

                /*
                 *  Menubar was enabled
                 */
            case "menubar":
                menuon();
                break;

                /*
                 *  Edit the specified buffers so user sees them in a buffer list.
                 *  These entries should precede the window entries otherwise we'll
                 *  end up with the windows containing the wrong info.
                 */
            case "buffer":
                l = 1, c = 1;
                line = substr(line, 8);

                /* If we have a line and column specifier then get that */
                if (index(line, delimit)) {
                    lst = split(line, delimit, 1);
                    file  = lst[0];
                    l     = lst[1];
                    c     = lst[2];
                } else {
                    file = line;
                }

                /* Only edit file if the file exists and if debugging not the crisp log */
                if (exist(file) && 0 == crlogfile(file)) {
                    ml = inq_msg_level();
                    set_msg_level(0);
                    message("%s", file);
                    if (edit_file(file) != -1) {
                        move_abs(l, c);         /* restore line */

                                                /* optional edit flags */
                        if (ss_version >= 3 && length_of_list(lst) >= 4) {
                            string flag, nflags;
                            list flags;
                                                /* apply sticky flags, need global list */
                            flags = tokenize(lst[3], ",", TOK_COLLAPSE_MULTIPLE);
                            while (list_each(flags, flag) >= 0) {
                                switch (flag) {
                                case "ruler":
                                case "eof":
                                case "noundo":
                                case "hiwhitespace":
                                case "line_numbers":
                                case "line_oldnumbers":
                                case "line_status":
                                case "spell":
                                case "folding":
                                    nflags += flag + ",";
                                    break;
                                default: {
                                        list parts = tokenize(flag, "=");
                                        if (2 == length_of_list(parts)) {
                                            switch (parts[0]) {
                                            case "marginl":
                                                set_margins(NULL, atoi(parts[1]));
                                                break;
                                            case "marginr":
                                                set_margins(NULL, NULL, atoi(parts[1]));
                                                break;
                                            case "margins":
                                                set_margins(NULL, NULL, NULL, atoi(parts[1]));
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                            set_buffer_flags(NULL, nflags);
                        }
                        ++files;
                    }
                    set_msg_level(ml);
                }
                set_buffer(state_buf);
                break;

                /*
                 *  If we have a window definition, then create it.
                 */
            case "window":
                lst = split(substr(line, 8), delimit, 1);
                file = lst[7];

                /* ignore request if debugging and the grief log */
                if (0 == crlogfile(file)) {
                    set_window(curwin);
                    ml = inq_msg_level();
                    set_msg_level(0);
                    edit_file(file);
                    set_msg_level(ml);
                    if (partial_restore) {
                        if (lst[8] == "CURRENT") {
                            current_file = file;    /* current file */
                        }
                    } else {
                        i = create_tiled_window(lst[0], lst[1], lst[2], lst[3], inq_buffer());
                        if (old_curwin == -1) {
                            old_curwin = i;         /* current window */
                        }
                        set_top_left(lst[6], NULL, NULL, lst[4], lst[5]);
                    }
                    set_buffer(state_buf);
                }
                break;

                /*
                 *  Restore bookmarks. These should follow the buffer and window
                 *  definitions because we assume the buffers are now loaded.
                 */
            case "bookmark":
                lst = split(substr(line, 10), delimit, 1);
                drop_bookmark(lst[0], "y", inq_buffer(lst[1]), lst[2], lst[3]);
                break;

                /*
                 *  Restore the search/translation and replacement patterns.
                 */
            case "search":
                search__pattern = substr(line, 8);
                break;
            case "translate":
                translate__pattern = substr(line, 11);
                break;
            case "repl":
                translate__replacement = substr(line, 6);
                break;
            }
            move_abs(line_no++);
        }
    }
    message("session restored");

    /*
     *  Run any optional restore processes
     */
    for (i = 0, l = length_of_list(restorers); i < l; i++) {
        line = restorers[i];
        if (inq_macro("_" + line + "_restore") > 0) {
            execute_macro("_" + line + "_restore");
        }
    }

    /*
     *  Restore previous buffer
     */
    if (old_curwin >= 0) {
        delete_window(curwin);

        set_window(old_curwin);
        i = inq_window_buf();
        if (i >= 0) {
            set_buffer(i);
        }
    } else {
        set_buffer(curbuf);
        attach_buffer(curbuf);
    }

    if (current_file) {
        edit_file(current_file);
    }

    /*
     *  Set flag to make sure we save full state on exit if not already set.
     */
    ss_flag = TRUE;

    return files;
}

static int
crlogfile(string file)
{
    if (inq_debug()) {
        if (0 == strfilecmp(GRLOG_FILE, basename(file))) {
            string cwd;

            getwd(NULL, cwd);
            if (0 == strfilecmp(cwd, dirname(file))) {
                return 1;
            }
        }
    }
    return 0;
}

/*end*/
