/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: edt.cr,v 1.5 2014/10/27 23:28:21 ayoung Exp $
 * EDT emulation
 *
 *
 *
 */

#include "grief.h"

#define GOLD         "OP"

/* Definitions for current direction */

#define ADVANCE      1
#define BACKUP       -1

extern void         dumb(void);

static int          edt_cmap;
static int          edt_direction = ADVANCE;
static int          edt_col;
static string       edt_undo_line, edt_undo_word, edt_undo_char;

void
main()
{
    /* Create a global character map so that control characters
     * display as they do in the real EDT
     */
    edt_cmap = create_char_map(0, NULL,
                    quote_list(
                        "<NUL>", "<SOH>", "<STX>", "<ETX>", "<EOT>", "<ENQ>", "<ACK>", "<BEL>",
                        "<BS>", "<HT>", "<NL>", "<VT>", "<FF>", "<CR>", "<SO>", "<SI>",
                        "<DLE>", "<DC1>", "<DC2>", "<DC3>", "<DC4>", "<NAK>", "<SYN>", "<ETB>",
                        "<CAN>", "<EM>", "<SUB>", "<ESC>", "<FS>", "<GS>", "<RS>", "<US>"),
                    quote_list('\t', 1));
}


void
edt()
{
    dumb();
    assign_to_key(GOLD + "OD",     "<<");
    assign_to_key(GOLD + "OC",     ">>");

    assign_to_key("OQ",            "help");            /* PF2 */
    assign_to_key(GOLD + "OQ",     "help");            /* PF2 */

    assign_to_key("OR",            "search_next");     /* PF3 */
    assign_to_key(GOLD + "OR",     "search__fwd");     /* PF3 */

    assign_to_key("OS",            "edt_pf4");

    assign_to_key(GOLD + "OS",     "edt_gold_pf4");

    assign_to_key("Ow",            "search_fwd \"\x0c\"");  /* 7 */
    assign_to_key(GOLD + "Ow",     "execute_macro");   /* 7 */

    assign_to_key("Ox",            "edt_k8");

    assign_to_key(GOLD + "Ox",     "page_direction");  /* 8 */
    assign_to_key("Oy",            "message \"Sorry, not supported\""); /* 9 */
    assign_to_key("Om",            "objects delete_word_right");    /* - */
    assign_to_key(GOLD + "Om",     "edt_k_minus");

    assign_to_key("Ot",            "edt_k4");
    assign_to_key(GOLD + "Ot",     "end_of_buffer");   /* 4 */
    assign_to_key("Ou",            "edt_k5");
    assign_to_key(GOLD + "Ou",     "top_of_buffer");   /* 5 */
    assign_to_key("Ov",            "cut");             /* 6 */
    assign_to_key(GOLD + "Ov",     "paste");           /* 6 */
    assign_to_key("Ol",            "edt_k_comma");
    assign_to_key(GOLD + "Ol",     "edt_gold_comma");

    assign_to_key("Oq",            "edt_k1");

    assign_to_key("Or",            "end_of_line");     /* 2 */
    assign_to_key("Os",            "edt_k3");
    assign_to_key("OM",            "copy");            /* Enter */
    assign_to_key("Op",            "edt_k0");
    assign_to_key(GOLD + "Op",     "edt_gold_0");
    assign_to_key("On",            "edt_dot");

    assign_to_key("^E",             "edit_file");
    assign_to_key("#127",           "backspace");
    assign_to_key("^H",             "edt_backspace");
    assign_to_key("^L",             "self_insert");
    assign_to_key("^W",             "write_buffer");
    assign_to_key("^U",             "undo");
    autoindent("y");
}


void
edt_pf4()
{
    edt_undo_line = read();
    delete_to_eol();
    delete_char();
}


void
edt_gold_pf4()
{
    insert(edt_undo_line);
}


void
edt_k8()
{
    if (edt_direction == ADVANCE)
        page_down();
    else
        page_up();
}


void
edt_k_minus()
{
    insert(edt_undo_word);
}


void
edt_k4()
{
    message("Advance.");
    edt_direction = ADVANCE;
}


void
edt_k5()
{
    message("Backup.");
    edt_direction = BACKUP;
}


void
edt_k_comma()
{
    edt_undo_char = read(1);
    delete_char();
}


void
edt_gold_comma()
{
    insert(edt_undo_char);
}


void
edt_k1()
{
    if (edt_direction == ADVANCE)
        objects("word_right");
    else
        objects("word_left");
}


void
edt_k3()
{
    if (edt_direction == ADVANCE)
        right();
    else
        left();
}


void
edt_k0()
{
    if (edt_direction == ADVANCE)
        down();
    else
        up();
}


void
edt_gold_0()
{
    save_position();
    beginning_of_line();
    insert("\n");
    restore_position();
}


void
edt_dot()
{
    message("Anchor dropped.");
    mark();
}


void
edt_backspace()
{
    inq_position(NULL, edt_col);
    if (edt_col == 1)
        up();
    else
        beginning_of_line();
}

/*eof*/
