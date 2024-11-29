/* -*- mode: cr; indent-width: 4; tabs: 8; -*-
 * $Id: xterm.cr,v 1.39 2024/11/24 12:26:42 cvsuser Exp $
 * terminal description file for the xterm window under X11, an VT-100 like emulator.
 *
 *
 */

/**********************************************************************

Example legacy 'xterm' resources allowing full keyboard access.

XTerm*font:			9x15
XTerm*appKeypadDefault: 	true
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
	Meta<Key>F1:			string(\033[AF1~)\n\
	Meta<Key>F2:			string(\033[AF2~)\n\
	Meta<Key>F3:			string(\033[AF3~)\n\
	Meta<Key>F4:			string(\033[AF4~)\n\
	Meta<Key>F5:			string(\033[AF5~)\n\
	Meta<Key>F6:			string(\033[AF6~)\n\
	Meta<Key>F7:			string(\033[AF7~)\n\
	Meta<Key>F8:			string(\033[AF8~)\n\
	Meta<Key>F9:			string(\033[AF9~)\n\
	Meta<Key>F10:			string(\033[AF10~)\n\
	Meta<Key>F11:			string(\033[AF11~)\n\
	Meta<Key>F12:			string(\033[AF12~)\n\
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

! Optional:
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

static int xterm_load(string name);
static void xterm_locale();

void
main(void)
{
    string bterm, ostype, colorterm, sysname;
    int datype = -1, daversion = -1;

    //  Auto-configure with specialised term spec.
    //
    //  Note, either GRTERM=xterm or TERM=xterm was set.
    //
    bterm = lower(getenv("GRTERM"));
    if (strlen(bterm)) {
        list termpts = split(bterm, "-");

        bterm = "";
        if (length_of_list(termpts) > 1) {
            if ("xterm" == termpts[0]) {        /* assume: xterm-xxx, for example xterm-rxvt */
                bterm = termpts[1];
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

    //  GRTERM/TERM
    //
    if (bterm == "gnome" ||
            (bterm == "" && colorterm == "gnome-terminal") /*legacy*/) {
        xterm_load("xterm-gnome");
        return;
    }

    if (bterm == "aix" || bterm == "aixterm" ||
            (bterm == "" && sysname == "AIX")) {
        //
        //  xterm-aixterm or AIX host.
        //
        message("xterm_aix");
        xterm_arrow();
        bterm = "aix";

    } else if (bterm == "linux") {
        //
        //  xterm-linux or linux host.
        //
        xterm_locale();
        xterm_load("xterm-linux");
        return;

    } else if (bterm == "mrxvt" || (bterm == "rxvt" && getenv("MRXVT_TABTITLE") != "")) {
        //
        //  xterm-mrxvt or xterm-rxvt
        //
        //  MRXVT_TABTITLE
        //      Set to the initial tab title of each terminal. Notice that its value will not be
        //      altered if the user uses a shortcut or escape sequence to change the tab title;
        //      the user must modify manually after doing so.
        //
        xterm_load("xterm-mrxvt");
        return;

    } else if (bterm == "rxvt" || bterm == "urxvt" ||
                colorterm == "rxvt" || colorterm == "rxvt-xpm") {
        //
        //  term-rxvt, term-urxvt or
        //  COLORTERM=rxvt/rxvt-xpm
        //
        xterm_load("xterm-rxvt");
        return;

    } else if (bterm == "mrxvt" || bterm == "mintty") {
        //
        //  xterm-mrxvt or xterm-mintty
        //
        if (xterm_load("xterm_" + bterm)) {
            return;
        }
    }

    //TODO/TEST
    //  COLORTERM=xfce4-terminal

    //TODO/TEST
    //  if (bterm == "konsole" ||
    //          getenv("KONSOLE_DCOP") != "") ||
    //          getenv("KONSOLE_DBUS_SESSION") != "") {
    //      message("xterm_konsole");
    //      load_macro("tty/xterm_konsole");
    //      return;
    //  }

    //  Device Attribute Tests
    //
    //      DECDA2R (CSI > 65 ; FIRMWARE ; KEYBOARD [; OPTION]* c)
    //
    //      Firmware:
    //
    //          1  = VT220
    //          2  = VT240
    //          3  = DECmate II
    //          18 = VT330
    //          19 = VT340
    //          24 = VT320
    //          28 = DECterm
    //          32 = VT382J
    //          41 = VTV420
    //          42 = VT1000
    //          44 = VT382T
    //          61 = VT510
    //          64 = VT520
    //          65 = VT525
    //          66 = VTStar
    //
    //      A number of xterm compatible terminals normally run under the generic TERM
    //      identifier "xterm" yet can be derived using the "VT/XTERM Device Attribute",
    //      these include.
    //
    //          Terminal                  Type        Version         Example
    //          -------------------------------------------------------------------------
    //          Gnome-terminal (legacy)   1           >= 1115         1;3801;0
    //          Gnome-terminal            65          >= 6001         65;6001;1
    //          PuTTY                     0           136             0;136;0
    //          kconsole                                              0;115;0
    //          Terminal.app              1           95              1;95;0
    //          iTerm2                    0           95              0;95;0
    //          minTTY                    77(M)                       77;20005;0c ("20000" == 2.0.0)
    //          rxvt                      82(R)                       82;20703;0c ("20703" == 2.7.3)
    //          screen                    83(S)                       83;40500;0 (added "30600" == 3.6.0)
    //          urxvt                     85(U)
    //          libvterm                                              0;100;0
    //          msterminal                0           10              0;10;1c
    //          xterm                     -2(a)
    //
    //      Notes:
    //      a) Sourced from XTERM_VERSION not DA values.
    //
    //      https://invisible-island.net/xterm/ctlseqs/ctlseqs.pdf
    //
    bterm = "xterm";                            /* default */

    get_term_feature(TF_VT_DATYPE, datype);
    get_term_feature(TF_VT_DAVERSION, daversion);
