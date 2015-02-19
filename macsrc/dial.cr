/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dial.cr,v 1.7 2014/10/27 23:28:20 ayoung Exp $
 * Legacy Modem interface
 *
 *
 */

#include "grief.h"

#define TIMEOUT         60
#define PREFIX          "\rATDT9"

#if defined(__PROTOTYPES__)
extern void             dial(~string, ~string, ~int, ...);

static void             bix1(void);
static void             dial_hangup(void);
static int              dial_dial(~list, ~int);
static void             dial_send(string filename);
static void             dial_recv(void);
#endif

static int              modem_active;
static int              dial_buf;
static list             modem_strings = {
        {300,           "1\r"},
        {1200,          "5\r"},
        {2400,          "10\r"},
        {4800,          "3\r"},
        {9600,          "??"},
        {19200,         "??"},
        {"NO CARRIER",  "3\r"},
        {"BUSY",        "7\r"},
        {"NO ANSWER",   "8\r"},
        {"RING",        "2\r"},
        {"BLACKLISTED", "26\r"}
        };


/*
 *  Initialise modem description table
 */
void
main()
{
    unregister_macro(REG_EXIT, "dial_hangup");
    register_macro(REG_EXIT, "dial_hangup");
}


/*
 *  Example dial macro for calling BIX.
 *
 *  Note that this macro is censored before being distributed world-wide. So you'll have to fill
 *  in your own telephone number / passwords etc.
 *
 *  Please tailor to your own needs, but please keep copy safe otherwise
 *  future installations of GRIEF may destroy your private copy.
 */
void
bix()
{
    echo_line(E_LINE|E_TIME);                   // just Line number and time, reduces load.
    dial("BIX", "01-200-1353", 1200, bix1());
}


static void
bix1(void)
{
    insert_process("\r\rd1\r\r");
    wait_for(20, "NUI?");
    insert_process("npssdem033WHU\r");
    wait_for(20, "ADD?");
    insert_process("a931060015787\r");
    wait_for(20, "ame? ");
    insert_process("foxy\r");
}


/*
 *  dial ---
 */
void
dial(~string, ~string, ~int, ...)               // ~system-name, ~number, ~speed, ~callback
{
    string system_name, number;
    int speed, line, col;
    int cmds, retval;
    declare d;

    dial_hangup();

    if (!get_parm(0, system_name, "System to dial : "))
        return;
    if (!get_parm(1, number, "Number to dial : "))
        return;
    if (!get_parm(2, speed, "Speed : ", NULL, 1200))
        return;

    dial_buf = create_shell("/bin/sh", system_name + "-Window", PF_ECHO | PF_WAIT);
    assign_to_key("<Ctrl-S>", "dial_send");
    assign_to_key("<Ctrl-R>", "dial_recv");
    wait_for(10, "$", SF_NOT_REGEXP);
    insert("cu -l /dev/cua0 -t -s 1200\n");
    inq_position(line, col);
    set_process_position(line, col);
    insert_process("cu -l /dev/cua0 -t -s 1200\n");
    wait_for(10, "onnected\r");
    modem_active = TRUE;

    retval = dial_dial(modem_strings, PREFIX + number + "\r");
    if (retval < 0) {
        error("Dialup failed.");
        return;
    }

    d = modem_strings[retval][0];
    if (is_string(d)) {
        error(d);
        return;
    }

    if (d != speed) {
        error("Connected at wrong speed - %d.", d);
        return;
    }
    message("Connected at %d baud", speed);

    end_of_buffer();
    inq_position(line, col);
    set_process_position(line, col);
    get_parm(3, cmds);                          /* execute callback */
    connect(0);
    sh_line_mode();
}


static void
dial_hangup(void)
{
    if (!modem_active)
        return;

    sh_char_mode();
    message("Saying goodbye to modem.");
    attach_buffer(dial_buf);
    set_buffer(dial_buf);
    insert_process("\r~.\r");
    refresh();
    wait_for(5, "\\[EOT]");
    modem_active = FALSE;
}


static int
dial_dial(~list, ~int)
{
    list l, wlist;
    int n, retval;
    declare atom;
    string number;
    int line, col;

    if (!get_parm(0, l))
        return -1;
    if (!get_parm(1, number))
        return -1;

    while (TRUE) {
        atom = l[n];
        if (is_null(atom))
        break;
        wlist[n] = atom[1];
        ++n;
    }
    insert(number);
    refresh();
    inq_position(line, col);
    set_process_position(line, col);
    insert_process(number);
    connect(PF_WAIT);
    retval = wait_for(TIMEOUT, wlist);
    return retval;
}


static void
dial_send(string filename)
{
    if (filename == "")
        filename = select_file("*", "Send File", FALSE);
    if (filename == "")
        return;
    insert_process("\r~Csx -bkvv " + filename + "\n\n");
    refresh();
}


static void
dial_recv(void)
{
    insert_process("\r~Crz -bvv\n");
    refresh();
}

/*eof*/
