/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_rxvt.cr,v 1.9 2014/10/22 02:34:43 ayoung Exp $
 *
    rxvt is a very popular xterm replacement.

    It uses the same escape sequences at the X11R6 xterm for F1 through F12.

    Shift-F1 through Shift-F12 work similarly to multi-gnome-terminal; they add ten to
    the number on the key (so there are again two ways to get F11 and F12).

    rxvt generates the same escape sequences as multi-gnome-terminal for F11 through
    F20, and uses ^[[23$ and ^[[24$ for F21 and F22, respectively.

    The sequence continues with Ctrl-F1 through Ctrl-F12 generating ^[[11^ through
    ^[[24^ for F23 through F34 (no overlap with previous sequences), Ctrl-Shift-F1
    through Ctrl-Shift-F10 generating ^[[23^ through ^[[34^ for F33 through F42
    (two-key overlap), and Ctrl-Shift-F11 and Ctrl-Shift-F12 generating ^[[23@ and
    ^[[24@ for F43 and F44.

    The base $TERM type for rxvt is rxvt, though it ships with several types for
    different circumstances, including rxvt-basic and rxvt-m. It also comes with
    rxvt-unicode, but on my system that definition only lists function keys up to F20.

    Alt-Meta keys:

        Rxvt*meta8:         false

            True: handle Meta (Alt) + keypress to set the 8th bit. False
            [default]: handle Meta (Alt) + keypress as an escape prefix

        in your ~/.Xresources and then run xrdb -merge ~/.Xresources to have the
        setting kick in, then it should work for you.

        Note some of the Alt- keys can be used by rxvt.

    Back-Tab:

        rxvt by default shall map Shift-Tab to \x1b[Z, but some Xwin environments
        report Shift-Tab as XP_KP_Tab (keypad-tab). Hence the following patch maybe
        required (rxvt 2.7.10):

            command.c:
        #if defined(XK_KP_Tab)
                    case XK_KP_Tab:
        #endif
                    case XK_Tab:

        Note: The kysym override does not work as shift-tab reports shift as active and
        keysym processing only occurs if shift or ctrl aren't asserted.

    Color settings:

        Here are values that are supposed to resemble a VGA screen, including the murky
        brown that passes for low-intensity yellow:

            Rxvt*color0:        #000000
            Rxvt*color1:        #A80000
            Rxvt*color2:        #00A800
            Rxvt*color3:        #A8A800
            Rxvt*color4:        #0000A8
            Rxvt*color5:        #A800A8
            Rxvt*color6:        #00A8A8
            Rxvt*color7:        #A8A8A8

            Rxvt*color8:        #000054
            Rxvt*color9:        #FF0054
            Rxvt*color10:       #00FF54
            Rxvt*color11:       #FFFF54
            Rxvt*color12:       #0000FF
            Rxvt*color13:       #FF00FF
            Rxvt*color14:       #00FFFF
            Rxvt*color15:       #FFFFFF

        Also the macro xterm_pccolors() using X11 colors gives a close match.

    Fonts:

        Under cygwin/rxvt would advice the use of 'Lucida ConsoleP' font. This
        mono-space TrueType font is encoded for codepage 437; rather then the ANSI or
        OSI character-set; which includes the needed line drawing characters.

        One source as at 15/01/2010, being otherwise google.

            http://cygutils.fruitbat.org/bashprompt/index.html

        Once downloaded must be installed

            Control Panel -> Fonts -> File Menu -> Install New Font

    Summary:

        One of more complete key-mappings out of the box.

        rxvt 2.7.10 seems to be the best tested version to date.

        rxvt 2.6.x pgdn/pgup key mappings wont work as expected, as they are consumed
        internally for scroll up and down.

 *
 *
 *
 */

/**********************************************************************

Example 'rxvt' resources:

!Basic
rxvt.scrollBar:             True
rxvt.reverseWrap:           True
rxvt.fullCursor:            True
rxvt.scrollTtyOutput:       Off
rxvt.scrollKey:             On
rxvt.titleBar:              False
rxvt.saveLines:             30000
rxvt.scrollBar_right:       True

!Colors (dark)
rxvt.reverseVideo:          False
rxvt.background:            Black
rxvt.forerground:           White
rxvt.cursorColor:           Green

!cywgin specific configuration, one of the following
rxvt.font:                  Lucida ConsoleP-14
rxvt.font:                  Lucida Console-14
rxvt.font:                  -*-lucidatypewriter-medium*-*-*-14-*-*-*-*-*-*-*
rxvt.font:                  -*-lucidatypewriter-bold-*-*-*-14-*-*-*-*-*-*-*

! Tab and Shift-Tab support
rxtv*VT100.Translations:    #override \n\
    Shift<Key>KP_Tab:           string(\033[Z)\n\
    ~Shift<Key>Prior:           string(\033[5~)\n\
    ~Shift<Key>Next:            string(\033[6~)\n\
    Shift<Key>Prior:            scroll-back(1,page)\n\
    Shift<Key>Next:             scroll-forw(1,page)\n

Note: If you're already in X, you'll need to reload your .Xdefaults before they'll take
effect, for example

    xrdb < ~/.Xdefaults

.....................................................................*/

#include "tty.h"
#include "tty_xterm.h"

void
main()
{
    int attributes;

    /*
     *  Load support functions
     */
    set_term_feature(TF_NAME, "xterm-rxvt");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Set characters used for extended graphics support when drawing windows.
     */
    xterm_graphic();

    /*
     *  Define escape sequences used for special optimisations on output.
     */
    xterm_color();                              /* colour by default */
    rxvt_standard();

    /*
     *  Lacking termcap
     */
    get_term_feature(TF_ATTRIBUTES, attributes);
    if ((attributes & TF_AFUNCTIONKEYS) == 0) {
        set_term_keyboard(
             F1_F12, quote_list(                /* standard F1-F12 */
                "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",
                "\x1b[15~",     "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
                "\x1b[20~",     "\x1b[21~",     "\x1b[23~",     "\x1b[24~"));
    }

    /*
     *  Define keyboard layout for non-ascii characters.
     */
    set_term_keyboard(                          /* Function keys */
        SHIFT_F1_F12, quote_list(
            "\x1b[23~",     "\x1b[24~",     "\x1b[25~",     "\x1b[26~",     "\x1b[28~",
            "\x1b[29~",     "\x1b[31~",     "\x1b[32~",     "\x1b[33~",     "\x1b[34~",
            "\x1b[23$",     "\x1b[24$"),

        CTRL_F1_F12, quote_list(
            "\x1b[11^",     "\x1b[12^",     "\x1b[13^",     "\x1b[14^",     "\x1b[15^",
            "\x1b[17^",     "\x1b[18^",     "\x1b[19^",     "\x1b[20^",     "\x1b[21^",
            "\x1b[23^",     "\x1b[24^"),

        ALT_A_Z, quote_list(                    /* Alt-[A-Z] */
            "\x1b\xe1",     NULL,           NULL,           NULL,           "\x1b\xe9",
            NULL,           NULL,           NULL,           "\x1b\xed",     NULL,
            NULL,           NULL,           NULL,           NULL,           "\x1b\xf3",
            NULL,           NULL,           NULL,           NULL,           NULL,
            "\x1b\xfa",     NULL,           NULL,           NULL,           NULL,
            NULL),

        ALT_A_Z, quote_list(                    /* Shifted Alt-[A-Z] */
            "\x1b\xc1",     NULL,           NULL,           NULL,           "\x1b\xc9",
            NULL,           NULL,           NULL,           "\x1b\xcd",     NULL,
            NULL,           NULL,           NULL,           NULL,           "\x1b\xd3",
            NULL,           NULL,           NULL,           NULL,           NULL,
            "\x1b\xda",     NULL,           NULL,           NULL,           NULL,
            NULL),

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del/.           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* Standard (Application mode) */
            "\x1b[2~",      "\x1b[8~",      "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1b[7~",      "\x1bOA",       "\x1b[5~",
            "\x1b[3~",      "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1b[2^",      "\x1b[8^",      "\x1bOb",       "\x1b[6^",      "\x1bOd",
            NULL,           "\x1bOc",       "\x1b[7^",      "\x1bOa",       "\x1b[5^",
            "\x1b[3^"),

        SHIFT_KEYPAD_2,     "\x1b[b",           /* Shift-Down */
        SHIFT_KEYPAD_4,     "\x1b[d",           /* Shift-Left */
        SHIFT_KEYPAD_6,     "\x1b[c",           /* Shift-Right */
        SHIFT_KEYPAD_8,     "\x1b[a",           /* Shift-Up */

        ALT_KEYPAD_2,       "\x1b\x1bOB",       /* Alt-Down */
        ALT_KEYPAD_4,       "\x1b\x1bOD",       /* Alt-Left */
        ALT_KEYPAD_6,       "\x1b\x1bOC",       /* Alt-Right */
        ALT_KEYPAD_8,       "\x1b\x1bOA",       /* Alt-Up */

        ALT_KEYPAD_2,       "\x1b\x1b[B",       /* Alt-Down */
        ALT_KEYPAD_4,       "\x1b\x1b[D",       /* Alt-Left */
        ALT_KEYPAD_6,       "\x1b\x1b[C",       /* Alt-Right */
        ALT_KEYPAD_8,       "\x1b\x1b[A",       /* Alt-Up */

        BACK_TAB,           "\x1b[Z",           /* Shift-Tab */
        BACK_TAB,           "\x1b\t",           /* Alt-Tab */
        BACKSPACE,          "\x7f"
        );

    xterm_altmeta_keys();
}


void
rxvt_standard(void)
{
    set_term_feature(TF_PRINT_SPACE, "\x1b[%dX");
    set_term_feature(TF_CURSOR_RIGHT, "\x1b[%dC");
    set_term_feature(TF_GRAPHIC_MODE, "\x1B(0");
    set_term_feature(TF_TEXT_MODE, "\x1B(B");
//  set_term_feature(TF_EIGHT_BIT, TRUE);
}


void
rxvt_ctrlshift(void)
{
    set_term_keyboard(                          /* Function keys */
        CTRLSHIFT_F1_F12, quote_list(
            NULL,           NULL,           "\x1b[25^",     "\x1b[26^",     "\x1b[28^",
            "\x1b[29^",     "\x1b[31^",     "\x1b[32^",     "\x1b[33^",     "\x1b[34^",
            "\x1b[23@",     "\x1b[24@")
        );
}


void
rxvt()
{
    /*NOTHING*/
}

