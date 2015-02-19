/* -*- indent-width: 4; -*- */
/* $Id: history.cr,v 1.20 2014/10/27 23:28:23 ayoung Exp $
 * Command line history.
 *
 *
 */

#include "grief.h"

/*
 *  list of prompts on the command line together with the function to be called
 *  during _bad_key events.
 */
static list         compl_list =    /*FIXME -- runtime configuration required*/
    quote_list(
        "Command:",                 "compl_cmd",
        "Edit file:",               "compl_editfile",
        "Macro file:",              "compl_macrofile",
        "File to read:",            "compl_readfile",
        "Go to bookmark:",          "compl_bookmark",
        "Load keystroke file:",     "compl_keylib",
        "Cut to named scrap:",      "compl_scrap",
        "Copy to named scrap:",     "compl_scrap",
        "Paste named scrap:",       "compl_scrap"
        );

/*
 *  prompt abbreviations.
 *
 *      Command:    prompt where the user is typing in a multi-token command line.
 */
static list         compl_commands =
    quote_list(
        "cd",                       "compl_cd"
        );

void                book_keys(void);

static string       hist_last_response;
static int          hist_buf,
                    hist_first_line,
                    hist_lines,
                    hist_no;

#define HIST_MAX    15


void
main()
{
    if (!hist_buf) {
        hist_buf = create_buffer("History Buffer", NULL, TRUE);
    }
}


void
_griset_history(string spec)
{
    list args;
    int i;

    args = split(spec, " =.", 1);               /* TODO --- import parameters */
    for (i = 0; i < length_of_list(args); i += 2) {
        //
        //  depth=<numeric>
        //
        //  mode=<options>
        //      first
        //          Only return the first match.
        //
        //      longest
        //          Complete till longest common string. If this does not
        //          result in a longer string, use the next part.
        //
        //      list
        //          When more then one match, list all matches.
        //
        //      list_full
        //          When more then one match, list all matches and
        //          complete first match.
        //
        //      list_longest
        //          When more then one match, list all matches and
        //          complete till longest common string.
        //
    }
}


string
_griget_history(void)
{
    return "";
}


int
_inq_history(void)
{
    return hist_buf;
}

//replacement
void
_prompt_begin(void /*string prompt*/)
{
    int curbuf, msg_level, idx;
    string curr_msg;

    curr_msg = inq_message();
    if (idx = index(curr_msg, ":")) {
        curr_msg = substr(curr_msg, 1, idx-1);
    }

    curbuf = inq_buffer();
    set_buffer(hist_buf);
    hist_no = -1;
    top_of_buffer();
    msg_level = inq_msg_level();
    set_msg_level(3);

    /* Locate the prompts sub-section */
    if (re_search( NULL,
            "<### "+ inq_command() + "::" + quote_regexp(curr_msg) + " ###") <= 0) {
        /*
         *  Not found, insert header
         */
        insert("### " + inq_command() + "::" + curr_msg + " ###\n");
        inq_position(hist_first_line);
        hist_lines = -1;
        set_buffer(curbuf);
        set_msg_level(msg_level);
        return;
    }

    /* Locate the sub-section end */
    down();
    inq_position(hist_first_line);
    if (re_search(NULL, "<### * ###$") <= 0) {
        hist_lines = inq_lines();
    } else {
        inq_position(hist_lines);
    }
    hist_lines -= hist_first_line;
    set_buffer(curbuf);
    attach_buffer(curbuf);
    set_msg_level(msg_level);
}


replacement string
_bad_key()
{
    string line;
    int curbuf, key_pressed;
    int do_push_back = TRUE;

    key_pressed = read_char();
    switch (key_pressed) {
    case key_to_int("<Up>"):            /* next within history */
        hist_no++;
        break;
    case key_to_int("<Down>"):          /* previous history */
        hist_no--;
        break;
    case key_to_int("<Alt-L>"):
        return hist_last_response;

    case key_to_int("<Tab>"):           // completion
        line = completion(inq_cmd_line(), inq_message());
        if (line != "") {
            if (do_push_back) {
                push_back(key_to_int("<End>"));
            }
            return line;
        }
        break;
    case key_to_int("<Alt-B>"):         // current buffer
        return compl_buffer(1);

    case key_to_int("<Alt-F>"):         // current path and file
        return compl_buffer(0);

    case key_to_int("<Alt-P>"):         // paste buffer text
        return compl_paste();

    case key_to_int("<Alt-W>"):         // walk history list
        return compl_history();

    default:
        return inq_cmd_line();
    }

    if (hist_lines <= 0) {              // No history?
        return "";
    }

    if (hist_no < 0) {
        hist_no = hist_lines - 1;
    } else if (hist_no >= hist_lines) {
        hist_no = 0;
    }

    curbuf = set_buffer(hist_buf);
    goto_line(hist_first_line + hist_no);
    line = rtrim(read());
    set_buffer(curbuf);

    return line;
}

