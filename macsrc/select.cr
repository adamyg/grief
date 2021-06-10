/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: select.cr,v 1.43 2021/06/10 06:13:05 cvsuser Exp $
 * Selection macros implementing buffer based popup user interface.
 *
 *
 */

#include "grief.h"

/* File select box */
#if defined(MSDOS)
#define DEFMARKER       "\xfe"
#else
#define DEFMARKER       "="
#endif
#define DELIMIT         "|"

/* Esc processing */
#define SEL_FALLOWESC   0x0001
#define SEL_FWASESC     0x0010
#define SEL_FWASEXIT    0x0020

/* Line where we put the first popup. */
#define TOP_LINE        1

/* Number of lines further down screen between each successive popup. */
#define TOP_LINE_INCR   1

/* Offset from from right hand margin for each successive popup window. */
#define WINDOW_OFFSET   6
#define MARGIN          12

int                     top_line = TOP_LINE;
int                     window_offset = WINDOW_OFFSET;

/* top_keyboard is the top level keyboard map. */
extern int              top_keyboard;           /* grief.cr */
extern int              popup_level;            /* grief.cr */

#if defined(__PROTOTYPES__)
static string           buf_shortname(string name, int len, int maxlen);
void                    buf_keys(void);
static void             buf_delete(void);
static void             buf_write(void);

static list             selfile_list(string path, string wild_card, string title, int dirs, int multi);
void                    selfile_keys(void);

void                    sel_copy(void);
void                    sel_keys(void);
void                    sel_tab(void);
void                    sel_home(void);
void                    sel_end(void);
void                    sel_up(void);
void                    sel_down(void);
void                    sel_enter(void);
void                    sel_pgup(void);
void                    sel_pgdn(void);
void                    sel_help(void);
void                    sel_alpha(void);
void                    sel_back(void);
static void             sel_timer(int reset);
static void             sel_match(void);
void                    sel_list(void);

static void             field_back(void);
static void             field_home(void);
static void             field_down(void);
static void             field_up(void);
static void             field_end(void);
static void             field_space(void);
static int              field_select(void);
static void             field_esc(void);
static int              field_display(list args, int line, int msg_flag);
#endif /*__PROTOTYPES__*/

static int              _sel_time;
static string           _sel_prefix = "";
static string           _sel_marker = DEFMARKER;


/*
 *  main ---
 *      runtime initialisation
 */
void
main(void)
{
    if (create_menu_window(FALSE) >= 0) {       /* menubar exists */
        ++top_line;
    }
}


/*
 *  buffer_list ---
 *      Display list of buffers on screen, and allow user to make a selection.
 *
 *  Parameters:
 *      shortmode -         First parameter says whether to display in long or short
 *                          format. Short format is compatible with the BRIEF display;
 *                          Long mode is adds extra status fields, demonstrating
 *                          GRIEF enhancements.
 *
 *      sysbuffers -        States whether to display system buffers as well.
 *
 *  Todo:
 *      resize events
 */
