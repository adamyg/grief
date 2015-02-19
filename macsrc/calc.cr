/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: calc.cr,v 1.9 2014/10/27 23:28:18 ayoung Exp $
 * Popup calculator.
 *
 *
 */

#include "grief.h"

#define OP_STATE        1                       /* User just typed an operator */
#define NUM_STATE       2                       /* User is typing a number */
#define EQ_STATE        3                       /* User just typed an = */

#if defined(__PROTOTYPES__)
static void             calc_abs(void);
static void             calc_lsh(void);
static void             calc_sign(void);
static void             calc_clear(void);
static void             calc_mem(int action);
static void             calc_radix(int radix);
static void             calc_typed(void);
static int              calc_keydown(string ch);
static void             calc_accumulator(void);
#endif

static list             mem = quote_list(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

static int              calcx_dialog;
static int              calcx_active;

static int              accumulator, yaccumulator;
static int              radix, calc_state;
static string           op;

void
main()
{
    /*
     *  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
     *
     *  ( ) Hex (x) Dec ( ) Oct ( ) Bin
     *
     *
     */
    calcx_dialog =
        dialog_create(make_list(
            DLGA_TITLE,                     "Calculator",
            DLGA_CALLBACK,                  "::calc_callback",
            DLGA_KEYDOWN,                   TRUE,       // enable KEYDOWN events

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,

                DLGC_GROUP,
                    DLGA_ATTACH_LEFT,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,          "accumulator",
                        DLGA_ROWS,          1,
                        DLGA_COLS,          38,
                        DLGA_GREYED,
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_RIGHT,
                    DLGC_PUSH_BUTTON,
                        DLGA_ATTACH_LEFT,
                        DLGA_LABEL,         "&C ",
                        DLGA_NAME,          "clear",
                    DLGC_END,
                DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_W,
                    DLGC_RADIO_BUTTON,
                        DLGA_ORIENTATION,   1,          // 1=horizontal, 0=vertical
                        DLGA_ALIGN_W,
                        DLGA_NAME,          "radix",
                        DLGA_LABEL,         "Hex",
                        DLGA_LABEL,         "Dec",
                        DLGA_LABEL,         "Oct",
                        DLGA_LABEL,         "Bin",
                DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_TOP,
                    DLGA_ALIGN_E,
                    DLGC_PUSH_BUTTON,
                        DLGA_ATTACH_LEFT,
                        DLGA_LABEL,         "Bk",       // clear last digit == <Backspace>
                        DLGA_NAME,          "backspace",
                        DLGA_ACCELERATOR,   "<Backspace>",

                    DLGC_PUSH_BUTTON,
                        DLGA_ATTACH_LEFT,
                        DLGA_LABEL,         "CE",       // clear current value == <Delete>
                        DLGA_NAME,          "del",
                        DLGA_ACCELERATOR,   "<Del>",
                    DLGC_END,
                DLGC_END,


            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&AND",
                        DLGA_NAME,          "&",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&OR ",
                        DLGA_NAME,          "|",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&XOR",
                        DLGA_NAME,          "^",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&NOT",
                        DLGA_NAME,          "!",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "CM&P",
                        DLGA_NAME,          "~",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "A",
                        DLGA_NAME,          "a",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "7",
                        DLGA_NAME,          "7",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "4",
                        DLGA_NAME,          "4",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "1",
                        DLGA_NAME,          "1",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "0",
                        DLGA_NAME,          "0",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "B",
                        DLGA_NAME,          "b",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "8",
                        DLGA_NAME,          "8",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "5",
                        DLGA_NAME,          "4",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "2",
                        DLGA_NAME,          "2",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         ".",
                        DLGA_NAME,          ".",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_NW,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "C",
                        DLGA_NAME,          "c",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "9",
                        DLGA_NAME,          "9",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "6",
                        DLGA_NAME,          "6",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "3",
                        DLGA_NAME,          "3",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "%",
                        DLGA_NAME,          "%",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "D",
                        DLGA_NAME,          "d",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "/",
                        DLGA_NAME,          "/",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "*",
                        DLGA_NAME,          "*",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "-",
                        DLGA_NAME,          "-",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "+",
                        DLGA_NAME,          "+",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "E",
                        DLGA_NAME,          "e",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&MOD",
                        DLGA_NAME,          "%",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "&LSH",
                        DLGA_NAME,          "lsh",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "A&BS",
                        DLGA_NAME,          "abs",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "+/&-",     // sign
                        DLGA_NAME,          "sign",
                    DLGC_END,

                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_N,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "F",
                        DLGA_NAME,          "f",
                        DLGA_GREYED,
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "M&s",      // memory save
                        DLGA_NAME,          "ms",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "M&r",      // memory recall
                        DLGA_NAME,          "mr",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "M&+",      // memory append
                        DLGA_NAME,          "m+",
                    DLGC_PUSH_BUTTON,
                        DLGA_LABEL,         "=",
                        DLGA_DEFAULT_BUTTON,            // <Enter>
                    DLGC_END,
                DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,             "&Quit",
                    DLGA_NAME,              "quit",
                    DLGA_CANCEL_BUTTON,                 // <Esc>
                DLGC_END
            ));
}


void
calc(void)
{
    extern int window_offset, top_line;
    int curbuf, curwin, buf, win;

    curbuf = inq_buffer();
    curwin = inq_window();
    if ((buf = create_buffer("Calculator", NULL, 1)) == -1 ||
            (win = sized_window(11, 28, int_to_key(ALT_H) + " for Help")) == -1) {
        return;
    }

    set_buffer(buf);
    set_window(win);
    attach_buffer(buf);
    insert("\n");
    insert("---------------------------\n");
    insert("Hex  | Oct  | Dec  | rec   \n");
    insert("Not  | And  | oR   | sto   \n");
    insert(" a   |  b   |  c   |       \n");
    insert(" d   |  e   |  f   |  %    \n");
    insert(" 9   |  8   |  7   |  /    \n");
    insert(" 6   |  5   |  4   |  *    \n");
    insert(" 3   |  2   |  1   |  -    \n");
    insert(" 0   |  =   | Sign |  +    \n");
    top_of_buffer();

    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Esc>",          "exit");
    assign_to_key("B",              "::calc_radix 2");
    assign_to_key("O",              "::calc_radix 8");
    assign_to_key("D",              "::calc_radix 10");
    assign_to_key("H",              "::calc_radix 16");
    assign_to_key("r",              "::calc_mem 0");
    assign_to_key("s",              "::calc_mem 1");
    assign_to_key("N",              "::calc_keydown \"!\"");
    assign_to_key("S",              "::calc_sign");
    assign_to_key("C",              "::calc_clear");
    assign_to_key("<Alt-H>",        "cshelp \"features\" \"Calculator (infix)\"");
    register_macro(REG_TYPED,       "::calc_typed");

    radix = 10;
    calc_clear();
    window_offset += 9;
    top_line += 2;
    process();
    window_offset -= 9;
    top_line -= 2;

    keyboard_pop();
    unregister_macro(REG_TYPED,     "::calc_typed");
    delete_buffer(buf);
    delete_window();

    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
    refresh();
}


void
calcx()
{
    calcx_active = TRUE;
    dialog_run(calcx_dialog);
    calcx_active = FALSE;
}


static int
calc_callback(int ident, string name, int event, int p1, int p2)
{
    static int sequence = 0;
    list args = arg_list();

    UNUSED(ident, p2);

    switch (event) {
    case DLGE_INIT:
        message("calc_callback(%d; %s) : INIT", ++sequence, args);
        calc_accumulator();
        break;

    case DLGE_KEYDOWN:
        message("calc_callback(%d; %s) : KEYDOWN(%d)", ++sequence, args, p1);
        if (isprint(p1)) {
            string ch;
            sprintf(ch, "%c", p1);
            message("keydown: %c", p1);
            calc_keydown(ch);
        }
        break;

    case DLGE_COMMAND:
    case DLGE_BUTTON:
        message("calc_callback(%d; %s) : %s(%s)", ++sequence, args,
            (event == DLGE_COMMAND ? "COMMAND" : "BUTTON"), name);
        switch (name) {
        case "quit":
            dialog_exit();
            break;
        case "clear":
            calc_clear();
            break;
        case "backspace":
            accumulator /= radix;
            calc_accumulator();
            break;
        case "del":
            accumulator = 0;
            calc_accumulator();
            break;
        case "abs":
            calc_abs();
            break;
        case "lshs":
            calc_lsh();
            break;
        case "sign":
            calc_sign();
            break;
        case "ms":
            calc_mem(0);
            break;
        case "mr":
            calc_mem(1);
            break;
        case "m+":
            calc_mem(2);
            break;
        default:
            if ("" != name) {
                calc_keydown(name);
            }
            break;
        }
        break;

    case DLGE_CHANGE:
        switch (name) {
        case "radix":
            switch(p1) {
            case 0:                 // hex
                calc_radix(16);
                break;
            case 1:                 // decimal
                calc_radix(10);
                break;
            case 2:                 // octal
                calc_radix(8);
                break;
            case 3:                 // binary
                calc_radix(2);
                break;
            }
            break;
        }
        break;
    }
    return FALSE;
}



static void
calc_abs(void)
{
    accumulator = abs(accumulator);
    calc_accumulator();
}


static void
calc_lsh(void)
{
    accumulator <<= 1;
    calc_accumulator();
}


/*
 *  calc_sign ---
 *      Sign change
 */
static void
calc_sign(void)
{
    accumulator = -accumulator;
    calc_accumulator();
}


/*
 *  calc_clear  ---
 *      Clear the accumulator
 */
static void
calc_clear(void)
{
    accumulator = yaccumulator = 0;
    calc_state = OP_STATE;
    op = "+";
    calc_accumulator();
}


/*
 *  calc_mem ---
 *      Memory operations
 */
static void
calc_mem(int action)
{
    int i;

    switch (action) {
    case 0: {
            message("Recall Memory - type a digit.");
            while (-1 == (i = read_char())) {
                continue;
            }
            if (i >= '0' && i <= '9') {
                accumulator = mem[i - '0'];
                calc_accumulator();
                message("%d recalled from MEM%d", accumulator, i);
            }
            message("");
        }
        break;

    case 1: {
            message("Store in Memory - type a digit.");
            while (-1 == (i = read_char())) {
                continue;
            }
            if (i >= '0' && i <= '9') {
                i -= '0';
                mem[i] = accumulator;
                message("%d stored in MEM%d", accumulator, i);
            }
        }
        break;

    case 2:
        break;
    }
}


static void
calc_radix(int newradix)
{
    radix = newradix;
    calc_state = OP_STATE;
    calc_accumulator();
}


static void
calc_typed(void)
{
    string ch;
    int i;

    prev_char();
    ch = rtrim(read(1));
    delete_char();
    if (calc_keydown(ch)) {
        return;
    }

    i = atoi(ch, 0);
    switch (int_to_key(i)) {
    case "<Backspace>":
        backspace();
        beginning_of_line();
        right(3);
        re_search(SF_UNIX, "[~ ]");
        ch = rtrim(read());
        accumulator = atoi(ch);
        calc_accumulator();
    }
}


static int
calc_keydown(string ch)
{
    error("keydown: %s", ch);

    if (index("qQ", ch)) {
        if (calcx_active) {
            dialog_exit();
        } else {
            exit();
        }
        return TRUE;
    }

    if (index("0123456789abcdef", ch)) {
        switch (calc_state) {
        case OP_STATE:
            accumulator = 0;
            break;
        case EQ_STATE:
            yaccumulator = 0;
            accumulator = 0;
            break;
        }
        if (index("abcdef", ch)) {
            sprintf(ch, "%d", index("abcdef", ch) + 9);
        }
        accumulator = (accumulator * radix) + atoi(ch);
        calc_accumulator();
        calc_state = NUM_STATE;
        message("dec: %d, hex: %x", accumulator, accumulator);
        return TRUE;
    }

    if (index("+-*/%A&RX^!~=", ch)) {
        int old_acc;

        old_acc = accumulator;
        switch (op) {
        case "+": accumulator = yaccumulator + accumulator; break;
        case "-": accumulator = yaccumulator - accumulator; break;
        case "*": accumulator = yaccumulator * accumulator; break;
        case "/": accumulator = yaccumulator / accumulator; break;
        case "%": accumulator = yaccumulator % accumulator; break;
        case "A": case "&":
            accumulator = yaccumulator & accumulator;
            break;
        case "R": case "|":
            accumulator = yaccumulator | accumulator;
            break;
        case "X": case "^":
            accumulator = yaccumulator ^ accumulator;
            break;
        case "!":
            accumulator = !accumulator;
            break;
        case "~":
            accumulator = ~accumulator;
            break;
        }

        calc_accumulator();
        yaccumulator = accumulator;
        accumulator = old_acc;

        if (ch == "=") {
            calc_state = EQ_STATE;
        } else {
            calc_state = OP_STATE;
            op = ch;
        }

        error("op: %c, dec: %d hex: %x", ch, accumulator, accumulator);
        return TRUE;
    }
    return 0;
}


static void
calc_accumulator(void)
{
    int width = (calcx_active ? 32 : 20);

    string accumval;
    int radixval;

    switch (radix) {
    case 2:
        sprintf(accumval, "BIN %*b", width, accumulator);
        radixval = 3;
        break;
    case 8:
        sprintf(accumval, "OCT %*o", width, accumulator);
        radixval = 2;
        break;
    case 10:
        sprintf(accumval, "DEC %*d", width, accumulator);
        radixval = 1;
        break;
    case 16:
        sprintf(accumval, "HEX %*x", width, accumulator);
        radixval = 0;
        break;
    }

    if (calcx_active) {
        widget_set(calcx_dialog, "accumulator", accumval);
        widget_set(calcx_dialog, "radix", radixval);
    } else {
        top_of_buffer();
        delete_to_eol();
        insert(accumval);
    }
}

/*eof*/
