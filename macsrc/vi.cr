/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: vi.cr,v 1.8 2014/10/27 23:28:30 ayoung Exp $
 * VI emulation.
 *
 *
 */

#include "grief.h"

#define OP_NOTHING      0                       /* No inserts/deletes done yet.  */
#define OP_INSERT       1                       /* Last operation was an insert. */
#define OP_DELETE       2                       /* Last operation was a delete.  */

#define CHAR            0
#define CUT             1

int                     vi_word_left(void);
static void             z_cmd(void);
static void             replace(void);
static void             cmd_zero(void);
void                    vi_digit(int i);
static void             cmdcmd(string cmd_ch);
void                    remember_deletion(int type, int chars, int clear_insert);
static void             esc_cmd(void);
static void             cmd(string cmd_ch);
static void             dot_cmd(void);
static void             cmd1(string mac);
void                    vi_insert_mode(void);
void                    vi_command_mode(void);
void                    vi_open(void);
void                    vi_Open(void);
void                    vi_add(void);

void                    e(string file);
void                    r(string file);
void                    w(void);
void                    n(void);
void                    p(void);
void                    q(void);
void                    x(void);

void                    vi_join_line(void);
static void             Change(void);
static void             up_arrow_cmd(void);
static void             i_cmd(void);
static void             Z_cmd(void);
void                    show(void);
void                    search_fn(string str);
string                  assign_mark(int n);
string                  assign_region(int start_line, int start_col, int end_line, int end_col);


/* Registers
 */
static string           vir_insert;             /* Register containing last text inserted. */
static string           vir_delete;             /* Register containing last test deleted so */
                                                /* we can undo it */

static string           vi_start_fn = "^\\{";
static int              vi_doing_dot = FALSE;   /* If TRUE we are executing a '.' command. */
static int              vi_iline = 0;           /* Start line of insert. */
static int              vi_icol = 0;            /* Start column of insert. */
static int              vi_dline = 0;           /* Start line of deleted text. */
static int              vi_dcol = 0;            /* Start column of deleted text. */
static int              vi_delins = FALSE;      /* TRUE if doing a delete + insert, eg change */
static int              vi_report = 5;
static int              vi_mode = FALSE;
static int              vi_num = 1;
static int              vi_1st_ch = 1;

int                     _command_keymap, _insert_keymap;
string                  last_command;