void
buffer_list(~ int shortmode, ~ int sysbuffers)
{
    string  homedir;
    string  buf_name;
    string  file_name, t_file_name;
    string  line, lines, modes;
    string  desc, encoding;
    string  nl;

    int     buf_no = 1;
    int     listbuf;
    int     win, cline, ccol;
    int     retval;
    int     curbuf, this_buf;
    int     position;
    int     buflen, flaglen, pathlen, linelen;
    int     homelen;
    int     len;

    shortmode = !shortmode;

    /* build buffer */
    curbuf = inq_buffer();
    inq_position(cline, ccol);
    listbuf = create_buffer("Buffer List", NULL, 1);

    homedir = inq_home();
    homelen = wstrlen(homedir);

    /* dynamic sizing */
#define MINWIN      74                          /* below this, window is truncated */
#define BORDER      4

    buflen = 14;                                /* .. (min) buffer name */
    if (shortmode) {
        flaglen = 1;                            /* .. short flags */

    } else if (sysbuffers) {
        flaglen = 30;                           /* .. system flags */

    } else {
        flaglen = 24;                           /* .. standard flags */
    }
                                                /* ... (min) path name */
    pathlen = (MINWIN - BORDER) - (buflen + flaglen + 1);

    inq_screen_size(NULL, linelen);             /* current width */

    if ((linelen -= BORDER) > buflen+flaglen+pathlen) {
        len = 0;                                /* dynamic resize ? */
        set_buffer(curbuf);                     /* anchor search */
        this_buf = curbuf;                      /* .. first */
        do {
            if (sysbuffers || (this_buf != inq_scrap() && !inq_system())) {
                int t_len;

                inq_names(NULL, NULL, buf_name);
                if ((t_len = wcwidth(buf_name)) > len) {
                    len = t_len;
                }
            }
            set_buffer(next_buffer(1));         /* ... next */

        } while ((this_buf = inq_buffer()) != curbuf);

        if (len > buflen) {                     /* resize buffer name */
            if (len + flaglen + pathlen > linelen) {
                buflen =                        /* ... limit to min pathlen */
                    linelen - (flaglen + pathlen);
            } else {
                buflen = len;
            }
        }

        pathlen =                               /* resize path name */
            linelen - (buflen + flaglen + 1);
    }

    /* list buffers */
    set_buffer(curbuf);
    set_buffer(next_buffer(1));                 /* .. first */
    while (1) {
        this_buf = inq_buffer();

                                                /* filter all hidden buffers */
        if (0 == (BF_HIDDEN & inq_buffer_flags())) {

            if (sysbuffers || (! inq_system() && this_buf != inq_scrap())) {
                inq_names(file_name, NULL, buf_name);

                /* buffer name */
                if ((len = wcwidth(buf_name)) > buflen) {
                                                /* .. shorten name */
                    buf_name = buf_shortname(buf_name, len, buflen);

                } else {
                    buf_name = buf_name;
                    while (len++ < buflen) {
                        buf_name += " ";        /* .. pad buffer name */
                    }
                }

                /* format file name */
                if (homelen &&                  /* .. strip home */
                        (homedir == wsubstr(file_name, 1, homelen)))  {
                    t_file_name = "~" + wsubstr(file_name, homelen + 1);

                } else {
                    t_file_name = file_name;
                }

                if ((len = wcwidth(t_file_name)) > pathlen) {
                    /*
                     *  truncate image
                     */
                    int plen, flen;
                                                /* .. determine path length */
                    if (0 == (plen = rindex(t_file_name, "/"))) {
                        plen = rindex(t_file_name, "\\");
                    }

                    flen = pathlen/4;           /* .. max length (25%) of path */
                    if (plen < (pathlen - flen)){
                        flen = pathlen - plen;  /* .. or smaller */
                    }

                    t_file_name =               /* .. shorten name to 'flen' */
                        wsubstr(t_file_name, 1, plen) +
                        buf_shortname(wsubstr(t_file_name, plen + 1), len - plen, flen);

                    if ((len = wcwidth(t_file_name)) > pathlen) {
                        t_file_name =           /* .. shorten path */
                            ".." + wsubstr(t_file_name, len - (pathlen - 3));
                    }
                }

                /* build line
                 *  Note:   Changes within the format length must be reflected
                 *          within the global 'flaglen' parameter.
                 */
                if (shortmode) {
                    if (inq_modified()) t_file_name += "*";
                    sprintf(line, "%s %-*S", buf_name, pathlen + 10, t_file_name);

                } else {
                    inq_position(position);

                    modes = "";                 /* 10 */
                    modes += inq_modified()                   ? "*" : " ";
                    modes += inq_buffer_flags() & BF_PROCESS  ? "P" : " ";
                    modes += inq_buffer_flags() & BF_BACKUP   ? "B" : " ";
                    modes += inq_buffer_flags() & BF_READONLY ? "R" : " ";
                    modes += inq_system()                     ? "S" : " ";

                    if (BFTYP_UNKNOWN == inq_buffer_type(NULL, desc, encoding)) {
                                                /* old-style */
                        if (inq_buffer_flags() & BF_BINARY) {
                            desc = "bin";
                        } else if (inq_buffer_flags() & BF_CR_MODE) {
                            desc = "dos";
                        } else {
                            desc = "na";        /* not supported */
                        }
                    }

                    if (linelen >= 90) {        /* long form */
                        if (0 == strlen(encoding)) {
                            encoding = desc;
                        }
                        encoding = format("%-16.16s", encoding);
                    } else {
                        encoding = format("%-4.4s", desc);
                    }

                    if (inq_lines() > 99999) {
                        lines = format("%4dk", inq_lines()/1000);
                    } else {
                        lines = format("%5d", inq_lines());
                    }

                    if (sysbuffers) {           /* sys mode, include buffer identifier */
                        sprintf(line, "%S (%3d) %s %5d %s %s %-*S",
                            buf_name, inq_buffer(), lines, position, modes, encoding, pathlen + 10, t_file_name);

                    } else {                    /* normal mode */
                        sprintf(line, "%S %s %5d %s %s %-*S",
                            buf_name, lines, position, modes, encoding, pathlen + 10, t_file_name);
                    }
                }

                inq_names(file_name, NULL, NULL);

                set_buffer(listbuf);
                insert(nl);
                nl = "\n";
                insert(line);

                insert(DELIMIT);                /* insert fullname for Alt-D */
                insert(file_name);

                ++buf_no;
                set_buffer(this_buf);
            }
        }

        if (this_buf == curbuf) {
            break;                              /* .. end of list */
        }

        set_buffer(next_buffer(1));             /* ... next */
    }

    message("%d buffer%s in list.", buf_no - 1, buf_no == 1 ? "" : "s");

    win = sized_window(buf_no, linelen,
                linelen >= 60 ?
                    "<Up/Down/Enter> select, <Alt-D> delete, <Alt-W> write" :
                linelen >= 48 ?
                    "<Up/Down/Enter>, <Alt-D> del, <Alt-W> write" :
                    "<Enter> <Alt-D> <Alt-W>");

    set_ctrl_state(WCTRLO_CLOSE_BTN, WCTRLS_SHOW, win);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_SHOW, win);

    set_buffer(listbuf);
    set_buffer_type(NULL, BFTYP_UTF8);
    sort_buffer();
    set_buffer(curbuf);
    retval = select_buffer(listbuf, win, SEL_NORMAL, "buf_keys", NULL, "Buffer List");
    message("");

    if (retval < 0) {
        delete_buffer(listbuf);
        set_buffer(curbuf);
        attach_buffer(curbuf);
        move_abs(cline, ccol);
        return;
    }

    set_buffer(listbuf);
    move_abs(retval, 0);
    line = rtrim(read());
    delete_buffer(listbuf);
    set_buffer(curbuf);
    move_abs(cline, ccol);

    line = substr(line, rindex(line, DELIMIT) + 1);
    if (substr(line, strlen(line)) == "*")
        line = substr(line, strlen(line) - 1);
    if (line != "") {
        if (edit_file(line) != -1) {
            display_file_name();
        }
    }
}


static string
buf_shortname(string name, int len, int maxlen)
{
    int i = maxlen/2;

    if (len < maxlen) {
        return name;
    }
    return wsubstr(name, 1, i) + "~" + wsubstr(name, len - (maxlen - (i+2)));
}


void
buf_keys(void)
{
    assign_to_key("<Alt-D>",        "::buf_delete");
    assign_to_key("<Ctrl-D>",       "::buf_delete");
    assign_to_key("<Alt-W>",        "::buf_write");
    assign_to_key("<Ctrl-W>",       "::buf_write");
}


static void
buf_delete(void)
{
    extern int list_items;
    string line, str;
    int buf;

    line = rtrim(read());
    line = substr(line, rindex(line, DELIMIT) + 1);
    if (substr(line, strlen(line)) == "*")
        line = substr(line, strlen(line) - 1);

    buf = inq_buffer(line);

    /* Dont let user delete a buffer which is currently being displayed */
    if (inq_views(buf)) {
        error( "Cannot delete a buffer being displayed." );
        return;
    }

    /* If buffer has been modified, check whether user is really sure. */
    if (inq_modified(buf)) {
        str = "X";
        while (str != "y" && str != "Y") {
            if (! get_parm(NULL, str, "Buffer has not been saved. Delete [ynw]? ", 1))
                str = "n";

            if ("n" == str || "N" == str) {
                message("");
                return;
            }

            if ("w" == str || "W" == str) {
                int curbuf;

                curbuf = inq_buffer();
                set_buffer(buf);
                write_buffer();
                set_buffer(curbuf);
                break;
            }
        }
    }

    delete_buffer(buf);
    delete_line();
    --list_items;
}


