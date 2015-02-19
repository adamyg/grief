/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: remember.cr,v 1.8 2014/10/27 23:28:26 ayoung Exp $
 *
 *  Keyboard macro library handler This file defines a replacement macro for the
 *  remember primitive which keeps track of the system buffers associated with the
 *  keyboard macros which have been defined and allow the user to save them and restore
 *  them.
 *
 *
 */

#include "grief.h"

static list         kb_list;
static string       kb_path;                /* Current path for the keyboard macro library */

void                keylib_keys(void);
static void         km_load(void);


/*
 *  Set up default path for keyboard macros
 */
void
main(void)
{
    kb_path = getenv("GRKBDPATH");          // keyboard library
    if (kb_path == "") {
        kb_path = getenv("GRTEMPLATE");     // template libary
        if (kb_path == "") {
            kb_path = inq_tmpdir();         // temporary directory
        }
    }
}


/*
 *  Function to define the keyboard library path. This is designed to be callable from
 *  the init code so we can set up a path on startup and save it on exit
 */
static string
km_path(string path)
{
    kb_path = path;
    return kb_path;
}


/*
 *  Replacement remember interface
 */
replacement int
remember()
{
    int km_id;

    /*
     *  If remember returns < 0, then we are beginning the remember.
     *  Otherwise we are terminating it.
     */
    if ((km_id = remember()) < 0) {
        return km_id;
    }
    kb_list += km_id;
    return km_id;
}


/*
 *  Function to display current keyboard macros and allows user
 *  to save them to files.
 */
void
keylib()
{
    int buf, curbuf;
    int win, curwin;
    int i, lines;
    string k, tmp;
    int km_buf;
    int exec_mac = -1;

    if (length_of_list(kb_list) == 0) {
        km_load();
        return;
    }

    curbuf = inq_buffer();
    curwin = inq_window();
    buf = create_buffer("Keyboard Macros", NULL, TRUE);
    set_buffer(buf);
    for (i = 0; i < length_of_list(kb_list);) {
        k = inq_keystroke_macro(kb_list[i], km_buf);
        if (strlen(k) > 45)
            k = substr(k, 1, 42) + "...";
        lines = inq_lines();
        set_buffer(buf);
        sprintf(tmp, "%d. %-45s %6d\n", ++i, k, lines);
        insert(tmp);
    }
    win = sized_window(inq_lines(), inq_line_length() + 1,
                int_to_key(ALT_E) + " - execute; " +
                int_to_key(ALT_L) + " - load; " +
                int_to_key(ALT_W) + " - write");
    select_buffer(buf, win, SEL_NORMAL, keylib_keys());

    /* Check to see if user wants to execute a macro */
    if (exec_mac >= 0) {
        playback(kb_list[exec_mac]);
    }

    delete_buffer(buf);
    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
}


void
keylib_keys(void)
{
    assign_to_key("<Alt-E>", "::km_exec");
    assign_to_key("<Alt-L>", "::km_load");
    assign_to_key("<Alt-W>", "::km_save");
}


static void
km_exec(void)
{
    extern int exec_mac;
    int line;

    inq_position(line);
    if (--line >= length_of_list(kb_list))
        return;
    exec_mac = line;
    push_back(key_to_int("<Enter>"));
}


static void
km_save(void)
{
    string filename;
    int curbuf;
    int line, buf;
    string km;

    inq_position(line);
    if (--line >= length_of_list(kb_list))
        return;

    if (get_parm(NULL, filename, "Key macro file: ", NULL, kb_path) <= 0) {
        message("");
        return;
    }
    curbuf = inq_buffer();
    km = inq_keystroke_macro(kb_list[line], buf);
    set_buffer(buf);
    output_file(filename + ".cr");
    write_buffer();

    /* Now write out the keystroke sequence into a .km file */
    buf = create_buffer(filename + ".km", NULL, TRUE);
    set_buffer(buf);
    insert(km);
    write_buffer();
    set_buffer(curbuf);
    delete_buffer(buf);
    message("Saved macro %s", filename);
}


/*
 *  Routine to read in a previously saved keyboard macro
 */
static void
km_load(void)
{
    string filename;
    string s;
    int buf, curbuf;

    /* Prompt user for macro name if not passed directly to function */
    if (get_parm(0, filename, "Load keystroke file: ", NULL, kb_path) <= 0)
        return;

    curbuf = inq_buffer();
    if (substr(filename, strlen(filename) - 2) != ".km")
        filename += ".km";

    /* Read keystroke file into a string */
    if ((buf = create_buffer("tmp", filename, TRUE)) == -1)
        return;

    set_buffer(buf);
    top_of_buffer();
    s = ltrim(read());
    set_buffer(curbuf);
    delete_buffer(buf);

    /* If no data in the keystroke then don't give it to CRISP */
    if (strlen(s) == 0) {
        error("No keysequence found.");
        return;
    }
    kb_list += load_keystroke_macro(s);
}


/*
 *  Function called when prompted to load a keystroke macro
 *   pop up a window of valid possible files.
 */
string
compl_keylib(string str)
{
    list kl_list;
    int ret;

    if (str == "")
        str = kb_path;
    kl_list = file_glob(str + "/*.km");
    if (length_of_list(kl_list) == 0) {
        beep();
        return str;
    }
    ret = select_list("Keystroke Files", "", 1, kl_list);
    refresh();
    if (ret >= 0)
        return kl_list[ret - 1];
    push_back(key_to_int("<Esc>"));
    return "";
}

/*end*/