void
main()
{
    keyboard_push();
    assign_to_key("<F1>",           "change_window");
    assign_to_key("<F2>",           "move_edge");
    assign_to_key("<F3>",           "create_edge");
    assign_to_key("<F4>",           "delete_edge");
    assign_to_key("<F7>",           "remember");
    assign_to_key("<F8>",           "playback");
    assign_to_key("<F9>",           "load_macro");
    assign_to_key("<F10>",          "execute_macro");
    assign_to_key("<Left Arrow>",   "::cmd \"h\"");
    assign_to_key("<Right Arrow>",  "::cmd \"l\"");
    assign_to_key("<Up Arrow>",     "::cmd \"k\"");
    assign_to_key("<Down Arrow>",   "::cmd \"j\"");
    assign_to_key("<PgUp>",         "page_up");
    assign_to_key("<PgDn>",         "page_down");
    assign_to_key("<Esc>",          "::esc_cmd");
    assign_to_key(" ",              "::cmd \" \"");
    assign_to_key(".",              "::cmd \".\"");
    assign_to_key("-",              "::cmd \"k\"");
    assign_to_key("+",              "::cmd \"j\"");
    assign_to_key("$",              "::cmd \"$\"");
    assign_to_key("~",              "::cmd \"~\"");
    assign_to_key("/",              "search__fwd");
    assign_to_key("?",              "search__back");
    assign_to_key("[",              "search_fn \"[\"");
    assign_to_key("]",              "search_fn \"]\"");
    assign_to_key(">",              "::cmdcmd \">\"");
    assign_to_key("\\<",            "::cmdcmd \"<\"");
    assign_to_key("\\%",            "::cmd \"%\"");
    assign_to_key("\\^",            "::up_arrow_cmd");
    assign_to_key("<Ctrl-B>",       "page_up");
    assign_to_key("<Ctrl-E>",       "edit_file");
    assign_to_key("<Ctrl-F>",       "page_down");
    assign_to_key("<Ctrl-G>",       "routines");
    assign_to_key("<Ctrl-H>",       "::cmd \"h\"");
    assign_to_key("<Ctrl-J>",       "::cmd \"j\"");
    assign_to_key("<Ctrl-L>",       "mark 3");
    assign_to_key("<Ctrl-M>",       "::cmd \"j\"");
    assign_to_key("<Ctrl-N>",       "edit_next_buffer");
    assign_to_key("<Ctrl-R>",       "redraw");
    assign_to_key("<Ctrl-T>",       "translate__fwd");
    assign_to_key("<Ctrl-U>",       "::cmd \"\025\"");
    assign_to_key("<Ctrl-W>",       "::vi_w");
    assign_to_key("\x1a",           "shell");
    assign_to_key("<Ctrl-]>",       "tag_function");
    assign_to_key("#127",           "::cmd \"h\"");
    assign_to_key("0",              "::cmd_zero");
    assign_to_key("1",              "vi_digit 1");
    assign_to_key("2",              "vi_digit 2");
    assign_to_key("3",              "vi_digit 3");
    assign_to_key("4",              "vi_digit 4");
    assign_to_key("5",              "vi_digit 5");
    assign_to_key("6",              "vi_digit 6");
    assign_to_key("7",              "vi_digit 7");
    assign_to_key("8",              "vi_digit 8");
    assign_to_key("9",              "vi_digit 9");
    assign_to_key("A",              "::cmd \"A\"");
    assign_to_key("B",              "objects word_left");
    assign_to_key("C",              "::Change");
    assign_to_key("D",              "delete_to_eol");
    assign_to_key("F",              "::cmd \"F\"");
    assign_to_key("G",              "::cmd1 \"goto_line\"");
    assign_to_key("H",              "top_of_window");
    assign_to_key("I",              "::i_cmd");
    assign_to_key("J",              "::cmd \"J\"");
    assign_to_key("L",              "end_of_window");
    assign_to_key("O",              "vi_Open");
    assign_to_key("P",              "::cmd \"P\"");
    assign_to_key("R",              "::cmd \"R\"");
    assign_to_key("W",              "objects word_right");
    assign_to_key("X",              "backspace");
    assign_to_key("Y",              "::cmdcmd \"Y\"");
    assign_to_key("Z",              "::Z_cmd");
    assign_to_key("a",              "::cmd \"a\"");
    assign_to_key("b",              "::cmd \"b\"");
    assign_to_key("c",              "::cmdcmd \"c\"");
    assign_to_key("d",              "::cmdcmd \"d\"");
    assign_to_key("f",              "::cmd \"f\"");
    assign_to_key("h",              "::cmd \"h\"");
    assign_to_key("i",              "::cmd \"i\"");
    assign_to_key("j",              "::cmd \"j\"");
    assign_to_key("k",              "::cmd \"k\"");
    assign_to_key("l",              "::cmd \"l\"");
    assign_to_key("n",              "::cmd \"n\"");
    assign_to_key("o",              "vi_open");
    assign_to_key("p",              "::cmd \"p\"");
    assign_to_key("r",              "::replace");
    assign_to_key("s",              "::cmd \"s\"");
    assign_to_key("u",              "::cmd \"u\"");
    assign_to_key("w",              "::cmd \"w\"");
    assign_to_key("x",              "::cmd \"x\"");
    assign_to_key("y",              "::cmdcmd \"y\"");
    assign_to_key("z",              "::z_cmd");
    assign_to_key(":",              "::cmd \":\"");
    _command_keymap = inq_keyboard();
    keyboard_pop(1);

    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Esc>",          "vi_command_mode");
    assign_to_key("^H",             "backspace");
    assign_to_key("#127",           "backspace");
    autoindent("y");
    _insert_keymap = inq_keyboard();
    keyboard_pop(1);

    keyboard_push(_command_keymap);
}


int
vi_word_left(void)
{
    return word_left("<|[= .()/\t]\\c[~= .()/\t]");
}


static void
z_cmd(void)
{
    switch (read_char())
    {
    case '.':
        set_center_of_window();
        break;
    case '-':
        set_bottom_of_window();
        break;
    case '\r':
        set_top_of_window();
        break;
    case '+':
        set_top_of_window();
        break;
    default:
        beep();
        break;
    }
    esc_cmd();
}


static void
replace(void)
{
    int ch = read_char();
    string buf;

    sprintf(buf, "%c", ch);
    vir_delete = read(1);
    vir_insert = buf;
    inq_position(vi_iline, vi_icol);
    inq_position(vi_dline, vi_dcol);
    delete_char(vi_num);
    insert(buf, vi_num);
    last_command = "r" + buf;
}


static void
cmd_zero(void)
{
    if (vi_1st_ch)
        beginning_of_line();
    else
        vi_digit(0);
}


void
vi_digit(int i)
{
    if (vi_1st_ch) {
        vi_num = 0;
        vi_1st_ch = FALSE;
    }
    vi_num = 10 * vi_num + i;
}