static void
buf_write(void)
{
    string line;
    int curbuf, buf;

    line = rtrim(read());
    line = substr(line, rindex(line, DELIMIT) + 1);
    if (substr(line, strlen(line)) == "*") {
        line = substr(line, strlen(line) - 1);
    }
    buf = inq_buffer(line);
    if (! inq_modified(buf)) {
        error("Buffer already saved.");
        return;
    }
    curbuf = inq_buffer();
    set_buffer(buf);
    write_buffer();
    set_buffer(curbuf);
    re_translate(SF_NOT_REGEXP, "*", " ");
    beginning_of_line();
    message("Saved buffer: %s", line);
}


string
select_file(string wild_card, ~string title, int dirs)
{
    list files;
    string path, file, cwd;
    int mode, i;

    getwd(NULL, cwd);
    wild_card = expandpath(wild_card);
    if (wild_card == "") {
        wild_card = "*";
    } else if (0 == lstat(wild_card, NULL, NULL, NULL, NULL, mode) && (mode & S_IFDIR)) {
        wild_card += "/*";
    } else {
        wild_card += "*";
    }

    if ((i = rindex(wild_card, "/")) > 0 ||
            (i = rindex(wild_card, ":")) == 2) {
        cd(substr(wild_card, 1, i));            /* change dir/drive */
        wild_card = substr(wild_card, i+1);
    }

    while (1) {
        getwd(NULL, path);
        files = selfile_list(path, wild_card, title, dirs, FALSE);
        if (0 == length_of_list(files)) {
            file = "";                          /* user aborted */
            break;
        }
        file = files[0];                        /* selected file */
        if (substr(file, strlen(file)) != "/") {
            break;                              /* non-directory */
        }
        cd(file);                               /* user selected a directory so lets show him that one. */
        wild_card = "*";
    }
    refresh();
    cd(cwd);

    if (file != "") {                           /* if user specified a file, force the prompt to accept the line. */
        push_back(key_to_int("<Enter>"));
    }
    return path + "/" + file;
}


list
select_files(string wild_card, ~string title, int dirs)
{
    list files;
    string path, cwd;
    int mode;
    int i;

    getwd(NULL, cwd);
    wild_card = expandpath(wild_card);
    if ("" == wild_card) {
        wild_card = "*";
    } else if (0 == lstat(wild_card, NULL, NULL, NULL, NULL, mode) && (mode & S_IFDIR)) {
        wild_card += "/*";
    } else {
        wild_card += "*";
    }

    if ((i = rindex(wild_card, "/")) > 0 ||
                (i = rindex(wild_card, ":")) == 2) {
        cd(substr(wild_card, 1, i));            // change dir/drive
        wild_card = substr(wild_card, i+1);
    }

    while (1) {
        getwd(NULL, path);
        files = selfile_list(path, wild_card, title, dirs, TRUE);
        if (0 == length_of_list(files) ||       // user aborted
                length_of_list(files) > 1) {    // or file(s) selected
            break;
        }
        message("cd %s", files[0]);
        cd(files[0]);
        wild_card = "*";
    }
    message("");
    refresh();
    cd(cwd);
    return files;
}


static list
selfile_list(string path, string wild_card, string title, int dirs, int multi)
{
    string name, tmpbuf;
    list dirlist, files;
    int curbuf, buf, win;
    int count, size, mtime, mode;
    int items, width, min_width;
    int ret;

    curbuf = inq_buffer();
    if ((min_width = strlen(path) + 6) < 28) {
        min_width = 28;
    }
    buf = create_buffer(title != "" ? title : path, NULL, 1);
    set_buffer(buf);
    set_buffer_type(NULL, BFTYP_UTF8);

    if (wild_card == "" || wild_card == "*" ||
            substr(wild_card, strlen(wild_card) - 1, 1) == "/") {
        if (strlen(path) > 1) {
            if (index(path, ":") != 2 || strlen(path) != 3) {
                if (multi) {                    // not root directory
                    dirlist += " ../";
                } else {
                    dirlist += "../";
                }
            }
        }
        wild_card = "*";
    }

    file_pattern(wild_card);
    while (find_file(name, size, mtime, NULL, mode)) {
        if (dirs && 0 == (mode & S_IFDIR)) {
            continue;                           // ignore, only directories
        }
        if (multi) {
            sprintf(tmpbuf, " %s%s", name, (mode & S_IFDIR) ? "/" : "");
        } else {
            sprintf(tmpbuf, "%s%s", name, (mode & S_IFDIR) ? "/" : "");
        }
        if (0 == (S_IFDIR & mode)) {            // inserts files etc
            insert(tmpbuf + "\n");
            ++items;
        } else {
            dirlist += tmpbuf;                  // accumulate directories
        }
        ++count;
    }

    if (0 == count) {
        beep();
    } else {
        if (items) {
            if (0 == length_of_list(dirlist)) {
                end_of_buffer();
                delete_line();
            }
            sort_buffer();                      // sort files
        }

        if (length_of_list(dirlist)) {          // insert directories
            top_of_buffer();
            items += length_of_list(dirlist);
            dirlist = sort_list(dirlist);
            while (list_each(dirlist, tmpbuf) >= 0) {
                insert(tmpbuf + "\n");
            }
            delete_line();
        }
        if ((width = inq_line_length()) < min_width) {
            width = min_width;
        }
        window_offset += 20;
        win = sized_window(items + 1, width, "");
        window_offset -= 20;
        ret = select_buffer(buf, win, NULL, "selfile_keys", NULL,
                    "help_display \"features/abbfile.hlp\" \"Filename Abbreviations\"");
        if (ret >= 0) {
            goto_line(ret);                     // current selection
            name = trim(read());
            if (substr(path, strlen(path)) != "/") {
                path += "/";
            }
            if (multi) {
                if (substr(name, strlen(name)) == "/") {
                    files += path + name;       // new dir
                } else {
                    top_of_buffer();
                    if (re_search(0, "<" + _sel_marker + "\\c") > 0) {
                        files += path;          // multi selections

                        do {
                            name = trim(read());
                            files += name;
                        } while (re_search(0, "<" + _sel_marker + "\\c") > 0);
                    } else {
                        files += path;
                        files += name;
                    }
                }
            } else {
                files += name;                  // current file
            }
        }
    }
    delete_buffer(buf);
    set_buffer(curbuf);
    attach_buffer(curbuf);
    return files;
}


