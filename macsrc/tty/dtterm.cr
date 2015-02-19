/* -*- mode: cr; tabs: 8; -*- */
/* $Id: dtterm.cr,v 1.12 2014/10/22 02:34:40 ayoung Exp $
 * terminal description for the dterm window under X.
 *
 *
 */

/**********************************************************************

Example 'dtterm' resources:

.Xdefaults to gain access to the Key Cursor and Home/End keys whilst
running under dtterm.

Dtterm*savelines:		100s
Dtterm*appKeypadDefault:	True
Dtterm*sunFunctionKeys:		False
Dtterm*kshMode:			True

!Note:  under dtterm #override CD1.2 with not work, instead use one must
!       use #replace (bug id 1232154).
!
!	Ctrl<Key>osfBeginLine:		string("Ctrl  osfBeginLine")
!	Ctrl<Key>osfEndLine:		string("Ctrl  osfEndLine")
!	Ctrl<Key>Home:			string("Ctrl  Home")
!	Ctrl<Key>End:			string("Ctrl  End")
!	Shift<Key>osfPageUp:		string("Shift osfPageUp")
!	Shift<Key>osfPageDown:		string("Shift osfPageDown")
!	Shift<Key>osfDelete:		string("Shift osfDelete")
!	Shift<Key>osfInsert:		string("Shift osfInsert")
!	Shift<Key>osfBeginLine:		string("Shift osfBeginLine")
!	Shift<Key>osfEndLine:		string("Shift osfEndLine")
!	Shift<Key>Home:			string("Shift Home")
!	Shift<Key>End:			string("Shift End")
!	Shift<Key>osfLeft:		string("Shift Left")
!	Shift<Key>osfRight:		string("Shift Right")
!	<Key>osfPageUp:			string("osfPageUp" )
!	<Key>osfPageDown:		string("osfPageDown")
!	<Key>osfDelete:			string("osfDelete")
!	<Key>osfInsert:			string("osfInsert")
!
! CDE 1.2
!	<Key>Home:			string(\033[1~)
!	<Key>End:			string(\033[4~)
!
! CDE 1.3+
!	<Key>osfBeginLine:		string(\033[1~)
!	<Key>osfEndLine:		string(\033[4~)
!	<Key>osfPageUp:			string(\033[5~)
!	<Key>osfPageDown:		string(\033[6~)
!	<Key>osfHelp:			string(\033[11~)
!
Dtterm*translations:		#override \
	Ctrl<Key>osfLeft:		string(\033[90~) \n\
	Ctrl<Key>osfRight:		string(\033[91~) \n\
	Ctrl<Key>osfUp:			string(\033[92~) \n\
	Ctrl<Key>osfDown:		string(\033[93~) \n\
	<Key>osfBeginLine:		string(\033[1~) \n\
	<Key>osfEndLine:		string(\033[4~) \n\
	<Key>osfPageUp:			string(\033[5~) \n\
	<Key>osfPageDown:		string(\033[6~) \n\
	Meta<Key>F1:			string(\033[224A) \n\
	Meta<Key>F2:			string(\033[225A) \n\
	Meta<Key>F3:			string(\033[226A) \n\
	Meta<Key>F4:			string(\033[227A) \n\
	Meta<Key>F5:			string(\033[228A) \n\
	Meta<Key>F6:			string(\033[229A) \n\
	Meta<Key>F7:			string(\033[230A) \n\
	Meta<Key>F8:			string(\033[231A) \n\
	Meta<Key>F9:			string(\033[232A) \n\
	Meta<Key>F10:			string(\033[233A) \n\
	Ctrl<Key>F1:			string(\033[224C) \n\
	Ctrl<Key>F2:			string(\033[225C) \n\
	Ctrl<Key>F3:			string(\033[226C) \n\
	Ctrl<Key>F4:			string(\033[227C) \n\
	Ctrl<Key>F5:			string(\033[228C) \n\
	Ctrl<Key>F6:			string(\033[229C) \n\
	Ctrl<Key>F7:			string(\033[230C) \n\
	Ctrl<Key>F8:			string(\033[231C) \n\
	Ctrl<Key>F9:			string(\033[232C) \n\
	Ctrl<Key>F10:			string(\033[233C) \n\
	Shift<Key>F1:			string(\033[224S) \n\
	Shift<Key>F2:			string(\033[225S) \n\
	Shift<Key>F3:			string(\033[226S) \n\
	Shift<Key>F4:			string(\033[227S) \n\
	Shift<Key>F5:			string(\033[228S) \n\
	Shift<Key>F6:			string(\033[229S) \n\
	Shift<Key>F7:			string(\033[230S) \n\
	Shift<Key>F8:			string(\033[231S) \n\
	Shift<Key>F9:			string(\033[232S) \n\
	Shift<Key>F10:			string(\033[233S) \n\
	<Key>F1:			string(\033[11~) \n\

.....................................................................*/