//replacement
void
_prompt_end(void)
{
    int    curbuf;
    string cmd;

    if (inq_message() == "Command cancelled.") {
        return;
    }
    cmd = inq_cmd_line();
    if (strlen(cmd) > 2) {
        hist_last_response = cmd;
    }

    curbuf = set_buffer(hist_buf);
    if (strlen(cmd) > 1) {
        goto_line(hist_first_line);
        insert(cmd + "\n");                     /* insert at head of history */
        hist_lines++;

        if (re_search(NULL, "<{### }|{" + quote_regexp(cmd) + "$}") > 0)
            if (read(1) != "#") {
                delete_line();                  /* remove duplicate */
                hist_lines--;
            }
        if (hist_lines > HIST_MAX) {
            goto_line(hist_first_line + (hist_lines - 1));
            delete_line();                      /* remove last command */
        }
    }

    set_buffer(curbuf);
    attach_buffer(curbuf);
}


string
completion(string word, string prompt)
{
    int len, n, ret, match_count;
    list cmd_list, match_list;
    string arg;

    if ((n = index(prompt, ":")) > 0) {
        prompt = substr(prompt, 1, n);
        if ((n = re_search(SF_NOT_REGEXP, prompt, compl_list)) >= 0)
           return execute_macro(compl_list[n + 1], word);

        /*
         *  If its the command prompt then check the first command word
         *  to see if we have a special handler
         */
        if (prompt == "Command:") {
            n = index(word, " ");
            if (n) {
                arg = ltrim(substr(word, 1, n - 1));
            } else {
                arg = trim(word);
            }

            if ((n = re_search(NULL, "^" + quote_regexp(arg) + "$", compl_commands)) >= 0) {
                return execute_macro(compl_commands[n + 1], word);
            }
        }

        /*
         *  Test if the macro has defined a completion handler
         *
         *      eg. prompt  "New Mode"
         *          handle   _completion_New_Mode
         */
        prompt = substr(prompt, 1, strlen(prompt)-1);
        prompt = re_translate(SF_GLOBAL, "[ \t]+", "_", prompt);
        if (inq_macro("_completion_" + prompt) > 0) {
            return execute_macro("_completion_" + prompt, word);
        }
    }

    len = strlen(word);
    if (len == 0) {
        beep();
        return "";
    }

    match_count = -1;
    word = "<" + word;

    cmd_list = command_list();
    n = re_search(SF_IGNORE_CASE, word, cmd_list);
    while (n >= 0) {
        match_list[++match_count] = cmd_list[n];
        n = re_search(SF_IGNORE_CASE, word, cmd_list, n + 1);
    }

    if (match_count < 0) {
        beep();
        return "";
    }

    if (0 == match_count) {
        ret = 0;
    } else {
        ret = select_list("Abbreviations", "", 1, match_list, SEL_CENTER,
                "help_display \"features/abbrev.hlp\" \"Help on Abbreviations\"");
        refresh();
    }

    if (ret < 0) {
        return "";
    }
    return match_list[ret - 1];
}


/*
 *  Invoked when we need to insert the current buffer name ...
 */
string
compl_buffer(int what)
{
    string filename, bufname;

#if (XXX)
    if (_dialog_orgbufid != 0) {
        int id;
        id = inq_buffer();
        set_buffer(_dialog_orgbufid);
        inq_names(filename, NULL, bufname);
        set_buffer(id);
    } else {
        inq_names(filename, NULL, bufname);
    }
#else
    inq_names(filename, NULL, bufname);
#endif

    push_back(key_to_int("<End>"));
    return inq_cmd_line() + (what ? bufname : filename);
}


/*
 *  Invoked when we need to insert the current buffer text ...
 */
string
compl_paste(void)
{
    int    line, col, line1, col1;
    string buff;

    inq_position(line, col);
    objects("word_right");
    inq_position(line1, col1);
    if (line != line1) {
        col = 0;
    }
    if (col1 > col) {
        move_abs(line1, col);
        buff = read(col1-col);
    }
    move_abs(line1, col1);
    push_back(key_to_int("<End>"));
    return inq_cmd_line() + compress(buff);
}


/*
 *  Invoked when we need to do filename completion
 */
string
compl_cmd(~string file)
{
    extern int do_push_back;
    string spec;
    int loc;

    if ((loc = rindex(file, " "))) {
        spec = substr(file, loc+1);
        file = substr(file, 1, loc);
    } else {
        spec = file;
        file = "";
    }
    file += select_file(trim(spec), NULL, FALSE);
    if (file != "") {
        do_push_back = FALSE;
    }
    return file;
}


/*
 *  Invoked when we need to do filename completion
 */
string
compl_readfile(~string file)
{
    extern int do_push_back;

    file = select_file(ltrim(file), NULL, FALSE);
    if (file != "")
        do_push_back = FALSE;
    return file;
}


string
compl_editfile(~string file)
{
    extern int do_push_back;
    list files;
    string path;
    int n;

    files = select_files(trim(file), NULL, FALSE);
    if (0 == length_of_list(files)) {
        return file;                            /* previous command line */
    }
    do_push_back = FALSE;
    path = files[0];                            /* path=arg0 */
    if (length_of_list(files) == 1) {
        return path;                            /* path only */
    }
    for (n = length_of_list(files); --n >= 2;) {
        edit_file( path + files[n] );           /* foreach additional file */
    }
    push_back(key_to_int("<Enter>"));
    return path + files[1];                     /* new command line */
}