void
selfile_keys(void)
{
    assign_to_key("<Space>",           "::sel_mark -1");
    assign_to_key("<Del>",             "::sel_mark 0");
    assign_to_key("<Ins>",             "::sel_mark 1");
    assign_to_key("-",                 "::sel_mark 0");
    assign_to_key("+",                 "::sel_mark 1");
    assign_to_key("<Keypad-Minus>",    "::sel_mark 0");
    assign_to_key("<Keypad-Plus>",     "::sel_mark 1");
    assign_to_key("<Keypad-Star>",     "::sel_mark 2");
}


static void
sel_mark(int how)
{
    extern int list_items;
    extern int multi;
    string name;

    if (! multi) {
        return;
    }

    if (-1 == how) {
        beginning_of_line();
        name = trim(read());
        if (substr(name, strlen(name)) == "/") {
            beep();                             /* cannot mark dir's */
        } else {
            beginning_of_line();
            delete_char();
            if (substr(name, 1, 1) == _sel_marker) {
                insert(" ");
            } else {
                insert(_sel_marker);
            }
        }
        sel_down();

    } else {
        list patlist, results;                  /* Select/unselect */
        int current_item, line, count;
        string pat;

        results[0] = "";
        patlist[0] = "Pattern: ";
        patlist[1] = "";

        results = field_list((how == 0 ? "Unselect" : (how == 1 ? "Select" : "Invert")),
                                results, patlist, TRUE, TRUE);

        if (length_of_list(results) > 0 && "" != (pat = results[0])) {
            line = list_items + 1;
            inq_position(current_item);
            while (line) {
                move_abs(line, 1);              /* parse 'line' */
                name = trim(read());
                if (substr(name, strlen(name)) != "/" &&
                        re_search(NULL, pat, name + 1) > 0) {
                    beginning_of_line();
                    delete_char();
                    switch (how) {
                    case 0:                     /* clear */
                        insert(" ");
                        break;
                    case 1:                     /* set */
                        insert(_sel_marker);
                        break;
                    case 2:                     /* invert */
                        if (substr(name, 1, 1) == _sel_marker) {
                            insert(" ");
                        } else {
                            insert(_sel_marker);
                        }
                    }
                    ++count;
                }
                --line;
            }
            move_abs(current_item, 1);          /* restore position */
            message("selected %d files ...", count);
        }
    }
}


/*
 *  Allows current buffer to be edited and we return when <Esc> key is hit.
 */
void
select_editable(void)
{
    string old_escape;

    keyboard_push(top_keyboard);
    old_escape = inq_assignment("<Esc>");
    assign_to_key("<Esc>", "exit");
    process();
    assign_to_key("<Esc>", old_escape);
    keyboard_pop(TRUE);
}


/*
 *  select_list ---
 *       Select list.
 *
 *  Parameters;
 *      title -             Window title, top window text.
 *
 *      message_string -    Message string, bottom window text.
 *
 *      step -              Number of elements in each line of 'list', at current 1, 2
 *                          or 3 are supported.
 *
 *      items -             Selection list.
 *
 *      flags -             Control flags:
 *
 *                              SEL_NORMAL --
 *                                  normal, left justication.
 *
 *                              SEL_CENTER --
 *                                  center lines in window.
 *
 *                              SEL_TOP_OF_WINDOW --
 *                                  make indicated line top of window.
 *
 *      [help_var] -        Context help topic.
 *
 *      [do_list]  -        Action to perform if item is selected (<Enter>).
 *
 *      [start_line]  -     Initial highlighted line within list.
 *
 *      [keyfunction] -     Optional additionl command to execute to set-up local
 *                          keyboard definitions.
 */
int
select_list(string title, string message_string, int step,
        declare items, ~int flags, ~declare help_var, ~list do_list, ~int, ~string)
{
    int     buffer, old_buffer, win, old_win, retval;
    int     len, depth, width, start_line;
    int     do_built = TRUE;
    string  keyfunction;
    list    help_list;
    declare option;

    if (! is_list(items)) {
        return -1;
    }

    if (get_parm(7, start_line) <= 0) {
        start_line = 1;                         // default initial line
    }

    get_parm(8, keyfunction);

    save_position();
    old_buffer = inq_buffer();
    buffer = create_buffer(title, NULL, 1);
    set_buffer(buffer);
    width = strlen(title) + 4;

    len = length_of_list(items);
    depth = 0; option = "";
    if (step < 0) {                             // special, no default do_list/do_help
        do_built = FALSE;
        step *= -1;
    }
    while (1) {
        if ((len -= step) < 0) {
            break;
        }

        option = items[step * depth];
        if (! is_string(option)) {
            break;                              // end of valid items
        }

        if (do_built) {
            if (step > 1) {
                do_list[depth] = items[step * depth + 1];
                if (step > 2) {
                    help_list[depth] = items[step * depth + 2];
                }
            }
        }

        if (depth > 0) {
            insert("\n");
        }
        insert(option);
        ++depth;
    }

    width = inq_line_length();
    if (width < strlen(title)) {
        width = strlen(title) + 3;
    }
    if (! is_string(help_var)) {
        help_var = help_list;
    }

    old_win = inq_window();
    win = sized_window(inq_lines() + 1,
                width + (flags == SEL_CENTER ? 2 : 1), message_string);
    top_line += TOP_LINE_INCR;
    window_offset += 6;
    retval = select_buffer(buffer, win, flags, keyfunction, do_list, help_var, start_line);
    window_offset -= 6;
    top_line -= TOP_LINE_INCR;

    delete_buffer(buffer);
    set_buffer(old_buffer);
    set_window(old_win);
    attach_buffer(old_buffer);

    restore_position();
    return retval;
}


