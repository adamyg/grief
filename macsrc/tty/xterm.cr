/* -*- mode: cr; indent-width: 4; tabs: 8; -*-
 * $Id: xterm.cr,v 1.21 2021/07/06 10:50:50 adamy Exp $
 * terminal description file for the xterm window under X11, an VT-100 like emulator.
 *
 *
 */

/**********************************************************************

Example 'xterm' resources allowing full keyboard access.

XTerm*font:			9x15
XTerm*appKeypadDefault:		true
XTerm*scrollKey:		true
Xterm*colormode:		on

XTerm*VT100.Translations:	#override \
	~Shift<Key>Home:		string(\033[1~)\n\
	~Shift<Key>End:			string(\033[4~)\n\
	~Shift<Key>Prior:		string(\033[5~)\n\
	~Shift<Key>Next:		string(\033[6~)\n\
	Shift<Key>Home:			scroll-back(100,page)\n\
	Shift<Key>End:			scroll-forw(100,page)\n\
	Shift<Key>Tab:			string("\033	")\n\
	~Shift<Key>Tab:			string("	")\n\
	Ctrl<Key>=:			string(\033[C=)\n\
	Ctrl<Key>-:			string(\033[C-)\n\
	Ctrl<Key>Insert:		string(\033[CR0~)\n\
	Ctrl<Key>Delete:		string(\033[CRz~)\n\
	Ctrl<Key>Home:			string(\033[CR7~)\n\
	Ctrl<Key>End:			string(\033[CR1~)\n\
	Ctrl<Key>Left:			string(\033[Ot)\n\
	Ctrl<Key>Down:			string(\033[Or)\n\
	Ctrl<Key>Up:			string(\033[Ox)\n\
	Ctrl<Key>Right:			string(\033[Ov)\n\
	Shift<Key>F1:			string(\033[SF1~)\n\
	Shift<Key>F2:			string(\033[SF2~)\n\
	Shift<Key>F3:			string(\033[SF3~)\n\
	Shift<Key>F4:			string(\033[SF4~)\n\
	Shift<Key>F5:			string(\033[SF5~)\n\
	Shift<Key>F6:			string(\033[SF6~)\n\
	Shift<Key>F7:			string(\033[SF7~)\n\
	Shift<Key>F8:			string(\033[SF8~)\n\
	Shift<Key>F9:			string(\033[SF9~)\n\
	Shift<Key>F10:			string(\033[SF10~)\n\
	Shift<Key>F12:			string(\033[SF12~)\n\
	Ctrl<Key>F1:			string(\033[CF1~)\n\
	Ctrl<Key>F2:			string(\033[CF2~)\n\
	Ctrl<Key>F3:			string(\033[CF3~)\n\
	Ctrl<Key>F4:			string(\033[CF4~)\n\
	Ctrl<Key>F5:			string(\033[CF5~)\n\
	Ctrl<Key>F6:			string(\033[CF6~)\n\
	Ctrl<Key>F7:			string(\033[CF7~)\n\
	Ctrl<Key>F8:			string(\033[CF8~)\n\
	Ctrl<Key>F9:			string(\033[CF9~)\n\
	Ctrl<Key>F10:			string(\033[CF10~)\n\
	Ctrl<Key>F11:			string(\033[CF11~)\n\
	Ctrl<Key>F12:			string(\033[CF12~)\n\
	Meta<Key>F1:			string(\033[MF1~)\n\
	Meta<Key>F2:			string(\033[MF2~)\n\
	Meta<Key>F3:			string(\033[MF3~)\n\
	Meta<Key>F4:			string(\033[MF4~)\n\
	Meta<Key>F5:			string(\033[MF5~)\n\
	Meta<Key>F6:			string(\033[MF6~)\n\
	Meta<Key>F7:			string(\033[MF7~)\n\
	Meta<Key>F8:			string(\033[MF8~)\n\
	Meta<Key>F9:			string(\033[MF9~)\n\
	Meta<Key>F10:			string(\033[MF10~)\n\
	Meta<Key>F11:			string(\033[MF11~)\n\
	Meta<Key>F12:			string(\033[MF12~)\n\
	Shift<Key>KP_0:			string(\033OP)\n\
	Shift<Key>KP_1:			string(\033OQ)\n\
	Shift<Key>KP_2:			string(\033OR)\n\
	Shift<Key>KP_3:			string(\033OS)\n\
	Shift<Key>KP_4:			string(\033OT)\n\
	Shift<Key>KP_5:			string(\033OU)\n\
	Shift<Key>KP_6:			string(\033OV)\n\
	Shift<Key>KP_7:			string(\033OW)\n\
	Shift<Key>KP_8:			string(\033OX)\n\
	Shift<Key>KP_9:			string(\033OY)\n\
	Ctrl<Key>KP_0:			string(\033Op)\n\
	Ctrl<Key>KP_1:			string(\033Oq)\n\
	Ctrl<Key>KP_2:			string(\033Or)\n\
	Ctrl<Key>KP_3:			string(\033Os)\n\
	Ctrl<Key>KP_4:			string(\033Ot)\n\
	Ctrl<Key>KP_5:			string(\033Ou)\n\
	Ctrl<Key>KP_6:			string(\033Ov)\n\
	Ctrl<Key>KP_7:			string(\033Ow)\n\
	Ctrl<Key>KP_8:			string(\033Ox)\n\
	Ctrl<Key>KP_9:			string(\033Oy)\n

!  Optional:
!	None<Key>KP_0:			string(\033[2~)\n\
!	None<Key>KP_1:			string(\033[OF)\n\
!	None<Key>KP_2:			string(\033[OB)\n\
!	None<Key>KP_3:			string(\033[6~)\n\
!	None<Key>KP_4:			string(\033[OD)\n\
!	None<Key>KP_5:			string(\033[OX)\n\
!	None<Key>KP_6:			string(\033[OC)\n\
!	None<Key>KP_7:			string(\033[OH)\n\
!	None<Key>KP_8:			string(\033[OA)\n\
!	None<Key>KP_9:			string(\033[5~)\n\
!	None<Key>F1:			string(\033[192z)\n\
!	None<Key>F2:			string(\033[193z)\n\
!	None<Key>F3:			string(\033[194z)\n\
!	None<Key>F4:			string(\033[195z)\n\
!	None<Key>F5:			string(\033[196z)\n\
!	None<Key>F6:			string(\033[197z)\n\
!	None<Key>F7:			string(\033[198z)\n\
!	None<Key>F8:			string(\033[199z)\n\
!	None<Key>F9:			string(\033[200z)\n\
!	None<Key>F10:			string(\033[201z)\n\
!	None<Key>F11:			string(\033[234z)\n\
!	None<Key>F12:			string(\033[235z)\n\
!!

.....................................................................*/