static void
cmdcmd(string cmd_ch)
{
    string buf, verb;
    int ch;

    if (upper(cmd_ch) == cmd_ch &&
            index("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", cmd_ch)) {
        cmd_ch = lower(cmd_ch);
        buf = cmd_ch;
    } else {
        ch = read_char();
        sprintf(buf, "%c", ch);
    }

    sprintf(last_command, "%d%s%c", vi_num, cmd_ch, ch);
    save_position();
    drop_anchor(MK_NONINC);
    if (index("#$/?%behjklw", buf) != 0)
        cmd(buf);
    else
    {
        switch (ch)
        {
        case '0':
            beginning_of_line();
            break;
        default:
            if (cmd_ch == buf) {
                raise_anchor();
                restore_position();
                beginning_of_line();
                save_position();
                drop_anchor(MK_NONINC);
                down(vi_num);
            } else {
                beep();
                raise_anchor();
                restore_position();
                return;
            }
            break;
        }
    }

    switch (cmd_ch)
    {
    case ">":
        up();
        execute_macro(">>");
        raise_anchor();
        verb = ">ed";
        break;
    case "<":
        up();
        execute_macro("<<");
        raise_anchor();
        verb = "<ed";
        break;
    case "c":
        remember_deletion(CUT, 0, !vi_doing_dot);
        if (ch == 'c') {
            insert("\n");
            up();
        }
        verb = "";
        vi_delins = TRUE;
        vi_insert_mode();
        break;
    case "d":
        remember_deletion(CUT, 0, TRUE);
        verb = "deleted";
        break;
    case "y":
        copy();
        verb = "yanked";
        break;
    }

    if (vi_num >= vi_report && verb != "")
        message("%d lines %s.", vi_num, verb);
    restore_position();
    esc_cmd();
}


void
remember_deletion(int type, int chars, int clear_insert)
{
    switch (type)
    {
    case CHAR:
        vir_delete = read(chars);
        inq_position(vi_dline, vi_dcol);
        delete_char(chars);
        break;
    case CUT:
        vir_delete = assign_mark(1);
        inq_marked(vi_dline, vi_dcol);
        cut();
        break;
    }
    if (clear_insert)
        vir_insert = "";
}



static void
esc_cmd(void)
{
    vi_num = 1;
    vi_1st_ch = 1;
}



static void
cmd(string cmd_ch)
{
    int i, col;
    int first_loop = TRUE;
    string s;

    set_calling_name("");
    if (cmd_ch == " ")
        cmd_ch = "l";
    if (index("%u:jhkl .#$bfnw", cmd_ch) == 0)
        sprintf(last_command, "%d%s", vi_num, cmd_ch);

    while (vi_num > 0) {
        switch (cmd_ch) {
        case "\025":
            undo();
            vi_num = 0;
            break;

        case ":":
            execute_macro();
            vi_num = 0;
            break;

        case ".":
            dot_cmd();
            vi_num = 0;
            break;

        case "/":
            search__fwd();
            break;

        case "?":
            search__back();
            break;

//      case "%":
//          brace();
//          vi_num = 0;
//          break;

        case "~":
            s = read(1);
            delete_char();
            if (upper(s) == s)
                insert(lower(s));
            else
                insert(upper(s));
            break;

        case "#":
            down(vi_num - 1);
            end_of_line();
            inq_position(NULL, col);
            vi_num = 0;
            break;

        case "$":
            down(vi_num - 1);
            end_of_line();
            inq_position(NULL, col);
            if (col != 1)
                left();
            vi_num = 0;
            break;

        case "a":
            right();
            vi_insert_mode();
            break;

        case "b":
            objects("word_left");
            break;

        case "f":
            sprintf(s, "%c", read_char());
            right();
            re_search(NULL, s);
            break;

        case "h":
            left(vi_num);
            vi_num = 0;
            break;

        case "i":
            if (!first_loop)
                vi_doing_dot = TRUE;
            vi_insert_mode();
            break;

        case "j":
            down(vi_num);
            vi_num = 0;
            break;

        case "k":
            up(vi_num);
            vi_num = 0;
            break;

        case "l":
            right(vi_num);
            vi_num = 0;
            break;

        case "n":
            search_next();
            break;

        case "p":
            down();
            beginning_of_line();
            paste();
            up();
            break;

        case "s":
            delete_char(vi_num);
            vi_insert_mode();
            vi_num = 0;
            break;

        case "u":
            if (vir_insert != "") {
                move_abs(vi_iline, vi_icol);
                delete_char(strlen(vir_insert));
            }

            if (vir_delete != "") {
                move_abs(vi_dline, vi_dcol);
                insert(vir_delete);
                move_abs(vi_dline, vi_dcol);
            }
#define SWAP(a,b,t) t = a; a = b; b = t;
            {
                int x;

                SWAP(vi_iline, vi_dline, x);
                SWAP(vi_icol, vi_dcol, x);
                SWAP(vir_insert, vir_delete, s);
                vi_num = 0;
            }
            break;

       case "w":
            objects("word_right");
            break;

       case "x":
            remember_deletion(CHAR, vi_num, TRUE);
            vir_insert = "";
            vi_num = 0;
            break;

       case "A":
            if (first_loop)
                end_of_line();
            else
                vi_doing_dot = TRUE;
            vi_insert_mode();
            break;

        case "J":
            vi_join_line();
            break;

        case "P":
            inq_position(i);
            beginning_of_line();
            paste();
            move_abs(i);
            break;

        case "R":
            insert_mode();
            vi_insert_mode();
            insert_mode();
            vi_num = 0;
            break;
        }
        first_loop = FALSE;
        --vi_num;
    }
    vi_num = 1;
    vi_1st_ch = 1;
}