/*
 *  select_slim_list ---
 *      This function is similar to select_list() but we assume that the list is
 *      unstructured -- there are no actions or help associated with each element.
 */
int
select_slim_list(string title, string message_string,
        declare l, int flags, ~declare help_var, ~string do_list, ~int step)
{
    int width;

    list help_list;
    int buffer, old_buffer, win, len, old_win, i, retval;

    if (!is_list(l)) {
        return -1;
    }

    /* Save position so that cursor doesn't move to the top of the
     *  screen when the user has finished selecting.
     */
    save_position();

    old_buffer = inq_buffer();
    buffer = create_buffer(title, NULL, 1);
    set_buffer(buffer);
    width = strlen(title) + 4;

    if (step == 0) {
        step = 1;
    }
    len = length_of_list(l);
    for (i = 0; i < len; i += step) {
        insert(l[i] + "\n");
    }
    delete_line();

    width = inq_line_length();
    if (width < strlen(title)) {
        width = strlen(title) + 3;
    }
    if (!is_string(help_var)) {
        help_var = help_list;
    }

    old_win = inq_window();
    win = sized_window(inq_lines() + 1,
                width + (flags == SEL_CENTER ? 2 : 1), message_string);
    top_line += TOP_LINE_INCR;
    window_offset += 6;
    retval = select_buffer(buffer, win, flags, do_list, NULL, help_var);
    window_offset -= 6;
    top_line -= TOP_LINE_INCR;

    delete_buffer(buffer);
    set_buffer(old_buffer);
    set_window(old_win);
    attach_buffer(old_buffer);

    /* Restore position -- leaves cursor in same place on screen
     * as when user entered this macro.
     */
    restore_position();

    return retval;
}


/* Macro to select an entry from a buffer (passed as a parameter).
 * The cursor keys and <Home>, <End> allow movement; typing a letter
 * looks for a line which matches the character typed.
 * This macro returns a number indicating the line in the buffer
 * selected, or < 0 to indicate the user pressed <Esc> to abort
 * the selection.
 *
 * (macro select_buffer
 *         buf             Buffer to make selection from
 *         win             Window in which to display buffer.
 *         flags           Flags:
 *                           SEL_NORMAL -- normal, left justication
 *                           SEL_CENTER -- center lines in window.
 *                           SEL_TOP_OF_WINDOW --
 *                                  make indicated line top of window.
 *         keylist         Optional additionl command to execute to set up
 *                         local keyboard definitions.
 *         [do_list]       Action to perform if item is selected (<Enter>).
 *         [help_list]     List of macros to call if <Alt-H> typed on menu item.
 *         [start_line]    Item to make active
 *         [keep_window]   Whether the window should be kept or deleted ?
 *         )
 */
int
select_buffer(int buf, int win, ~int flags, ~declare, ~list do_list,
      ~declare help_list, ~int start_line, ~int keep_window)
{
    int but_clicks, but_line;
    int old_buf, old_win;
    int list_items, width, depth, retval;
    int selection, selection_flags;
    declare keylist;

    UNUSED(do_list);                            /* used by lower level functions */
    UNUSED(help_list);                          /* used by lower level functions */

    UNUSED(but_clicks, but_line);

    but_clicks = but_line = 0;

    old_buf = inq_buffer();
    old_win = inq_window();

    set_window(win);
    set_buffer(buf);
    attach_buffer(buf);
    top_of_buffer();
    list_items = inq_lines();

    if (start_line < 1) {
        start_line = 1;                         /* no line, top of window */
    } else if (start_line > list_items) {
        start_line = list_items;
    }

    if (flags & SEL_CENTER) {
        string title, msg;

        depth = 0;
        width = inq_line_length();
        inq_window_info(win, NULL, NULL, NULL, NULL, NULL, title, msg);
        if  (strlen(title) + 2 > width)
            width = strlen(title) + 2;
        if  (strlen(msg) + 2 > width)
            width = strlen(msg) + 2;
        while (++depth <= list_items) {
            insert(" ", (width - strlen(read())) / 2 + 1);
            beginning_of_line();
            down();
        }
    }

    top_of_buffer();
    --list_items;

    keyboard_push();
    keyboard_typeables();

    copy_keyboard(top_keyboard,         "search__fwd", "search__back",
                                            "search_next");
    assign_to_key("<Alt-C>",            "sel_copy");
    assign_to_key("<Alt-D>",            "screen_dump");
    assign_to_key("<Alt-H>",            "sel_help");
    assign_to_key("<Tab>",              "sel_tab");
    assign_to_key("<Home>",             "sel_home");
    assign_to_key("<End>",              "sel_end");
    assign_to_key("<Up>",               "sel_up");
    assign_to_key("<Down>",             "sel_down");
    assign_to_key("<Wheel-Up>",         "sel_up");
    assign_to_key("<Wheel-Down>",       "sel_down");
    assign_to_key("<PgUp>",             "sel_pgup");
    assign_to_key("<PgDn>",             "sel_pgdn");
    assign_to_key("<Esc>",              "sel_esc");
    assign_to_key("<Keypad-minus>",     "sel_esc");
    assign_to_key("<Enter>",            "sel_list");
    assign_to_key("<Alt-K>",            "sel_keys");
    assign_to_key("<Alt-P>",            "screen_dump");
    assign_to_key("<Backspace>",        "sel_back");

    if (display_mode() & (DC_WINDOW|DC_MOUSE)) {
        assign_to_key("<Button1-Down>", "::sel_button1 1");
        assign_to_key("<Button1-Double>", "::sel_button1 2");
    }

    goto_line(start_line);
    if (SEL_TOP_OF_WINDOW & flags) {
        set_top_left(start_line);

    } else if (start_line > 1) {                // SEL_CENTER_OF_WINDOW
        int height;

        inq_window_size(height);
        if (start_line > (height / 2)) {        // center
            int top = start_line - (height/2);

            if (top > (list_items - height)) {
                top = (list_items - height);
            }
            set_top_left(top);
        }
    }
    drop_anchor(3);

    /* Evaluate function setting up private key-bindings. If function
     * returns a list, save that in case user wants to see key-bindings.
     */
    get_parm(3, keylist);
    if (is_string(keylist)) {
        keylist = execute_macro(keylist);
    }

    /* Intercept all normal keys typed so we can do the selection of
     * an item which starts with a letter.
     */
    sel_timer(1);
    register_macro(REG_TYPED, "sel_alpha", TRUE);

    /* Keep track of the level of nesting. This allows the mousehandler
     * to decide what to do inside popups.
     */
    selection_flags = SEL_FALLOWESC;

    ++popup_level;
    process();
    --popup_level;

    if (selection_flags & SEL_FWASESC) {
        retval = -1;                            // 1
    } else if (selection_flags & SEL_FWASEXIT) {
        retval = selection;
    } else {
        inq_position(retval);
    }

    /* Remove registered macro. */
    unregister_macro(REG_TYPED, "sel_alpha", TRUE);
    raise_anchor();
    keyboard_pop();

    if (! keep_window && old_win != win) {
        delete_window();                        // destroy the window
    }
    set_window(old_win);
    set_buffer(old_buf);
    attach_buffer(old_buf);

    return retval;
}