#include "tty.h"
#include "tty_xterm.h"

static void             xterm_locale();

void
main()
{
    string t_grterm, bterm, ostype, colorterm, sysname;
    int datype = -1, daversion = -1;
    list bterm_parts;

    /*
     *   Auto-configure with specialised term spec.
     */
    t_grterm = getenv("GRTERM");                /* term override */
    if (strlen(t_grterm)) {
        bterm_parts = split(t_grterm, "-");
        if (length_of_list(bterm_parts)) {
            if ("xterm" == bterm_parts[0]) {
                bterm = bterm_parts[1];         /* xterm-rxvt */
                lower(bterm);
            }
        }
    }

    ostype = getenv("OSTYPE");
    colorterm = getenv("COLORTERM");
    uname(sysname);

    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    //  TERM/GRTERM
    //
    if (bterm == "gnome" ||
            (bterm == "" && colorterm == "gnome-terminal")) {
        message("xterm_gnome");
        load_macro("tty/xterm_gnome");
        return;
    }

//TODO
//  if (bterm == "konsole" ||
//          getenv("KONSOLE_DCOP") != "") ||
//          getenv("KONSOLE_DBUS_SESSION") != "") {
//      message("xterm_konsole");
//      load_macro("tty/xterm_konsole");
//      return;
//  }

    if (bterm == "aix" || bterm == "aixterm" ||
            (bterm == "" && sysname == "AIX")) {
        message("xterm_aix");
        xterm_arrow();
        bterm = "aix";

    } else if (bterm == "linux" ||
                (bterm == "" && (ostype == "linux" || ostype == "linux-gnu" || sysname == "Linux"))) {
        message("xterm_linux");
        xterm_locale();
        load_macro("tty/xterm_linux");
        return;

    } else if (bterm == "mrxvt" || (bterm == "rxvt" && getenv("MRXVT_TABTITLE") != "")) {
        //
        //  MRXVT_TABTITLE
        //      Set to the initial tab title of each terminal. Notice that its value will not be
        //      altered if the user uses a shortcut or escape sequence to change the tab title;
        //      the user must modify manually after doing so.
        //
        message("xterm-mrxvt");                 /* 05/01/10 */
        load_macro("tty/xterm_mrxvt");
        return;

    } else if (bterm == "rxvt" || colorterm == "rxvt" || colorterm == "rxvt-xpm") {
        message("xterm-rxvt");
        load_macro("tty/xterm_rxvt");
        return;

    } else if (bterm == "rxvt" || bterm == "urxvt" || bterm == "mintty") {
        message("xterm-" + bterm);
        if (load_macro("tty/xterm_" + bterm)) {
            return;
        }
    }

    //  Device Attribute Tests
    //
    //      A number of xterm compatible terminals normally run under the generic TERM
    //      identifier "xterm" yet can be derived using the "VT/XTERM Device Attribute",
    //      these include.
    //
    //          Terminal            Type        Version
    //
    //          gnone-terminal      0/1         >= 1115
    //          PuTTY               0/1         136
    //          rxvt                82
    //          urxvt               83
    //          MinTTY              77
    //          xterm               -2(a)
    //
    //      Notes:
    //          a) Sourced from XTERM_VERSION not DA values.
    //
    bterm = "xterm";                            /* default */

    get_term_feature(TF_VT_DATYPE, datype);
    get_term_feature(TF_VT_DAVERSION, daversion);

    if (datype >= 0) {
        if ((datype | 1) == 1) {
            if (daversion >= 1115) {            /* gnome, hopefully */
                message("xterm_gnome");
                load_macro("tty/xterm_gnome");
                return;

            } else if (136 == daversion) {      /* MinTTY/PuTTY */
                bterm = "mintty";

            } else if (83 == datype) {          /* urxvt */
                message("xterm-urxvt");
//TODO          load_macro("tty/xterm_rxvt");
//              return;
            }

        } else if (82 == datype) {              /* rxvt */
            message("xterm-rxvt");
            load_macro("tty/xterm_rxvt");
            return;

        } else if (77 == datype) {              /* MinTTY */
            bterm = "mintty";

        }
    }

    set_term_feature(TF_NAME, bterm);
    xterm_graphic();

    if ("mintty" == bterm) {
        if (load_macro("tty/xterm_mintty")) {
            return;
        }
    }

    if (-2 == datype) {
        if (daversion > 0 && daversion < 166) {
            set_term_feature(TF_UNICODE_VERSION, "3.0.0");
        }
        xterm_locale();
    }

    /*
     *  Color suffix
     */
    if (index(colorterm, "-256") > 0) {
        xterm_256color();                       /* 256[color] */

    } else if (index(colorterm, "-88") > 0) {
        xterm_88color();                        /* 88[color] */

    } else if (index(colorterm, "-m") > 0) {
        xterm_mono();                           /* m[ono] */

    } else {
        xterm_color();                          /* 16 colour by default */
    }

    xterm_standard();

    /*
     *  Define keyboard layout for non-ascii characters.
     */
    set_term_keyboard(
        F1_F12, quote_list(                     /* standard */
            "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",
            "\x1b[15~",     "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
            "\x1b[20~",     "\x1b[21~",     "\x1b[23~",     "\x1b[24~"),

        F1_F12, quote_list(                     /* Sun Function Keys */
            "\x1b[192z",    "\x1b[193z",    "\x1b[194z",    "\x1b[195z",
            "\x1b[196z",    "\x1b[197z",    "\x1b[198z",    "\x1b[199z",
            "\x1b[200z",    "\x1b[201z",    "\x1b[234z",    "\x1b[235z"),

        SHIFT_F1_F12, quote_list(               /* VT100.Trans */
            "\x1b[SF1~",    "\x1b[SF2~",    "\x1b[SF3~",    "\x1b[SF4~",
            "\x1b[SF5~",    "\x1b[SF6~",    "\x1b[SF7~",    "\x1b[SF8~",
            "\x1b[SF9~",    "\x1b[SF10~",   "\x1b[SF11~",   "\x1b[SF12~"),

        CTRL_F1_F12, quote_list(                /* VT100.Trans */
            "\x1b[CF1~",    "\x1b[CF2~",    "\x1b[CF3~",    "\x1b[CF4~",
            "\x1b[CF5~",    "\x1b[CF6~",    "\x1b[CF7~",    "\x1b[CF8~",
            "\x1b[CF9~",    "\x1b[CF10~",   "\x1b[CF11~",   "\x1b[CF12~"),

        ALT_F1_F12, quote_list(                 /* VT100.Trans */
            "\x1b[MF1~",    "\x1b[MF2~",    "\x1b[MF3~",    "\x1b[MF4~",
            "\x1b[MF5~",    "\x1b[MF6~",    "\x1b[MF7~",    "\x1b[MF8~",
            "\x1b[MF9~",    "\x1b[MF10~",   "\x1b[MF11~",   "\x1b[MF12~"),

        ALT_A_Z, quote_list(                    /* X.Org (7bit) (lower case) */
            "\xC3\xA1",     "\xC3\xA2",     "\xC3\xA3",     "\xC3\xA4",     "\xC3\xA5",
            "\xC3\xA6",     "\xC3\xA7",     "\xC3\xA8",     "\xC3\xA9",     "\xC3\xAA",
            "\xC3\xAB",     "\xC3\xAC",     "\xC3\xAD",     "\xC3\xAE",     "\xC3\xAF",
            "\xC3\xB0",     "\xC3\xB1",     "\xC3\xB2",     "\xC3\xB3",     "\xC3\xB4",
            "\xC3\xB5",     "\xC3\xB6",     "\xC3\xB7",     "\xC3\xB8",     "\xC3\xB9",
            "\xC3\xBA" ),

        ALT_A_Z, quote_list(                    /* X.Org (7bit) (upper case) */
            "\xC3\x81",     "\xC3\x82",     "\xC3\x83",     "\xC3\x84",     "\xC3\x85",
            "\xC3\x86",     "\xC3\x87",     "\xC3\x88",     "\xC3\x89",     "\xC3\x8A",
            "\xC3\x8B",     "\xC3\x8C",     "\xC3\x8D",     "\xC3\x8E",     "\xC3\x8F",
            "\xC3\x90",     "\xC3\x91",     "\xC3\x92",     "\xC3\x93",     "\xC3\x94",
            "\xC3\x95",     "\xC3\x96",     "\xC3\x97",     "\xC3\x98",     "\xC3\x99",
            "\xC3\x9A" ),

        ALT_A_Z, quote_list(                    /* 8bit (lower case) Meta */
            "\x1ba",        "\x1bb",        "\x1bc",        "\x1bd",        "\x1be",
            "\x1bf",        "\x1bg",        "\x1bh",        "\x1bi",        "\x1bj",
            "\x1bk",        "\x1bl",        "\x1bm",        "\x1bn",        "\x1bo",
            "\x1bp",        "\x1bq",        "\x1br",        "\x1bs",        "\x1bt",
            "\x1bu",        "\x1bv",        "\x1bw",        "\x1bx",        "\x1by",
            "\x1bz" ),

        ALT_A_Z, quote_list(                    /* 8bit (upper case) Meta */
            "\x1bA",        "\x1bB",        "\x1bC",        "\x1bD",        "\x1bE",
            "\x1bF",        "\x1bG",        "\x1bH",        "\x1bI",        "\x1bJ",
            "\x1bK",        "\x1bL",        "\x1bM",        "\x1bN",        "\x1b0",
            "\x1bP",        "\x1bQ",        "\x1bR",        "\x1bS",        "\x1bT",
            "\x1bU",        "\x1bV",        "\x1bW",        "\x1bX",        "\x1bY",
            "\x1bZ"),

        ALT_A_Z, quote_list(                    /* ??? */
            "\xE1",         "\xE2",         "\xE3",         "\xE4",         "\xE5",
            "\xE6",         "\xE7",         "\xE8",         "\xE9",         "\xEa",
            "\xEb",         "\xEc",         "\xED",         "\xEe",         "\xEf",
            "\xF0",         "\xF1",         "\xF2",         "\xF3",         "\xF4",
            "\xF5",         "\xF6",         "\xF7",         "\xF8",         "\xF9",
            "\xFa"),

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del/.           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* Standard (Application mode) */
            "\x1b[2~",      "\x1bOw",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~",
            "\x1b[3~",      "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM"),

        ALT_0_9, quote_list(                    /* X.Org (7bit) */
            "\xC2\xB0",     "\xC2\xB1",     "\xC2\xB2",     "\xC2\xB3",     "\xC2\xB4",
            "\xC2\xB5",     "\xC2\xB6",     "\xC2\xB7",     "\xC2\xB8",     "\xC2\xB9" ),

        ALT_0_9, quote_list(                    /* XFree */
            "\x1b0",        "\x1b1",        "\x1b2",        "\x1b3",        "\x1b4",
            "\x1b5",        "\x1b6",        "\x1b7",        "\x1b8",        "\x1b9" ),

        ALT_0_9, quote_list(                    /* ??? */
            "\x1ba0",       "\x1ba1",       "\x1ba2",       "\x1ba3",       "\x1ba4",
            "\x1ba5",       "\x1ba6",       "\x1ba7",       "\x1ba8",       "\x1ba9"),

        ALT_0_9, quote_list(                    /* ??? */
            "\xB0",         "\xB1",         "\xC0",         "\xA4",         "\xA4",
            "\xB5",         "\xB6",         "\xB7",         "\xB8",         "\xB9" ),

        SHIFT_KEYPAD_0_9, quote_list(           /* VT100.Trans */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",
            "\x1bOT",       "\x1bOU",       "\x1bOV",       "\x1bOW",
            "\x1bOX",       "\x1bOY",       "\x1bOn",       NULL,
            NULL,           NULL,           NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* VT100.Trans  */
            "\x1bOp",       "\x1bOq",       "\x1bOr",       "\x1bOs",
            "\x1bOt",       "\x1bOu",       "\x1bOv",       "\x1bOw",
            "\x1bOx",       "\x1bOy",       NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,
            NULL),

        BACK_TAB,           "\x1b[Z",           /* rxvt and others */
        BACK_TAB,           "\x1b\t",           /* alt-form, older VT100.Trans */
        BACKSPACE,          "\x7f",

        /* VT100.Trans */
        ALT_MINUS,          "\xad",
        ALT_EQUALS,         "\xbd",

        __CTRL('_'),        "\x1b[C-",
        __CTRL('='),        "\x1b[C=",

        KEY_HOME,           "\x1b[1~",
        KEY_END,            "\x1b[4~",

        KEY_HOME,           "\x1b[7~",          /* Alt Home/End */
        KEY_END,            "\x1b[8~",

        CTRL_KEYPAD_4,      "\x1b[Ot",          /* left */
        CTRL_KEYPAD_6,      "\x1b[Ov",          /* right */
        CTRL_KEYPAD_8,      "\x1b[Ox",          /* up */
        CTRL_KEYPAD_2,      "\x1b[Or",          /* down */
        CTRL_KEYPAD_0,      "\x1b[CR0~",        /* ins */
        CTRL_KEYPAD_DEL,    "\x1b[CRz~",        /* del */
        CTRL_KEYPAD_7,      "\x1b[CR7~",        /* home */
        CTRL_KEYPAD_1,      "\x1b[CR1~"         /* end */
        );
}


static void
xterm_locale()
{
    string xtversion = getenv("XTERM_VERSION");
    string xtlocale = getenv("XTERM_LOCALE");

    if (strlen(xtversion) && strlen(xtlocale)) {
        set_term_feature(TF_ENCODING, xtlocale);
    }
}


void
xterm(void)
{
    /*NOTHING*/
}
