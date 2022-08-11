#include <edidentifier.h>
__CIDENT_RCSID(gr_config_c,"$Id: config.c,v 1.35 2022/08/10 15:44:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.c,v 1.35 2022/08/10 15:44:56 cvsuser Exp $
 * Machine dependent configuration variables.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include "config.h"

#include <editor.h>
#include <edpaths.h>
#include <edalt.h>

/*
 *  Default environment
 */
#include <limits.h>                             /* confirm compiler environment */
#if !defined(__MAKEDEPEND__)
#if (CHAR_MIN == 0)
#error Character value is unsigned
#endif
#endif

const char *x_grfile        = "GRFILE=newfile";
const char *x_grflags       = "GRFLAGS=-i60";

#if defined(_VMS)
const char *x_machtype      = "VMS";
#elif defined(__OS2__)
const char *x_machtype      = "OS/2";
#elif defined(_WIN32) || defined(WIN32)
#if defined(__MINGW32__)
const char *x_machtype      = "Mingw32";
#else
const char *x_machtype      = "Win32";          /* 98,NT,XP etc */
#endif
#elif defined(DOSISH)
const char *x_machtype      = "DOS";
#elif defined(__APPLE__)
const char *x_machtype      = "MACOSX";
#else
const char *x_machtype      = "UNIX";           /* others including Cygwin */
#endif

#if defined(_VMS)
const char *x_grtermcap     = "grief_library:[etc]termcap.dat";
const char *x_grhelp        = "grief_library:[help]";
const char *x_grpath        = "grief_library:[macros]";
#else
const char *x_grtermcap     = _PATH_TERMCAP;
const char *x_grhelp        = _PATH_GRIEF_HELP;
#if defined(DOSISH)
const char *x_grpath        = _PATH_GRIEF_MACROS ";" _PATH_GRIEF_SOURCE;
#else
const char *x_grpath        = _PATH_GRIEF_MACROS ":" _PATH_GRIEF_SOURCE;
#endif
#endif

#if defined(__MSDOS__)
#if defined(__MINGW32__)
const char *x_default_term  = "TERM=mingw32";
#elif defined(_WIN32) || defined(WIN32)
const char *x_default_term  = "TERM=win32";
#else
const char *x_default_term  = "TERM=dos";
#endif
#elif defined(__OS2__)
const char *x_default_term  = "TERM=os2";
#elif defined(__CYGWIN__)
const char *x_default_term  = "TERM=cygwin";
#elif defined(linux)
const char *x_default_term  = "TERM=linux";
#elif defined(_AIX)
const char *x_default_term  = "TERM=aixterm";
#else
const char *x_default_term  = "TERM=ansi";
#endif


/*
 *  Default colours
 */
const char *                x_col_table_dark[] = {
    "background="               "NONE",
    "normal="                   "NONE",
    "select="                   "CYAN",
    "message="                  "GREEN",
    "error="                    "RED",
    "hilite="                   "WHITE,DARK-BLUE",
    "standout="                 "BLUE",
    "whitespace="               "WHITE,RED",

    "dialog_focus="             "MAGENTA",
    "dialog_hilite="            "LIGHT-WHITE,DARK-BLUE",
    "dialog_but_normal="        "GREY",
    "dialog_but_focus="         "LIGHT-WHITE",
    "dialog_but_key_normal="    "DARK-RED:underline",
    "dialog_but_key_focus="     "RED:underline",
    "dialog_edit_greyed="       "GREY,BLACK",
    "dialog_edit_normal="       "BLACK,GREY",
    "dialog_edit_focus="        "BLACK,WHITE",

    "string="                   "GREEN",
    "operator="                 "BLUE",
    "number="                   "GREEN",
    "comment="                  "CYAN",
    "delimiter="                "DARK-GREY",
    "preprocessor="             "MAGENTA",
    "preprocessor_keyword="     "DARK-MAGENTA",
    "keyword="                  "YELLOW",

    NULL
    };


const char *                x_col_table_light[] = {
    "background="               "NONE",
    "normal="                   "NONE",
    "select="                   "DARK-BLUE",
    "message="                  "BLACK",
    "error="                    "DARK-RED",
    "hilite="                   "WHITE,DARK-BLUE",
    "standout="                 "DARK-BLUE",

    "dialog_focus="             "DARK-MAGENTA",
    "dialog_hilite="            "WHITE,DARK-BLUE",
    "dialog_but_normal="        "BLACK",
    "dialog_but_focus="         "WHITE,BLACK",
    "dialog_but_key_normal="    "RED:underline",
    "dialog_but_key_focus="     "RED,BLACK:underline",
    "dialog_edit_greyed="       "GREY,BLACK",
    "dialog_edit_normal="       "LIGHT-WHITE,BLACK",
    "dialog_edit_focus="        "CYAN,BLACK",

    "string="                   "DARK-RED",
    "operator="                 "DARK-BLUE",
    "number="                   "DARK-RED",
    "comment="                  "DARK-BLUE",
    "delimiter="                "DARK-BLUE",
    "preprocessor="             "DARK-MAGENTA",
    "preprocessor_keyword="     "DARK-RED",
    "keyword="                  "DARK-GREEN",

    NULL
    };


/*
 *  Default borderless window colours.
 */
const char *                x_col_windows[] = {
    "window1="                  "YELLOW,BLACK",
    "window2="                  "LIGHT-WHITE,BLUE",
    "window3="                  "BLACK,GREEN",
    "window4="                  "BLUE,CYAN",
    "window5="                  "YELLOW,RED",
    "window6="                  "BLUE,MAGENTA",
    "window7="                  "LIGHT-WHITE,BROWN",
    "window8="                  "BLUE,WHITE",

    "window9="                  "LIGHT-CYAN,BLACK",
    "window10="                 "LIGHT-WHITE,BLUE",
    "window11="                 "BLACK,GREEN",
    "window12="                 "BLUE,CYAN",
    "window13="                 "BLUE,RED",
    "window14="                 "LIGHT-WHITE,MAGENTA",
    "window15="                 "WHITE,BROWN",
    "window16="                 "BLACK,WHITE",

    NULL
    };


/*
 *  Default keyboard mapping
 */
const struct k_tbl          x_key_table[] = {
    /* Function keys */
    { F(1),                     "change_window" },
    { F(2),                     "move_edge" },
    { F(3),                     "create_edge" },
    { F(4),                     "delete_edge" },
    { F(5),                     "search_fwd" },
    { F(6),                     "translate" },
    { F(7),                     "remember" },
    { F(8),                     "playback" },
    { F(9),                     "load_macro" },
    { F(10),                    "execute_macro" },
    { SF(7),                    "pause" },

    /* Special functions keys */
    { KEY_HELP,                 "help" },
    { KEY_CUT_CMD,              "cut" },
    { KEY_COPY_CMD,             "copy" },
    { KEY_PASTE,                "paste" },
    { KEY_UNDO_CMD,             "undo" },
    { KEY_REDO,                 "redo" },
    { KEY_EXIT,                 "exit" },
    { KEY_COMMAND,              "execute_macro" },

    /* Keypad */
    { KEY_INS,                  "paste" },
    { KEY_END,                  "end_of_line" },
    { KEY_DOWN,                 "down" },
    { KEY_PAGEDOWN,             "page_down" },
    { WHEEL_DOWN,               "page_down" },
    { KEY_LEFT,                 "left" },
    { KEY_RIGHT,                "right" },
    { KEY_HOME,                 "beginning_of_line" },
    { KEY_UP,                   "up" },
    { KEY_PAGEUP,               "page_up" },
    { WHEEL_UP,                 "page_up" },
    { KEY_DEL,                  "delete_block" },
    { KEY_CUT,                  "cut" },
    { KEY_COPY,                 "copy" },
    { KEY_UNDO,                 "undo" },

    { SHIFT_KEYPAD_2,           "change_window 2" },
    { SHIFT_KEYPAD_4,           "change_window 3" },
    { SHIFT_KEYPAD_6,           "change_window 1" },
    { SHIFT_KEYPAD_8,           "change_window 0" },
    { CTRL_KEYPAD_1,            "end_of_window" },
    { CTRL_KEYPAD_3,            "end_of_buffer" },
    { CTRL_KEYPAD_7,            "top_of_window" },
    { CTRL_KEYPAD_9,            "top_of_buffer" },

    /* Others */
    { CTRL_X,                   "exit" },
    { ALT_0,                    "drop_bookmark 0" },
    { ALT_1,                    "drop_bookmark 1" },
    { ALT_2,                    "drop_bookmark 2" },
    { ALT_3,                    "drop_bookmark 3" },
    { ALT_4,                    "drop_bookmark 4" },
    { ALT_5,                    "drop_bookmark 5" },
    { ALT_6,                    "drop_bookmark 6" },
    { ALT_7,                    "drop_bookmark 7" },
    { ALT_8,                    "drop_bookmark 8" },
    { ALT_9,                    "drop_bookmark 9" },
    { ALT_A,                    "mark 4" },
    { ALT_B,                    "buffer_list" },
    { ALT_C,                    "mark 2" },
    { ALT_D,                    "delete_line" },
    { ALT_E,                    "edit_file" },
    { ALT_F,                    "feature" },
    { ALT_G,                    "goto_line" },
    { ALT_I,                    "insert_mode" },
    { ALT_J,                    "goto_bookmark" },
    { ALT_K,                    "delete_to_eol" },
    { ALT_L,                    "mark 3" },
    { ALT_M,                    "mark" },
    { ALT_O,                    "output_file" },
    { ALT_P,                    "print" },
    { ALT_Q,                    "quote" },
    { ALT_R,                    "read_file" },
    { ALT_S,                    "search_fwd" },
    { ALT_T,                    "translate" },
    { ALT_U,                    "undo" },
    { ALT_V,                    "version" },
    { ALT_W,                    "write_buffer" },
    { ALT_X,                    "exit" },
    { ALT_Z,                    "shell" },

    { 0,                        NULL }
    };

/*end*/