/*
 *  sel_copy ---
 *      This macro is called when the user hits <Alt-C> in a popup window. The whole
 *      window is copied to the scrap to allow the user to do what he/she wants with it.
 */
void
sel_copy(void)
{
    save_position();
    top_of_buffer();
    drop_anchor(MK_LINE);
    end_of_buffer();
    copy();
    restore_position();
    message("Buffer copied to scrap.");
}


/*
 *  sel_but_down ---
 *      Handle the left hand button being pressed.
 */
static void
sel_button1(int type)
{
    extern int but_clicks, but_line, win;
    int tm, x, y, cwin, line, col, where, region, event;

    /* Count clicks */
    tm = get_mouse_pos(x, y, cwin, line, col, where, region, event);
    if (1 == type) {
        if (tm < 0 || tm > CLICK_TIME_MS) {
            but_clicks = 1;                     // click window expired
        } else {
            ++but_clicks;
        }
    } else if (2 == type) {
        ++but_clicks;                           // double click
    }

    /* Process event */
    if (win == cwin) {
        switch (where) {
        case MOBJ_INSIDE:
            move_abs(line, 1);                  // move selection
            sel_warp();
            if (line == but_line && 2 == but_clicks) {
                push_back(key_to_int("<Enter>"));
                break;                          // double-clicks
            }
            but_line = line;
            break;
        case MOBJ_TOP_EDGE:
        case MOBJ_BOTTOM_EDGE:
        case MOBJ_TITLE:
        case MOBJ_CLOSE:
            sel_esc();
            break;
        }
    }
}


/*
 *  sel_keys ---
 *      Function called if user types <Alt-K> to see the macro specific key-bindings.
 */
void
sel_keys(void)
{
    extern declare keylist;

    if (is_list(keylist)) {
        select_list("Key Bindings", "", 1, keylist, SEL_NORMAL);

    } else {
        int curbuf, buf, win;
        int i, len;
        list lst;

        lst = key_list(NULL, NULL, NULL);
        len = length_of_list(lst);
        curbuf = inq_buffer();
        buf = create_buffer( "Key Binding", NULL, 1 );
        set_buffer(buf);
        for (i = 0; i < len; i += 2) {
            insert(lst[i]);  move_abs(0, 24); insert(lst[i + 1]); insert("\n");
        }
        delete_line();
        sort_buffer();

        message("");
        set_buffer(curbuf);                     // restore buffer
        top_line += TOP_LINE_INCR;
        win = sized_window(inq_lines(buf), inq_line_length(buf));
        select_buffer(buf, win, SEL_NORMAL);
        top_line -= TOP_LINE_INCR;

        delete_buffer(buf);                     // release local buffer
    }
}


void
sel_tab(void)
{
    /*eat character*/
}


void
sel_home(void)
{
    raise_anchor();
    move_abs(1, 0);
    drop_anchor(3);
}


void
sel_end(void)
{
    raise_anchor();
    move_abs(inq_lines(), 0);
    drop_anchor(3);
    refresh();
    set_bottom_of_window();
}


void
sel_up(void)
{
    raise_anchor();
    up();
    drop_anchor(3);
}


void
sel_down(void)
{
    extern int list_items;
    int current_item;

    inq_position(current_item);
    if (current_item <= list_items) {
        raise_anchor();
        down();
        drop_anchor(3);
    }
}


/*
 *  sel_esc ---
 *      Exit the select list, returning -1 to caller.
 */
void
sel_esc(void)
{
    extern int selection, selection_flags;

    selection_flags |= SEL_FWASESC;
    exit();
}


/*
 *  sel_exit ---
 *      Exit the select list, returning the specified value.
 */
void
sel_exit(~int retval)
{
    extern int selection, selection_flags;

    selection_flags |= SEL_FWASEXIT;
    selection = retval;
    exit();
}


/*
 *  sel_enter ---
 *      Exit the select list, returning the current selection.
 */
void
sel_enter(void)
{
    exit();
}


void
sel_pgup(void)
{
    raise_anchor();
    page_up();
    drop_anchor(3);
}


void
sel_pgdn(void)
{
    int line;

    raise_anchor();
    page_down();
    inq_position(line);
    if (line > inq_lines()) {
        goto_line(inq_lines());
    }
    drop_anchor(3);
}


void
sel_help(void)
{
    extern declare help_list;
    int line;

    inq_position(line);

    if (is_string(help_list) && help_list != "") {
        if (substr(help_list, 1, 13) == "help_display ") {
            execute_macro(help_list);           // see grep for example
        } else {
            cshelp("features", help_list);
        }

    } else if (is_list(help_list) && length_of_list(help_list) >= line) {
        cshelp("features", help_list[line - 1]);

    } else {
        cshelp("features", read());

    }
}


