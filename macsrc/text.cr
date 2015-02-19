/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: text.cr,v 1.17 2014/10/27 23:28:29 ayoung Exp $
 * Text handling / utilities macro.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
static void             grep_exec(string grepcmd, string patt, string files);
static void             grep_list(void);
static void             grep_edit(void);
static string           grep_prompt(string msg, string responses);
static void             grep_translate(void);
static void             grep_next(void);
static void             grep_prev(void);
static void             grep_display(void);
#endif

static int              grep_buf, grep_line_no = 1;
static string           grep_old_file, grep_files, grep_pattern;


void
main(void)
{
    grep_buf = create_buffer("GREP-Buffer", NULL, 1);
    grep_pattern = "GREP pattern: ";
    grep_files = "GREP files: ";
}


/*
 *  wc ---
 *      Macro to report how many words in current buffer, and display average word length.
 */
void
wc(void)
{
    string tmp_file, buf;
    int curbuf, tmp_buf, num_words, num_chars;

    save_position();
    top_of_buffer();
    drop_anchor(MK_LINE);
    end_of_buffer();

    sprintf(tmp_file, "%s/wc%06d.cr", inq_tmpdir(), getpid());
    write_block(tmp_file);
    tmp_file = fixslash(tmp_file);
    sprintf(buf, "wc -wc %s", tmp_file);
    message("Counting number of words...");
    tmp_buf = perform_command(buf, "wc-buffer");

    curbuf = inq_buffer();
    set_buffer(tmp_buf);
    end_of_buffer();
    beginning_of_line();
    re_search(NULL, "[0-9]");
    num_words = atoi(read());
    re_search(0, " \\c[0-9]");
    num_chars = atoi(read());

    message("Words: %d Avg. len: %d", num_words, num_chars / num_words);
    remove(tmp_file);

    set_buffer(curbuf);
    delete_buffer(tmp_buf);
    attach_buffer(curbuf);
    restore_position();
}


/*
 *  grep ---
 *      text search using 'grep'.
 */
void
grep(~string, ~string)
{
    string options, pattern, files;
    list args = arg_list();

    if (length_of_list(args)) {
        //
        //  command mode
        //
        string value;
        int ch;

        if ((ch = getopt(value, "iv", NULL, args, "grep")) > 0) {
            do {
                switch(ch) {
                case 'i':           // ignore case
                case 'v':           // invert
                    options += format("-%c ", ch);
                    break;
                default:
                    error("%s", value);
                    return;
                }
            } while ((ch = getopt(value)) > 0);
        }

        args = split_arguments(value);
        if (length_of_list(args) < 1) {
            error("grep: missing search pattern");
            return;

        } else if (length_of_list(args) < 2) {
            error("grep: missing file specification");
            return;

        } else if (length_of_list(args) > 2) {
            error("grep: unexpected arguments %s ... ", args[2]);
            return;
        }

        pattern = args[0];
        files = args[1];

    } else {
        //
        //  interactive
        //
        if (get_parm(0, pattern, grep_pattern) <= 0 || "" == (pattern = trim(pattern))) {
            error("grep: missing search pattern");
            return;
        }

        if (get_parm(1, files, grep_files) <= 0 || "" == (files = trim(files))) {
            error("grep: missing file specification");
            return;
        }
    }

    grep_exec("grep -n", pattern + options, files);
}


/*
 *  fgrep ---
 *      text search using 'fgrep'.
 */
void
fgrep(~string, ~string)
{
    string pattern, files;

    if (get_parm(0, pattern, grep_pattern) <= 0 || "" == (pattern = trim(pattern))) {
        error("fgrep: missing search pattern");

    } else if (get_parm(1, files, grep_files) <= 0 || "" == (files = trim(files))) {
        error("fgrep: missing file specification");

    } else {
        grep_exec("fgrep -n", pattern, files);
    }
}


/*
 *  egrep ---
 *      text search using 'egrep'.
 */
void
egrep(~string, ~string)
{
    string pattern, files;

    if (get_parm(0, pattern, grep_pattern) <= 0 || "" == (pattern = trim(pattern))) {
        error("egrep: missing search pattern");

    } else if (get_parm(1, files, grep_files) <= 0 || "" == (files = trim(files))) {
        error("egrep: missing file specification");

    } else {
        grep_exec("egrep -n", pattern, files);
    }
}


/*
 *  grep_exec ---
 *      Macro to perform a grep and display buffers at
 *      location of matched strings.
 */
static void
grep_exec(string grepcmd, string pattern, string files)
{
    int lines, curbuf = inq_buffer();
    string buf;

    sprintf(buf, "%s \"%s\" %s", grepcmd, pattern, files);

    message("Locating text...");
    grep_buf = perform_command(buf, "GREP-Buffer", grep_buf);
    set_buffer(grep_buf);
    top_of_buffer();
    lines = inq_lines();                        /* result length */
    set_buffer_flags(NULL, NULL, ~BF_CHANGED);
    set_buffer(curbuf);

    assign_to_key("<Ctrl-N>", "::grep_next");   /* next match */
    assign_to_key("<Ctrl-P>", "::grep_prev");   /* prev match */
    assign_to_key("<Ctrl-B>", "::grep_list");   /* show results */

    if (lines <= 0) {
        error("No matching lines found.");
    } else {
        grep_list();                            /* list results */
    }
}