static void
dot_cmd(void)
{
    int i;

    for (i = 1; i <= strlen(last_command); i++)
        push_back(key_to_int("\\" + substr(last_command, i, 1)));
    vi_doing_dot = TRUE;
}



static void
cmd1(string mac)
{
    string buf;

    if (vi_1st_ch && (mac == "goto_line"))
        buf = "end_of_buffer";
    else
        sprintf(buf, "%s %d", mac, vi_num);
    execute_macro(buf);
    vi_num = 1;
    vi_1st_ch = 1;
}



void
vi_insert_mode(void)
{
    if (vi_doing_dot) {
        insert(vir_insert);
        vi_doing_dot = FALSE;
        return;
    }
    keyboard_pop(1);
    keyboard_push(_insert_keymap);
    if (vi_mode)
        message("Insert mode.");
    inq_position(vi_iline, vi_icol);
}



void
vi_command_mode(void)
{
    int line, col;

    keyboard_pop(1);
    keyboard_push(_command_keymap);
    if (vi_mode)
        message("Command mode.");

    inq_position(line, col);
    vir_insert = assign_region(vi_iline, vi_icol, line, col);
    if (!vi_delins) {
        vir_delete = "";
        vi_delins = FALSE;
    }
}



void
vi_open(void)
{
    end_of_line();
    right();
    _indent();
    vi_insert_mode();
}



void
vi_Open(void)
{
    int line;

    inq_position(line);
    if (line == 1)  {
        beginning_of_line();
        insert("\n");
        up();
        vi_insert_mode();
    } else {
        up();
        vi_open();
    }
}


void
vi_add(void)
{
    right();
    vi_insert_mode();
}


void
e(string file)
{
    edit_file(file);
}


void
r(string file)
{
    read_file(file);
}


void
w(void)
{
    set_calling_name("");
    write_buffer();
}



void
n(void)
{
    edit_next_buffer();
}


void
p(void)
{
    previous_edited_buffer();
}


void
q(void)
{
    x();
}


void
x(void)
{
    set_calling_name("");
    exit();
}


void
vi_join_line(void)
{
    last_command = "vi_join_line";
    join_line();
}


static void
Change(void)
{
    push_back(key_to_int("\\#"));
    cmdcmd("c");
}



static void
up_arrow_cmd(void)
{
    beginning_of_line();
    re_search(NULL, "$|[~ \t]");
}



static void
i_cmd(void)
{
    up_arrow_cmd();
    vi_insert_mode();
}


static void
Z_cmd(void)
{
    int ch = read_char();

    if (ch != 'Z') {
        beep();
        return;
    }
    w();
    x();
}


void
show(void)
{
    buffer_list(1);
}


void
search_fn(string str)
{
    int ch1 = read_char();
    int ch = atoi(str, 0);

    if (ch != ch1) {
        beep();
        ch1 = 0x1b;
    }

    if (ch1 == 0x1b) {
        vi_num = 0;
        vi_1st_ch = TRUE;
        return;
    }

    if (ch1 == ']') {
        down();
        if (re_search(NULL, vi_start_fn) <= 0)
            end_of_buffer();
    } else {
        up();
        if (re_search(SF_BACKWARDS, vi_start_fn) <= 0)
            top_of_buffer();
    }
    set_center_of_window();
}


/* Routines to manipulate the scrap.
 */
string
assign_mark(int n)
{
    int start_line, start_col, end_line, end_col;

    if (inq_marked(start_line, start_col, end_line, end_col) == 0)
        return "";
    return assign_region(start_line, start_col, end_line, end_col + n);
}


string
assign_region(int start_line, int start_col, int end_line, int end_col)
{
    string scrap = "";

    save_position();
    move_abs(start_line, start_col);
    while (start_line < end_line) {
        scrap += read();
        start_line++;
        move_abs(start_line, 1);
    }

    if (start_line == end_line) {
        move_abs(0, end_col);
        save_position();
        insert("\n");
        up();
        move_abs(0, start_col);
        scrap += read(strlen(read()) - 1);
        restore_position();
        delete_char();
    } else {
        scrap += read(end_col);
    }

    restore_position();
    return scrap;
}

/*eof*/