string
compl_macrofile(~string file)
{
    extern int do_push_back;

    file = select_file(ltrim(file), NULL, FALSE);
    if (file != "") {
        do_push_back = FALSE;
    }
    return file;
}


/*
 *  Function to display the bookmark popup
 */
string
compl_bookmark(~string str)
{
    int curbuf, curwin;
    int buf, i, len, ret, win, width;
    list bk_list;
    string name, s;

    curbuf = inq_buffer();
    curwin = inq_window();

    bk_list = bookmark_list();
    len = length_of_list(bk_list);
    if (len <= 0)
        return "";
    buf = create_buffer("Bookmarks", NULL, TRUE);
    set_buffer(buf);
    for (i = 0; i < len; i += 4) {
        inq_names(NULL, NULL, name, bk_list[i + 1]);

        /* Get the line where the bookmark is */
        set_buffer(bk_list[i + 1]);
        save_position();
        move_abs(bk_list[i + 2], 1);
        s = trim(read());
        restore_position();
        set_buffer(buf);
        if (strlen(s) > 35)
        s = substr(s, 1, 32) + "...";
        sprintf(str, "%2d. %-14s %5d %5d: %s\n",
                bk_list[i], name, bk_list[i + 2], bk_list[i + 3], s);
        insert(str);
    }

    delete_line();
    sort_buffer();
    width = inq_line_length();
    if (width < 48)
        width = 48;
    win = sized_window(inq_lines()+1, width,
                "<Alt-D> to delete; <Enter> to select bookmark");
    ret = select_buffer(buf, win, SEL_NORMAL, book_keys());
    if (ret >= 0) {
        beginning_of_line();
        ret = atoi(read(6));
    }
    delete_buffer(buf);
    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
    refresh();
    if (ret >= 0) {
        push_back(key_to_int("<Enter>"));
        return "" + ret;
    }

    return "";
}


void
book_keys(void)
{
    assign_to_key("<Alt-D>", "::book_delete");
}


static void
book_delete(void)
{
    int ret;

    beginning_of_line();
    ret = atoi(read(6));
    delete_bookmark(ret);
    delete_line();
}


/*
 *  Function to handle the user typing the 'cd' command at the
 *  Command: prompt. Allow user to use the select_file() macro to
 *  complete the directory name
 */
string
compl_cd(~string cmd)
{
    extern int do_push_back;
    string file;
    int n;

    if ((n = index(cmd, " ")) != 0)
        cmd = trim(substr(cmd, n + 1));
    else cmd = "";
    file = select_file(cmd, NULL, TRUE);
    if (file != "")
        do_push_back = FALSE;
    return "cd " + file;
}


/*
 *  Macro called when user hits <Alt-H> at a prompt on the
 *  command line. Popup a help window telling him/her what the
 *  options are
 */
void
prompt_help(void)
{
    string msg = inq_message();
    int i;

    if ((i = index(msg, ":")) > 0) {
        msg = substr(msg, 1, i - 1);
    }
    cshelp("cshelp", msg);
    refresh();
}


/*
 *  Macro called when user hits <Alt-W> at a prompt on the
 *  command line. Popup a window of the command history.
 */
string
compl_history(~string str)
{
    int curbuf, curwin, buf, win, width, len, idx, ret;
    list ht_list;

    if (hist_lines <= 0)
        return "";

    curbuf = inq_buffer();
    curwin = inq_window();

    set_buffer(hist_buf);
    goto_line(hist_first_line);
    for (idx = 0; idx < hist_lines; idx++) {
        ht_list += read();
        down();
    }

    buf = create_buffer("History List", NULL, TRUE);
    set_buffer(buf);
    for (idx = 0; idx < hist_lines; idx++) {
        if (width < (len = strlen(ht_list[idx])))
            width = len;
        insert(ht_list[idx]);
    }
    if (width < 48)
        width = 48;

    win = sized_window(inq_lines() + 1, width,
                "<Alt-D> to delete; <Enter> to select");
    ret = select_buffer(buf, win, SEL_NORMAL, "history_keys");

    if (ret >= 0) {
        beginning_of_line();
        str = rtrim(read());
    }
    delete_buffer(buf);
    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
    refresh();
    if (ret >= 0)
        push_back(key_to_int("<Enter>"));
    return str;
}


void
history_keys()
{
    assign_to_key("<Alt-D>", "::history_delete");
}


static void
history_delete()
{
    if (hist_lines <= 1) {
        beep();

    } else {
        int curbuf, line;

        curbuf = inq_buffer();
        beginning_of_line();
        inq_position(line);
        delete_line();
        set_buffer(hist_buf);
        goto_line(hist_first_line + (line-1));
        delete_line();
        hist_lines--;
        hist_no=0;
        refresh();
        set_buffer(curbuf);
    }
}

/*eof*/