#include "tty.h"
#include "tty_xterm.h"

#if defined(__PROTOTYPES__)
static void                dtstandard(void);
static void                dtcolour(void);
static void                dtmono(void);
#endif

void
main(void)
{
        /*
         *   Load support functions
         */
        set_term_feature(TF_NAME, "dtterm");
        if (! inq_macro("xterm_util"))
                load_macro("tty/xterm_util");

        /*
         *   Set characters used for extended graphics support when
         *   drawing windows.
         */
        set_term_characters(
                "l",            /* Top left of window.                          */
                "k",            /* Top right of window.                         */
                "m",            /* Bottom left of window.                       */
                "j",            /* Bottom right of window.                      */
                "x",            /* Vertical bar for window sides.               */
                "q",            /* Top and bottom horizontal bar for window.    */
                "w",            /* Top join.                                    */
                "v",            /* Bottom join.                                 */
                "n",            /* Window 4-way intersection.                   */
                "u",            /* Left hand join.                              */
                "t"             /* Right hand join.                             */
                );

        /*
         *  Define escape sequences used for special optimisations on output.
         */
        dtcolour();                             /* colour by default */
        dtstandard();

        /*
         *  Define keyboard layout for non-ascii characters.
         */
        set_term_keyboard(
                F1_F12, quote_list(             /* vt220 mode */
                        "\x1B[11~",  "\x1B[12~",  "\x1B[13~",  "\x1B[14~",
                        "\x1B[15~",  "\x1B[17~",  "\x1B[18~",  "\x1B[19~",
                        "\x1B[20~",  "\x1B[21~",  "\x1B[23~",  "\x1B[24~" ),

                F1_F12, quote_list(             /* sunFunctionKeys mode */
                        "\x1B[224z", "\x1B[225z", "\x1B[226z", "\x1B[227z",
                        "\x1B[228z", "\x1B[229z", "\x1B[230z", "\x1B[231z",
                        "\x1B[232z", "\x1B[233z", "\x1B[191z", "\x1B[192z"),

                SHIFT_F1_F12, quote_list(       /* sunFunctionKeys mode */
                        "\x1B[224S", "\x1B[225S", "\x1B[226S", "\x1B[227S",
                        "\x1B[228S", "\x1B[229S", "\x1B[230S", "\x1B[231S",
                        "\x1B[232S", "\x1B[233S", "\x1B[191S", "\x1B[192S"),

                CTRL_F1_F12, quote_list(        /* sunFunctionKeys mode */
                        "\x1B[224C", "\x1B[225C", "\x1B[226C", "\x1B[227C",
                        "\x1B[228C", "\x1B[229C", "\x1B[230C", "\x1B[231C",
                        "\x1B[232C", "\x1B[233C", "\x1B[191C", "\x1B[192C"),

                ALT_F1_F12, quote_list(         /* sunFunctionKeys mode */
                        "\x1B[224A", "\x1B[225A", "\x1B[226A", "\x1B[227A",
                        "\x1B[228A", "\x1B[229A", "\x1B[230A", "\x1B[231A",
                        "\x1B[232A", "\x1B[233A", "\x1B[191A", "\x1B[192A"),

        /* Cursor keys */
                KEY_PAGEUP,     "\x1b[5~",      /* vt220 mode */
                KEY_PAGEDOWN,   "\x1b[6~",
                KEY_INS,        "\x1b[2~",
                KEY_DEL,        "\x7f",

                KEY_PAGEUP,     "\x1b[5z",      /* sunFunctionKeys mode */
                KEY_PAGEDOWN,   "\x1b[6z",
                KEY_INS,        "\x1b[2z",

                KEY_UP,         "\x1bOA",       /* application mode */
                KEY_DOWN,       "\x1bOB",
                KEY_LEFT,       "\x1bOD",
                KEY_RIGHT,      "\x1bOC",

                /*default*/                     /* normal mode (use ansi arrows) */

        /* Dtterm*translations (see above) */
                CTRL_KEYPAD_4,  "\x1b[90~",
                CTRL_KEYPAD_6,  "\x1b[91~",
                CTRL_KEYPAD_8,  "\x1b[92~",
                CTRL_KEYPAD_2,  "\x1b[93~",
                ALT_KEYPAD_4,   "\x1b[94~",
                ALT_KEYPAD_6,   "\x1b[95~",
                ALT_KEYPAD_8,   "\x1b[96~",
                ALT_KEYPAD_2,   "\x1b[97~",
                KEY_HOME,       "\x1b[1~",
                KEY_END,        "\x1b[4~",

                BACK_TAB,       "\x1b\x09",
                BACK_TAB,       "\x1bOI",

        /* Assumes dtterm (kshMode),
                *  Under kshMode. a key pressed with the extended modifier bit
                *  set will generate an escape character followed by the character
                *  generated by the un-extended keystroke.
                *
                *  Notes:  Alt-0 breaks the application key mode codes
                */
                ALT_A_Z, quote_list(
                        "\x1ba", "\x1bb", "\x1bc", "\x1bd", "\x1be",    /* ALT-a..E */
                        "\x1bf", "\x1bg", "\x1bh", "\x1bi", "\x1bj",    /* ALT-f..J */
                        "\x1bk", "\x1bl", "\x1bm", "\x1bn", "\x1bo",    /* ALT-k..O */
                        "\x1bp", "\x1bq", "\x1br", "\x1bs", "\x1bt",    /* ALT-p..T */
                        "\x1bu", "\x1bv", "\x1bw", "\x1bx", "\x1by",    /* ALT-u..Y */
                        "\x1bz"),

                ALT_A_Z, quote_list(
                        "\x1bA", "\x1bB", "\x1bC", "\x1bD", "\x1bE",    /* ALT-a..E */
                        "\x1bF", "\x1bG", "\x1bH", "\x1bI", "\x1bJ",    /* ALT-f..J */
                        "\x1bK", "\x1bL", "\x1bM", "\x1bN", "\x1b0",    /* ALT-k..O */
                        "\x1bP", "\x1bQ", "\x1bR", "\x1bS", "\x1bT",    /* ALT-p..T */
                        "\x1bU", "\x1bV", "\x1bW", "\x1bX", "\x1bY",    /* ALT-u..Y */
                        "\x1bZ"),

                ALT_0_9, quote_list(
                        "\x1b0", "\x1b1", "\xb12", "\xb13", "\xb14",    /* ALT-0..4 */
                        "\x1b5", "\x1b6", "\xb17", "\xb18", "\xb19"),   /* ALT-5..9 */

        /* Keypad in application mode,
                * Note: Must be defined after ALt-O to resolve conflict
                */
                KEYPAD_DIV,     "\x1bOo",       /* Keypad-/ */
                KEY_UNDO,       "\x1bOj",       /* Keypad-* */
                KEY_HOME,       "\x1bOw",       /* Keypad-7 */
                KEY_UP,         "\x1bOx",       /* Keypad-8 */
                KEY_PAGEUP,     "\x1bOy",       /* Keypad-9 */
                KEY_LEFT,       "\x1bOt",       /* Keypad-4 */
                KEYPAD_5,       "\x1bOu",       /* Keypad-5 */
                KEY_RIGHT,      "\x1bOv",       /* Keypad-6 */
                KEY_END,        "\x1bOq",       /* Keypad-1 */
                KEY_DOWN,       "\x1bOr",       /* Keypad-2 */
                KEY_PAGEDOWN,   "\x1bOs",       /* Keypad-3 */
                KEYPAD_MINUS,   "\x1bOm",       /* Keypad-- */
                KEYPAD_PLUS,    "\x1bOk",       /* Keypad-+ */

                CTRL_KEYPAD_0_9, quote_list(
                        NULL,           NULL,           NULL,           "\x1B[232z",
                        "\x1B[208z",    NULL,           "\x1B[209z",    NULL,
                        NULL,           "\x1B[231z")
                );

        assign_to_key("\x1b,",          "objects word_left");           /* ALT-< */
        assign_to_key("\x1b<",          "objects word_left");
        assign_to_key("\x1b.",          "objects word_right");          /* ALT-> */
        assign_to_key("\x1b>",          "objects word_right");
        assign_to_key("<Keypad-5>",     "search_next");

        ansi_arrows();
}


static void
dtcolour(void)
{
        set_term_feature(TF_COLOR, TRUE);       /* Terminal supports color. */
        dtstandard();
}


static void
dtmono(void)
{
        set_term_feature(TF_COLOR, FALSE);      /* Terminal supports color. */
        dtstandard();
}


static void
dtstandard(void)
{
        set_term_feature(TF_PRINT_SPACE, "\x1b[%dX");
        set_term_feature(TF_CURSOR_RIGHT, "\x1b[%dC");

        set_term_feature(TF_GRAPHIC_MODE, "\x1b(0");
        set_term_feature(TF_TEXT_MODE, "\x1b(B");

        /*  Does work under dtterm :(
         *
         *      ?47h    Use Alternate Screen Buffer.
         *      ?46l    Use Normal Screen Buffer.
         *
         *  ESC =   Application Keypad (DECPAM).
         *  ESC >   Normal Keypad (DECPNM).
         */
        set_term_feature(TF_NOALTSCREEN, TRUE);
        set_term_feature(TF_INIT, "\x1b=");
        set_term_feature(TF_RESET, "\x1b>");

        set_term_feature(TF_DISABLE_INSDEL, TRUE);
}

