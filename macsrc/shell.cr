/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: shell.cr,v 1.14 2014/10/27 23:28:27 ayoung Exp $
 * External command shell support.
 *
 *
 */

#include "grief.h"

#define SHM_CHAR        0x01                /* Char/line mode. */
#define SHM_ECHO_OFF    0x02                /* ECHO turned off. */

#define MAX_HISTORY     20

#define LOCAL           1

extern string           sh_newline;         /* Newline character for buffer */
extern list             sh_history;
extern int              sh_start_line,      /* Place where input started */
                        sh_start_col,       /* Place where input cursor */
                        sh_end_line,
                        sh_end_col,         /* has got to */
                        sh_mode,
                        sh_cmd,             /* Where to place next command on history */
                        sh_flags,
                        sh_index;           /* Index to current command. */

#if defined(MSDOS) || defined(OS2)
#define BINSX           ".exe"
#define PATHCH          ";"
#else
#define PATHCH          ":"
#if defined(VMS)
#define BINSX           ".EXE"
#else
#define BINSX           ""
#endif
#endif

#if defined(__PROTOTYPES__)
static int              create_shell_buffer(string buffer_name);
static void             sh_popup(void);
static void             sh_select(void);
static void             sh_send_sig(void);
static void             sh_backspace(void);
static void             sh_delete(void);
static void             sh_insert(void);
static void             sh_next_cmd(void);
static void             sh_prev_cmd(void);
static void             sh_recall(int n, int inc);
static void             sh_paste(void);
static void             sh_send_line(void);
#endif

static int              sh_key_map, sh_id;
static list             sh_list = {
    "Character Mode",   "sh_select",    "message \"Characters directly sent to process buffer.\"",
    "Line Mode",        "sh_select",    "message \"Local editing of line before sending to buffer.\"",
    "Echo Toggle",      "sh_select",    "message \"Toggle between local echo.\""
    };


void
main(void)
{
    keyboard_push();
    keyboard_typeables();
    sh_key_map = inq_keyboard();

    assign_to_key("<Enter>",        "::sh_send_line");
    assign_to_key("<Backspace>",    "::sh_backspace");
    assign_to_key("<Del>",          "::sh_delete");
    assign_to_key("#127",           "::sh_delete");
    assign_to_key("<Ins>",          "::sh_paste");
    assign_to_key("<Ctrl-C>",       "::sh_send_sig");
    assign_to_key("<Ctrl-N>",       "::sh_next_cmd");
    assign_to_key("<Ctrl-O>",       "::sh_popup");
    assign_to_key("<Ctrl-P>",       "::sh_prev_cmd");
    keyboard_pop(1);
}


string
which(string cmd, ~string path)
{
    if (substr(cmd, 1, 1) != "/" && substr(cmd, 1, 1) != "~") {
        int i;
        list paths;

        if (path == "")
            path = getenv("PATH");
        paths = split(path, PATHCH);
        i = length_of_list(paths);

        while (i-- > 0) {
            int mode;
            string res;

            if (paths[i] == "") {
                res = cmd;
            } else {
                res = paths[i] + "/" + cmd;
            }

            if (stat(res, NULL, NULL, NULL, NULL, mode) == 0 && !(mode & S_IFDIR)) {
                cmd = res;
                break;
            }

#if defined(OS2) || defined(MSDOS)
            res = res + BINSX;
            if (stat(res, NULL, NULL, NULL, NULL, mode) == 0 && !(mode & S_IFDIR)) {
                cmd = res;
                break;
            }
#endif
        }
    }
    return cmd;
}


int
create_shellsub(string prog, string name)
{
    return create_shell(which(prog), name);
}


void
bash(void)
{
    create_shellsub("bash", "bash-Buffer");
}


void
sh(void)
{
    create_shellsub("sh", "sh-Buffer");
}


void
csh(void)
{
    create_shellsub("csh", "csh-Buffer");
}


void
tcsh(void)
{
    create_shellsub("tcsh", "tcsh-Buffer");
}


void
ksh(void)
{
    create_shellsub("ksh", "ksh-Buffer");
}


#if defined(OS2) || defined(MSDOS)
void
cmd(void)
{
    create_shellsub("cmd", "cmd-Buffer");
}
#endif


#if defined(OS2)
void
os2(void)
{
    create_shellsub("4os2", "4os2-Buffer");
}
#endif


int
create_shell(string shell_path, string buffer_name, ...)
{
    int buf = create_shell_buffer(buffer_name);

    /* Set defaults */
    sh_flags = 0;
    sh_index = 0;
    sh_mode = 0;
    sh_cmd = 0;
    sh_start_line = -1;
    sh_newline = "\n";

    /* Default newline character is \n but allow calling macro to override it. */
    get_parm(2, sh_newline);
    register_macro(REG_TYPED, "::sh_insert", LOCAL);
    connect(PF_ECHO|PF_NOINSERT, shell_path);
    use_local_keyboard(sh_key_map);
    return(buf);
}


