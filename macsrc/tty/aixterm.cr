/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: aixterm.cr,v 1.6 2014/10/22 02:34:38 ayoung Exp $
   aixterm window under IBM/aix.

   This is the terminal description file for AIXTERM, IBM's version of XTERM.
   The editor works quite nicely in this environment, since AIXTERM returns
   escape sequences for all Alt-key combinations, and many Ctrl-Key
   combinations that are not usually available on ASCII terminals.

 *
 */

/**********************************************************************

Example 'aixterm' resources allowing full keyboard access.


.....................................................................*/

#include "tty.h"

void
main()
{
    /*
     *  aixterm-old
     *      implies pc850 character set.
     */
    set_term_feature(TF_NAME, "aixterm");

    set_term_characters(
        218,                /* Top left of window */
        191,                /* Top right of window */
        192,                /* Bottom left of window */
        217,                /* Bottom right of window */
        179,                /* Vertical bar for window sides */
        196,                /* Top and bottom horizontal bar for window */
        194,                /* Top join */
        193,                /* Bottom join */
        197,                /* Window 4-way intersection */
        195,                /* Left hand join */
        180 );              /* Right hand join */

    set_term_features(
        NULL,               /* Sequence, clear 'n' spaces. */
        NULL,               /* Sequence. print characters with top bitset */
        NULL,               /* Sequence, insert-mode cursor. */
        NULL,               /* Sequence, overwrite-mode cursor. */
        NULL,               /* Sequence, insert-mode cursor (on virtual space). */
        NULL,               /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,               /* Sequence, print ESCAPE character graphically. */
        NULL,               /* Sequence, repeat last character 'n' times. */
        FALSE,              /* Boolean,  ESC [0m resets color. */
        FALSE,              /* Boolean,  terminal supports color. */
        "\x1B[%dC"              /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[001q",    "\x1B[002q",    "\x1B[003q",     "\x1B[004q",    "\x1B[005q",
            "\x1B[006q",    "\x1B[007q",    "\x1B[008q",     "\x1B[009q",    "\x1B[010q",
            "\x1B[011q",    "\x1B[012q"),

        SHIFT_F1_F12, quote_list(
            "\x1B[013q",    "\x1B[014q",    "\x1B[015q",    "\x1B[016q",    "\x1B[017q",
            "\x1B[018q",    "\x1B[019q",    "\x1B[020q",    "\x1B[021q",    "\x1B[022q",
            "\x1B[023q",    "\x1B[024q"),

        CTRL_F1_F12, quote_list(
            "\x1B[025q",    "\x1B[026q",    "\x1B[027q",    "\x1B[028q",    "\x1B[029q",
            "\x1B[030q",    "\x1B[031q",    "\x1B[032q",    "\x1B[033q",    "\x1B[034q",
            "\x1B[035q",    "\x1B[036q"),

        /*  <Alt-A>,        <Alt-B>,        <Alt-C>,        <Alt-D>,        <Alt-E>,
        //  <Alt-F>,        <Alt-G>,        <Alt-H>,        <Alt-I>,        <Alt-J>,
        //  <Alt-K>,        <Alt-L>,        <Alt-M>,        <Alt-N>,        <Alt-O>,
        //  <Alt-P>,        <Alt-Q>,        <Alt-R>,        <Alt-S>,        <Alt-T>,
        //  <Alt-U>,        <Alt-V>,        <Alt-W>,        <Alt-X>,        <Alt-Y>,
        //  <Alt-Z>
        */
        ALT_A_Z, quote_list(
            "\x1B[087q",    "\x1B[105q",    "\x1B[103q",    "\x1B[089q",    "\x1B[076q",
            "\x1B[090q",    "\x1B[091q",    "\x1B[092q",    "\x1B[081q",    "\x1B[093q",
            "\x1B[094q",    "\x1B[095q",    "\x1B[107q",    "\x1B[106q",    "\x1B[082q",
            "\x1B[083q",    "\x1B[074q",    "\x1B[077q",    "\x1B[088q",    "\x1B[078q",
            "\x1B[080q",    "\x1B[104q",    "\x1B[075q",    "\x1B[102q",    "\x1B[079q",
            "\x1B[101q"),

        /*  <Alt-0>,        <Alt-1>,        <Alt-2>,        <Alt-3>,        <Alt-4>,
        //  <Alt-5>,        <Alt-6>,        <Alt-7>,        <Alt-8>
        */
        ALT_0_9, quote_list(
            "\x1B[056q",   "\x1B[058q",     "\x1B[059q",    "\x1B[060q",    "\x1B[061q",
            "\x1B[062q",    "\x1B[063q",    "\x1B[064q",    "\x1B[065q",    "\x1B[066q"),

        /*  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        */
        KEYPAD_0_9, quote_list(
            "\x1B[139q",    "\x1B[146q",    "\x1B[B",       "\x1B[154q",    "\x1B[D",
            NULL,           "\x1B[C",       "\x1B[H",       "\x1B[A",       "\x1B[150q"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B[140q",    "\x1B[148q",    "\x1B[165q",    "\x1B[156q",    "\x1B[159q",
            NULL,           "\x1B[168q",    "\x1B[144q",    "\x1B[162q",    "\x1B[152q"),

        DEL,                "\x1B[P",
        BACK_TAB,           "\x1bOI",
        BACK_TAB,           "\x1b[Z"
        );
}