/*
 *  sel_alpha ---
 *      Registered macro -- called when user hits a key which causes a self_insert. If
 *      its upper/lower case letter go to line which has this at the beginning.
 *
 *      If its a <Ctrl-letter> then go to that entry and pretend the user hit <Return>.
 */
void
sel_alpha(void)
{
    string ch, pat, real_ch;
    int current_item;

    prev_char();
    real_ch = ch = read(1);
    if (atoi(ch, 0) < ' ')
        sprintf(ch, "%c", atoi(ch, 0)+'@');
    delete_char();
    inq_position(current_item);

    sel_timer(0);
    pat = "<[ ]@" + quote_regexp(_sel_prefix + ch);
    move_rel(0, 1);
    if (re_search(NULL, pat) || re_search(NULL, upper(pat))) {
        raise_anchor();
        drop_anchor(3);
        move_rel(0, -1);
        _sel_prefix += ch;

    } else {
        top_of_buffer();
        if (re_search(NULL, pat) || re_search(NULL, upper(pat))) {
            raise_anchor();
            drop_anchor(3);
            move_rel(0, -1);
            _sel_prefix += ch;

        } else {
            move_abs(current_item, 0);
        }
    }

    if (atoi(real_ch, 0) < ' ') {
        push_back(key_to_int("<Enter>"));
    }

    if ("" == _sel_prefix) {
        message("");
    } else {
        message("alpha: %s", _sel_prefix);
    }
}


/*
 *  sel_back ---
 *      Backspace key processo, untypes characters from the prefix buffer.
 */
void
sel_back(void)
{
    sel_timer(0);
    if (strlen(_sel_prefix) > 1) {
        _sel_prefix = substr(_sel_prefix, 1, strlen(_sel_prefix) - 1);
        message("alpha: %s", _sel_prefix);

    } else {
        _sel_prefix = "";
        message("");
    }
}


/*
 *  sel_timer ---
 *      If >= 1.2 seconds have elapsed since the last key, we clear the prefix.
 */
static void
sel_timer(int reset)
{
    int mins, sec, msec;
    int nth10, diff;

    time(NULL, mins, sec, msec);
    nth10 = (((mins * 60) + sec) * 10) + (msec / 100);
    diff = nth10 - _sel_time;
    if (reset || diff >= 12) {
        _sel_prefix = "";
    }
    _sel_time = nth10;
}


/*
 *  sel_list ---
 *      This function is called when user selects an item in a list, returning the item
 *      (line number) selected.
 */
void
sel_list(void)
{
    extern list do_list;
    declare function;
    int line;

    /* Is there a selection list ? */
    if (! is_list(do_list) || ! length_of_list(do_list)) {
        exit();
        return;
    }

    /* Find the name of the macro to call based on the current line number. */
    inq_position(line); --line;
    function = do_list[line];

    /* We only support strings containing the macro name at present. */
    if (is_string(function)) {
        if (function == "") {
            exit();
        } else {
            execute_macro(function);
        }

    } else {
        select_list("XYZ", "ABC", 1, function);   /* ??? */
    }
}


/*
 *  sel_warp ---
 *      This function called by the mouse code to 'warp' the cursor.
 *
 *      The user clicked on an entry and we want to unhilight the old entry and hilight
 *      the currrent.
 */
void
sel_warp(void)
{
    raise_anchor();
    drop_anchor(MK_LINE);
    beginning_of_line();
}


/*
 *  sized_window ---
 *      This macro returns a window buffer of a reasonable size for the terminal. This
 *      macro takes into account that we may not be running on a plain 80x25, and gives
 *      us as many lines as will fit on the screen whilst taking the callers request
 *      into account.
 *
 *  Parameters:
 *      lines -         Maximum number of lines wanted.
 *      width -         Width of window.
 *      msg -           Message to appear on the bottom of the window.
 *      lx -            Optional, left x
 *      ty -            Optional, top y.
 *
 *  Returns:
 *      int - window handle.
 */
int
sized_window(int lines, int width, ~string msg, ~int, ~int)
{
    int curwin, newwin;
    int screen_lines, screen_cols;
    int lx, by, rx, ty, i;

    lx = -1, ty = -1;                           /* default, offset/top */
    get_parm(3, lx);
    get_parm(4, ty);

    inq_screen_size(screen_lines, screen_cols);
    curwin = inq_window();

    if (width < 0) {
        width = screen_cols - MARGIN;
    }

    if (lx < 0) {
        lx = screen_cols - (window_offset + width) - 1;
    }
    if (lx < 1) {
        lx = 1;
    }

    rx = lx + width;
    --screen_cols;
    if (rx >= screen_cols) {
        i = rx - screen_cols;                   /* diff */
        rx = screen_cols - 1;
        if ((lx -= i) < 1) {                    /* relocate left */
            lx = 1;
        }
    }

    if (ty < 0) {
        ty = top_line;
    }
    by = ty + lines;

    if (by >= screen_lines - 3) {
        by = screen_lines - 4;
    }
    create_window(lx, by, rx, ty, msg);
    newwin = inq_window();
    set_window(curwin);

    return newwin;
}


/*
 *  field_list ---
 *      Macro to display a list of items and allow the user to toggle the values in the
 *      list. The valid values for each item are given in a list-of-lists. This is a
 *      primitive but useful forms-entry macro.
 *
 *  Usage:
 *      field_list(
 *          "title to appear on top of window",
 *          list of default values,
 *          list of items and values,
 *          [TRUE is Esc should abort edit],
 *          )
 *
 *  Returns:
 *      List of integers indicating which options were selected.
 */