/*
 *  grep_list ---
 *      list the grep results.
 */
static void
grep_list(void)
{
    int     curbuf, win, lines;
    string  msg;

    curbuf = inq_buffer();
    set_buffer(grep_buf);                       /* previous results */
    top_of_buffer();
    lines = inq_lines();
    set_buffer(curbuf);
    if (lines <= 0) {                           /* empty */
        error("No matching lines found.");
        return;
    }

    keyboard_flush();
    grep_line_no = 1;
    msg = int_to_key(ALT_E) + " to edit, <Enter> to select, <Esc> to quit.";
    win = sized_window(lines + 1, -1, msg);
    grep_line_no =
        select_buffer(grep_buf, win, NULL,
                assign_to_key("<Alt-E>", "::grep_edit"),
                NULL, "help_display \"features/grep.hlp\" \"Help on Grep Window\"",
                grep_line_no, FALSE);
    if (grep_line_no < 1) {                     /* Esc ? */
        set_buffer(curbuf);
        return;
    }
    grep_display();
}


/*
 *  grep_edit ---
 *      <AlT-E> key stroke handler, edit the results. Allows the current buffer to be
 *      edited and we return when <Esc> key is hit. Upon which the user to prompted
 *      with 'tca'.
 *
 *          translate       Translate the result to the original file.
 *          continue        Continue editing.
 *          abort           Abort within translating.
 */
static void
grep_edit(void)
{
    string answer;

    raise_anchor();
    while (1) {
        message("Type <Esc> to terminate translate mode.");
        select_editable();
        answer = grep_prompt("GREP translate/continue/abort ? ", "tca");
        if (answer != "c") {
            break;
        }
    }

    if (answer == "t") {
        grep_translate();
    }
}


static string
grep_prompt(string msg, string responses)
{
    string answer;

    answer = "9";
    while (index(responses, answer) == 0) {
        get_parm(NULL, answer, msg, 1);
        answer = lower(answer);
    }
    return answer;
}


static void
grep_translate(void)
{
    int line, i, j, buf, line_no;
    string lbuf, drive, filename;

    buf = inq_buffer();
    top_of_buffer();
    line = 1;
    while (line < inq_lines()) {
        lbuf = read();
        i = index(lbuf, ":");
        drive = "";

        /*
         *  DOS & OS / 2 need special handling since ':' may (and probably will) occur as part of the filename
         */
#if defined(OS2) || defined(MSDOS)
        if (i == 2) {
            drive = substr(lbuf, 1, 2);
            lbuf = substr(lbuf, 3);
        }
#endif
        filename = drive + substr(lbuf, 1, i - 1);

        lbuf = substr(lbuf, i + 1);
        j = index(lbuf, ":");
        line_no = atoi(substr(lbuf, 1, j - 1));
        lbuf = substr(lbuf, j + 1);
        if (edit_file(filename) != -1) {
            move_abs(line_no, 0);
            delete_line();
            insert(lbuf);
            message("File='%s' line=%d", filename, line_no);
        }
        set_buffer(buf);
        ++line;
        down();
    }
    attach_buffer(buf);
}


static void
grep_next(void)
{
    ++grep_line_no;
    grep_display();
}


static void
grep_prev(void)
{
    --grep_line_no;
    grep_display();
}


static void
grep_display(void)
{
    string text, filename;
   int lines, colon, curbuf, line_no;

    curbuf = inq_buffer();
    set_buffer(grep_buf);
    lines = inq_lines();

    if (grep_line_no < 1 || grep_line_no > lines) {
        grep_line_no = 1;
        set_buffer(curbuf);
        message("No more matching lines.");

    } else {
        move_abs(grep_line_no, 1);
        text = read();

        colon = index(text, ":");
#if defined(OS2) || defined(MSDOS)
        if (colon != 2) {
#endif
            filename = substr(text, 1, colon - 1);
#if defined(OS2) || defined(MSDOS)
        } else {
            filename = substr(text, 1, 2);
            text = substr(text, 3);
            colon = index(text, ":");
            filename += substr(text, 1, colon - 1);
        }
#endif
        line_no = atoi(substr(text, colon + 1));
        if (filename == grep_old_file) {
            set_buffer(curbuf);

        } else if (edit_file(filename) == -1) {
            set_buffer(curbuf);
            return;

        } else {
            top_of_buffer();
            grep_old_file = filename;
        }
        move_abs(line_no, 1);
        message("<Ctrl-P> prev, <Ctrl-N> next, <Ctrl-B> list.");
    }
}

/*eof*/