static int
create_shell_buffer(string buffer_name)
{
    local string sh_newline;                    /* Newline character for buffer */
    local list sh_history;
    local int sh_start_line,                    /* Place where input started */
            sh_start_col,                       /* Place where input cursor */
            sh_end_line,
            sh_end_col,                         /* has got to */
            sh_mode,
            sh_cmd,                             /* Where to place next command on history */
            sh_flags,
            sh_index;                           /* Index to current command. */

    string tmpbuf;
    int buf;

    if (buffer_name == "") {
        buffer_name = "Shell-Buffer";
    }

    if (++sh_id > 1) {
        sprintf(tmpbuf, "-%d", sh_id);
        buffer_name += tmpbuf;
    }

    buf = create_buffer(buffer_name, NULL, 0);
    set_buffer(buf);
    set_buffer_flags(NULL, "ansi", "tabs,autoindent");
    attach_buffer(buf);
    inq_names(tmpbuf, NULL, NULL);

    make_local_variable(sh_start_line, sh_start_col, sh_end_line, sh_end_col,
            sh_mode, sh_index, sh_cmd, sh_history, sh_newline, sh_flags);
    return buf;
}


void
sh_char_mode(void)
{
    connect(PF_OVERWRITE);
    sh_mode |= SHM_CHAR;
}


void
sh_line_mode(void)
{
    connect(PF_ECHO|PF_NOINSERT);
    sh_mode = sh_mode & ~SHM_CHAR;
}


static void
sh_popup(void)
{
    int result;

    select_list("Options", "", 3, sh_list, SEL_NORMAL | SEL_TOP_OF_WINDOW);
    refresh();
    switch (result) {
    case 1:
        sh_char_mode();
        break;
    case 2:
        sh_line_mode();
        break;
    case 3:
        if (sh_mode & SHM_ECHO_OFF) {
            sh_mode &= ~SHM_ECHO_OFF;
        } else {
            sh_mode |= SHM_ECHO_OFF;
        }
        break;
    }
}


static void
sh_select(void)
{
    extern int result;

    inq_position(result);
    push_back(key_to_int("<Esc>"));
}



/*
 *  Send a SIGINT to the process.
 */
static void
sh_send_sig(void)
{
    send_signal(2);
}


static void
sh_backspace(void)
{
    if (sh_mode & SHM_CHAR) {
        insert_process("\008");
    }
    backspace();
}


static void
sh_delete(void)
{
    if (sh_mode & SHM_CHAR) {
        insert_process("\x7f");
    } else {
        delete_character();
    }
}


static void
sh_insert(void)
{
    if (sh_mode & SHM_CHAR) {
        prev_char();
        insert_process(read(1));
        if (sh_mode & SHM_ECHO_OFF) {
            delete_char();
        } else {
            next_char();
        }

    } else if (sh_start_line < 0) {
        sh_flags &= ~PF_OVERWRITE;
        connect(sh_flags);
        inq_position(sh_start_line, sh_start_col);
        --sh_start_col;
    }
}


static void
sh_next_cmd(void)
{
    sh_recall(sh_index, 1);
}


static void
sh_prev_cmd(void)
{
    sh_recall(sh_index, -1);
}


static void
sh_recall(int n, int inc)
{
    int l;
    declare atom;

    if (sh_mode & SHM_CHAR) {
        self_insert();
        return;
    }

    n += inc;

    if (n < 0)
        n = MAX_HISTORY - 1;
    if (n >= MAX_HISTORY)
        n = 0;

    atom = sh_history[n];
    if (!is_string(atom)) {
        atom = "";
        get_parm(0, n);
    }

    if (sh_start_line >= 0) {
        inq_position(l);
        if (sh_start_line != l) {
            delete_to_eol();
        } else {
            move_abs(sh_start_line, sh_start_col);
            delete_to_eol();
        }
    }

    inq_position(sh_start_line, sh_start_col);
    insert(atom);

    sh_index = n;
}


static void
sh_paste(void)
{
    end_of_line();
    if (sh_start_line < 0)
        inq_position(sh_start_line, sh_start_col);
    paste();
    sh_insert();
}


static void
sh_send_line(void)
{
    string line;
    int l, c, pl, pc;

    if (sh_mode & SHM_CHAR) {
        if (0 == (sh_mode & SHM_ECHO_OFF))
            insert("\n");
        inq_position(l, c);
        set_process_position(l, c);
        insert_process(sh_newline);
        return;
    }

    /*
     *  Try and locate where line came from. If on last line of buffer,
     *  then we can believe the sh_start_col field, otherwise we try
     *  and strip off the possible prompt at the beginning of the line.
     */
    inq_process_position(pl, pc);
    inq_position(l, c);
    if (l == pl) {
        move_abs(0, pc);
        line = read();
    }

    line = trim(line);
    if (line != "") {
        sh_history[sh_cmd] = line;
        ++sh_cmd;
        if (sh_cmd >= MAX_HISTORY)
            sh_cmd = 0;
        sh_index = sh_cmd;
    }

    sh_start_line = -1;
    end_of_buffer();
    insert("\n");
    inq_position(l, c);
    set_process_position(l, c);
    insert_process(line + "\n");

    sh_flags |= PF_OVERWRITE;
    connect(sh_flags);
}

/*eof*/