list
field_list(string title, list values, list args, ~int, int escnull = 0)
{
    int     curbuf, curwin, buf, win;
    int     selection, selection_flags;
    int     line, width, esc;
    list    org;
    string  s, msg;
    declare v;

    UNUSED(selection);

    curbuf = inq_buffer();
    curwin = inq_window();

    if (get_parm(3, esc) <= 0) {                // no esc specification, then <Esc> is treated as Exit.
        esc = 0;
    }

    if ((buf = create_buffer(title, NULL, 1)) == -1) {
        return NULL;
    }
    set_buffer(buf);

    for (line = 0; 1; ++line) {
        declare prompt, option;

        prompt = args[line * 2];                // assign initial value
        if (is_string(prompt)) {
            option = args[line * 2 + 1];
            if (is_string(option) && 0 == strlen(option)) {
                args[(2 * line) + 1] = values[line];
            }
        }

        if (! field_display(args, line, FALSE)) {
            break;
        }
    }

    msg =  "";                                  // build user msg
    if (line > 1) {
        msg += "<Up/Down> select, ";
    }
    msg += "<Enter> exit";
    if (esc) {
        msg += ", <Esc> abort.";
    }

    width = 40;
    if (strlen(msg) > width) {
        width = strlen(msg) + 4;
    }
    if (inq_line_length() > width) {
        width = inq_line_length() + 4;
    }
    win = sized_window(inq_lines() + 1, width, msg, 5, 3);
    set_window(win);
    attach_buffer(buf);

    field_display(args, 0, TRUE);               // active field

    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Backspace>",    "::field_back");
    assign_to_key("<Del>",          "::field_back");
    assign_to_key("<Esc>",          "::field_esc");
    assign_to_key("<Enter>",        "exit");
    assign_to_key("<Home>",         "::field_home");
    assign_to_key("<Up>",           "::field_up");
    assign_to_key("<Down>",         "::field_down");
    assign_to_key("<Wheel-Up>",     "::field_up");
    assign_to_key("<Wheel-Down>",   "::field_down");
    assign_to_key("<End>",          "::field_end");
    assign_to_key("<Space>",        "::field_space");
    assign_to_key("<Tab>",          "::field_select");
    assign_to_key("<Right>",        "::field_select");

    /* Keep track of the level of nesting. This allows the mousehandler to decide what
     *  to do inside popups.
     */
    selection_flags = 0;
    if (esc) {
        selection_flags = SEL_FALLOWESC;
    }

    ++popup_level;
    if (esc) {
        org = values;                           // save original values
    }
    process();
    --popup_level;

    keyboard_pop();
    message("");

    /*
     * The results list needs to have the actual value for non-toggling fields, rather than
     * just an index into the actual value. So, we scan the list and put in the actual value
     * read from the buffer.
     */
    if (0 == (SEL_FWASESC & selection_flags)) {
        top_of_buffer();
        for (line = 0; line < inq_lines(); ++line) {
            /*
             *  Check to see if we have a string at this position. We have
             *  to assign value to a polymorphic variable so we test its type.
             */
            v = values[line];
            if (is_string(v)) {
                s = read();
                values[line] = trim(substr(s, index(s, ":") + 2));
            }
            down();
        }
    }

    set_window(curwin);
    set_buffer(curbuf);
    delete_window(win);
    delete_buffer(buf);

    if (SEL_FWASESC & selection_flags) {
        if (escnull) {
            return NULL;                        // NULL list
        }
        return org;                             // otherwise original values
    }
    return values;                              // new values
}


/*
 *  Function called when user hits the <Backspace> key in a field list.
 */
static void
field_back(void)
{
    int col, colon_col;

    inq_position(NULL, col);
    beginning_of_line();
    colon_col = index(read(), ":") + 2;
    move_abs(NULL, col);
    if (col > colon_col) {
        backspace();
    }
}


static void
field_home(void)
{
    extern list args;

    field_display(args, 0,TRUE);
}


static void
field_down(void)
{
    extern list args;
    int line;

    inq_position(line);
    if (line <= inq_lines()-1) {
        field_display(args, line, TRUE);
    }
}


static void
field_up(void)
{
    extern list args;
    int line;

    inq_position(line);
    if (--line > 0) {
        field_display(args, --line, TRUE);
    }
}


static void
field_end(void)
{
    extern list args;

    field_display(args, inq_lines()-1, TRUE);
}


static void
field_space(void)
{
    extern list args, values;
    declare option;
    int line, m;

    inq_position(line); --line;
    option = args[line * 2 + 1];
    if (! is_list(option)) {
        self_insert(' ');
        return;
    }
    m = values[line];
    if (length_of_list(option) > m + 1) {
        values[line] = m + 1;
    } else {
        values[line] = 0;
    }
    field_display(args, line, TRUE);
}


static int
field_select(void)
{
    extern list args, values;
    declare option;
    int line, ret;

    inq_position(line); --line;                 // current item
    option = args[line * 2 + 1];

    if (! is_list(option)) {
        beep();

    } else {
        int depth = 0;
        list lst;
                                                // build new list
        while (depth < length_of_list(option)) {
            lst[depth] = option[depth];
            ++depth;
        }

        if (depth) {                            // create pick list
            top_line += 5;
            window_offset += 10;
            ret = select_list( "Pick List", "<Enter> to select",
                        1, lst, 0, NULL, NULL, values[line]+1 );
            window_offset -= 10;
            top_line -= 5;

            if (ret > 0) {
                values[line] = ret-1;
            }
            field_display(args, line, TRUE);
        }
    }
}


static void
field_esc(void)
{
    extern int selection_flags;

    if (selection_flags & SEL_FALLOWESC) {
        selection_flags |= SEL_FWASESC;
        exit();
    }
}


/*
 *  field_display ---
 *      Display the specified field 'n' being the index starting 0 from
 *      the argument list 'arg'.
 */
static int
field_display(list args, int line, int msg_flag)
{
    extern list values;
    declare prompt, option;

    prompt = args[line * 2];
    option = args[line * 2 + 1];

    if (! is_string(prompt)) {
        return FALSE;                           // missing prompt
    }

    raise_anchor();
    move_abs(line + 1, 1);
    delete_line();
    insert(prompt);
    save_position();

    if (is_string(option)) {
        if (msg_flag) {
            message("Type in new value of option.");
        }
        insert(option);

    } else if (is_list(option)) {
        declare idx;

        idx = values[line];
        if (is_string(idx)) {
            idx = atoi(idx);                    // option index
        }
        if (msg_flag) {
            message("Press <Space or Tab/Right> to select alternate option.");
        }
        insert(option[idx]);
    }

    if ((line + 1) * 2 < length_of_list(args)) {
        insert("\n");
    }

    restore_position();
    drop_anchor(4);
    end_of_line();
    return TRUE;
}

/*end*/