#if defined(TF_VT_HARDWARE)
    get_term_feature(TF_VT_HARDWARE, dahardware);
#endif

    if (datype >= 0) {
        switch (datype) {
        case 0:
            if (daversion == 115) {             /* kconsole */
                bterm = "xterm-kconsole";

            } else if (daversion == 136) {      /* putty */
                xterm_load("xterm-putty");
                return;

            } else {
                if (daversion == 10) {
                    /* https://github.com/microsoft/terminal/pull/6850/files */
                    if (getenv("WT_SESSION")) { /* msterminal "0;10;1c" */
                        xterm_load("xterm-msterminal");
                        return;
                    }
                }
            }
            break;

        case 1:
            if (daversion >= 1115) {            /* gnome, hopefully */
                xterm_load("xterm-gnome");
                return;
            }
            break;

        case 61: /* see: https://github.com/GNOME/vte/blob/master/src/vteseq.cc */
        case 65:
            if (daversion >= 6001) {
                xterm_load("xterm-gnome");
                return;
            }
            break;

        case 77:                                /* minTTY (ASCII=M) */
           xterm_load("xterm-mintty");
           return;

        case 82:                                /* rxvt (ASCII=R) */
           xterm_load("xterm-rxvt");
           return;

        case 83:                                /* TODO/screen (ASCII=S) */
           bterm = "xterm-screen";
           break;

        case 85:                                /* TODO/urxvt (ASCII=U) */
           bterm = "xterm-urxvt";
           break;
        }
    }

    if (bterm == "xterm") {
        if (ostype == "linux" || ostype == "linux-gnu" || sysname == "Linux") {
            //
            // xterm-linux or linux host.
            //
            xterm_locale();
            xterm_load("xterm-linux");
            return;
        }
    }

    set_term_feature(TF_NAME, bterm);
    xterm_graphic();

    if (-2 == datype) {
        if (daversion > 0 && daversion < 166) { /* legacy unicode */
            set_term_feature(TF_UNICODE_VERSION, "3.0.0");
        }
        xterm_locale();
    }

    xterm_standard();
    xterm_altmeta_keys();

    /*
     *  Define keyboard layout for non-ascii characters.
     *
     *  https://invisible-island.net/xterm/xterm-function-keys.html
     *
     *      NAME        vt100       vt220       scoansi     xterm-r5    xterm-r6    xterm-vt220 xterm-xf86  xterm-new   rxvt     mgt      screen
     *      kcub1       \EOD        \E[D        \E[D        \EOD        \EOD        \EOD        \EOD        \EOD        \E[D     \EOD     \EOD
     *      kcud1       \EOB        \E[B        \E[B        \EOB        \EOB        \EOB        \EOB        \EOB        \E[B     \EOB     \EOB
     *      kcuf1       \EOC        \E[C        \E[C        \EOC        \EOC        \EOC        \EOC        \EOC        \E[C     \EOC     \EOC
     *      kcuu1       \EOA        \E[A        \E[A        \EOA        \EOA        \EOA        \EOA        \EOA        \E[A     \EOA     \EOA
     *      kf0         \EOy        \EOq                    \E[21~
     *      kf1         \EOP        \EOP        \E[M        \E[11~      \E[11~      \EOP        \EOP        \EOP        \E[11~   \EOP     \EOP
     *      kf2         \EOQ        \EOQ        \E[N        \E[12~      \E[12~      \EOQ        \EOQ        \EOQ        \E[12~   \EOQ     \EOQ
     *      kf3         \EOR        \EOR        \E[O        \E[13~      \E[13~      \EOR        \EOR        \EOR        \E[13~   \EOR     \EOR
     *      kf4         \EOS        \EOS        \E[P        \E[14~      \E[14~      \EOS        \EOS        \EOS        \E[14~   \EOS     \EOS
     *      kf5         \EOt        \E[Q        \E[15~      \E[15~      \E[15~      \E[15~      \E[15~      \E[15~      \E[15~   \E[15~
     *      kf6         \EOu        \E[17~      \E[R        \E[17~      \E[17~      \E[17~      \E[17~      \E[17~      \E[17~   \E[17~   \E[17~
     *      kf7         \EOv        \E[18~      \E[S        \E[18~      \E[18~      \E[18~      \E[18~      \E[18~      \E[18~   \E[18~   \E[18~
     *      kf8         \EOl        \E[19~      \E[T        \E[19~      \E[19~      \E[19~      \E[19~      \E[19~      \E[19~   \E[19~   \E[19~
     *      kf9         \EOw        \E[20~      \E[U        \E[20~      \E[20~      \E[20~      \E[20~      \E[20~      \E[20~   \E[20~   \E[20~
     *      kf10        \EOx        \E[21~      \E[V        \E[21~      \E[21~      \E[21~      \E[21~      \E[21~      \E[21~   \E[21~   \E[21~
     *      kf11        \E[23~      \E[W        \E[23~      \E[23~      \E[23~      \E[23~      \E[23~      \E[23~      \E[23~   \E[23~
     *      kf12        \E[24~      \E[X        \E[24~      \E[24~      \E[24~      \E[24~      \E[24~      \E[24~      \E[24~   \E[24
     *
     *      kDC                                                                                 \E[3;2~     \E[3;2~     \E[3$
     *      kEND                                                                                \E[1;2F     \E[1;2F     \E[8$
     *      kHOM                                                                                \E[1;2H     \E[1;2H     \E[7$
     *      kIC                                                                                 \E[2;2~     \E[2;2~     \E[2$
     *      kLFT                                                                                \E[1;2D     \E[1;2D     \E[d
     *      kNXT                                                                                \E[6;2~     \E[6;2~     \E[6$
     *      kPRV                                                                                \E[5;2~     \E[5;2~     \E[5$
     *      kRIT                                                                                \E[1;2C     \E[1;2C     \E[c
     *
     *      kend                                \E[F        \E[4~                   \E[4~       \EOF        \EOF        \E[8~    \EOF     \E[4~
     *      khome                               \E[H        \E[1~                   \E[1~       \EOH        \EOH        \E[7~    \EOH     \E[1~
     *
     *      kDN                                                                                             \E[1;2B     \E[b
     *      kUP                                                                                             \E[1;2A     \E[a
     */
    set_term_keyboard(
        F1_F12, quote_list(                     /* xterm */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

        F1_F12, quote_list(                     /* rxvt */
            "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",     "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

        SHIFT_F1_F12, quote_list(               /* rxvt */
            "\x1b[25~",     "\x1b[26~",     "\x1b[28~",     "\x1b[29~",     "\x1b[31~",
            "\x1b[32~",     "\x1b[33~",     "\x1b[34~",     NULL,           NULL,
            NULL,           NULL),

        F1_F12, quote_list(                     /* Sun Function Keys */
            "\x1b[192z",    "\x1b[193z",    "\x1b[194z",    "\x1b[195z",    "\x1b[196z",
            "\x1b[197z",    "\x1b[198z",    "\x1b[199z",    "\x1b[200z",    "\x1b[201z",
            "\x1b[234z",    "\x1b[235z"),

        SHIFT_F1_F12, quote_list(               /* VT100.Trans */
            "\x1b[SF1~",    "\x1b[SF2~",    "\x1b[SF3~",    "\x1b[SF4~",    "\x1b[SF5~",
            "\x1b[SF6~",    "\x1b[SF7~",    "\x1b[SF8~",    "\x1b[SF9~",    "\x1b[SF10~",
            "\x1b[SF11~",   "\x1b[SF12~"),

        CTRL_F1_F12, quote_list(                /* VT100.Trans */
            "\x1b[CF1~",    "\x1b[CF2~",    "\x1b[CF3~",    "\x1b[CF4~",    "\x1b[CF5~",
            "\x1b[CF6~",    "\x1b[CF7~",    "\x1b[CF8~",    "\x1b[CF9~",    "\x1b[CF10~",
            "\x1b[CF11~",   "\x1b[CF12~"),

        ALT_F1_F12, quote_list(                 /* VT100.Trans */
            "\x1b[AF1~",    "\x1b[AF2~",    "\x1b[AF3~",    "\x1b[AF4~",    "\x1b[AF5~",
            "\x1b[AF6~",    "\x1b[AF7~",    "\x1b[AF8~",    "\x1b[AF9~",    "\x1b[AF10~",
            "\x1b[AF11~",   "\x1b[AF12~"),

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

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del/.           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* Standard (Application mode) */
            "\x1b[2~",      "\x1bOF",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~",
            "\x1b[3~",      "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM",       NULL,           NULL,           NULL,
            NULL),

        ALT_0_9, quote_list(                    /* X.Org (7bit) */
            "\xC2\xB0",     "\xC2\xB1",     "\xC2\xB2",     "\xC2\xB3",     "\xC2\xB4",
            "\xC2\xB5",     "\xC2\xB6",     "\xC2\xB7",     "\xC2\xB8",     "\xC2\xB9" ),

//      ALT_0_9, quote_list(                    /* Meta-Numeric */
//          "\x1ba0",       "\x1ba1",       "\x1ba2",       "\x1ba3",       "\x1ba4",
//          "\x1ba5",       "\x1ba6",       "\x1ba7",       "\x1ba8",       "\x1ba9"),

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
        KEY_BACKSPACE,      "\x7f",

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


static int
xterm_load(string name)
{
    string macroname;

    macroname = "tty/" + name;
    macroname = re_translate(SF_GLOBAL, "[ -]+", "_", macroname);

    message(name);
    set_term_feature(TF_NAME, name);

    return load_macro(macroname);
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

/*end*/
