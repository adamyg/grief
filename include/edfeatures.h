#ifndef GR_EDFEATURES_H_INCLUDED
#define GR_EDFEATURES_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edfeatures_h,"$Id: edfeatures.h,v 1.17 2023/01/01 11:26:58 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edfeatures.h,v 1.17 2023/01/01 11:26:58 cvsuser Exp $
 * Editor features.
 *
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edtypes.h>

__CBEGIN_DECLS

/*--export--defines--*/
/*
 *  Terminal/display features.
 */
#define TF_INIT                 1               /* additional init commands */
#define TF_RESET                2               /* additional reset commands */
#define TF_GRAPHIC_MODE         3               /* enable graphics mode */
#define TF_TEXT_MODE            4               /* leave graphics mode */
#define TF_INSERT_CURSOR        5               /* insert cursor */
#define TF_OVERWRITE_CURSOR     6               /* overwrite cursor */
#define TF_VINSERT_CURSOR       7               /* virtual insert cursor */
#define TF_VOVERWRITE_CURSOR    8               /* virtual overwrite cursor */
#define TF_PRINT_SPACE          9               /* print space */
#define TF_PRINT_BITEIGHT       10              /* print 8bit character */
#define TF_PRINT_ESCAPE         11              /* print ESC */
#define TF_REPEAT_LAST          12              /* repeat last character #1 times */
#define TF_CURSOR_RIGHT         13              /* move cursor within current line */
#define TF_MOUSE                14              /* mouse type */
#define TF_MOUSECLKMS           15              /* mouse click timeout */
#define TF_WINDOW_SETSIZE       16              /* window size (row/cols) */
#define TF_WINDOW_SETPOS        17              /* window position (row/col) */

#define TF_COLOR                30              /* supports color (> 1, then defines the default depth) */
#define TF_COLORDEPTH           31              /* color depth (8, 16, 88 or 256) */
#define TF_DEFAULT_FG           32              /* default foreground color */
#define TF_DEFAULT_BG           33              /* default background color */
#define TF_SCHEMEDARK           34              /* *true* if the default color is "dark" */
#define TF_COLORSETFGBG         35
#define TF_COLORSET_FG          36              /* color set foreground control sequence */
#define TF_COLORSET_BG          37              /* color set background control sequence */
#define TF_COLORMAP             38              /* color map (terminal) */
#define TF_COLORPALETTE         39              /* color palette (driver) */
#define TF_COLORSCHEME          40              /* current scheme dark or light */

#define TF_CLEAR_IS_BLACK       50              /* clear is black */
#define TF_DISABLE_INSDEL       51              /* disable ins/del scrolling method */
#define TF_DISABLE_SCROLL       52              /* disable scroll regions scrolling method */
#define TF_SCROLL_MAX           53              /* scroll region limit, optimisations */
#define TF_0M_RESETS_COLOR      53              /* 0m reset color (defunc) */
#define TF_EIGHT_BIT            55              /* supports 8bit characters */
#define TF_CODEPAGE             56              /* codepage */
#define TF_NOALTSCREEN          57              /* not used */
#define TF_LAZYUPDATE           58              /* lazy syntax hilite updates */
#define TF_NAME                 59              /* terminal name */
#define TF_ATTRIBUTES           60              /* terminal attribute flags */
#define TF_TTY_FAST             62              /* fast tty optimisations */
#define TF_TTY_GRAPHICSBOX      63              /* graphics mode required for box characters */

#define TF_SCREEN_ROWS          70              /* screen rows */
#define TF_SCREEN_COLS          71              /* screen cols */
#define TF_LINENO_COLUMNS       72
#define TF_WINDOW_MINROWS       73
#define TF_WINDOW_MINCOLS       74

#define TF_XTERM_CURSOR         80              /* XTERM cursor color support */
#define TF_XTERM_TITLE          81              /* XTERM title support */
#define TF_XTERM_COMPAT         82              /* XTERM compatible termuinal */
#define TF_XTERM_PALETTE        83              /* XTERM palette control */

#define TF_VT_DATYPE            90              /* VT/XTERM Devive Attribute Type */
#define TF_VT_DAVERSION         91              /* VT/XTERM Devive Attribute Version */

#define TF_ENCODING             100             /* terminal character encoding */
#define TF_ENCODING_GUESS       101             /* text encoding guess specification */
#define TF_UNICODE_VERSION      102             /* UNICODE version; eg. "6.0.0" */
/*--end--*/

/*--export--defines--*/
/*
 *  Terminal characters flags.
 */
#define TC_TOP_LEFT             1
#define TC_TOP_RIGHT            2
#define TC_BOT_LEFT             3
#define TC_BOT_RIGHT            4
#define TC_VERTICAL             5
#define TC_HORIZONTAL           6
#define TC_TOP_JOIN             7
#define TC_BOT_JOIN             8
#define TC_CROSS                9
#define TC_LEFT_JOIN            10
#define TC_RIGHT_JOIN           11
#define TC_SCROLL               12
#define TC_THUMB                13
/*--end--*/

/*--export--defines--*/
/*
 *  Terminal attributes flags.
 */
#define TF_AGRAPHICCHARACTERS   0x0000001       /* Graphic characteres (ACS defined) */
#define TF_AFUNCTIONKEYS        0x0000002       /* F1-F10 function keys */
#define TF_ACYGWIN              0x0000004       /* Cygwin native console */
#define TF_AUTF8ENCODING        0x0000010       /* UTF8 character encoding, Unicode implied */
#define TF_AUNICODEENCODING     0x0000020       /* Unicode character encoding */
#define TF_AMETAKEY             0x0000100       /* Meta keys */
/*--end--*/


/*
 *  Terminal configuration/control.
 */
#define PT_ESCMAX               20
#define PT_NAME                 64

struct _features {
    uint32_t    pt_magic;                       /* Structure magic. */
#define PT_MAGIC                MKMAGIC('P','t','p','T')

    char        pt_name[PT_NAME];               /* STRING,      Name. */

    /*
     *  special characters.
     */
    char        pt_top_left[PT_ESCMAX];         /* STRING,      Top left corner. */
    char        pt_top_right[PT_ESCMAX];        /* STRING,      Top right corner. */
    char        pt_bot_left[PT_ESCMAX];         /* STRING,      Bottom left corner. */
    char        pt_bot_right[PT_ESCMAX];        /* STRING,      Bottom right corner. */
    char        pt_vertical[PT_ESCMAX];         /* STRING,      Vertical Line. */
    char        pt_horizontal[PT_ESCMAX];       /* STRING,      Horizontal line. */
    char        pt_top_join[PT_ESCMAX];         /* STRING,      Horizontal line, with vertical join going down. */
    char        pt_bot_join[PT_ESCMAX];         /* STRING,      Horizontal line, with vertical join going up. */
    char        pt_cross[PT_ESCMAX];            /* STRING,      Four way intersection. */
    char        pt_left_join[PT_ESCMAX];        /* STRING,      Vertical line with join going left. */
    char        pt_right_join[PT_ESCMAX];       /* STRING,      Vertical line with join going right. */
    char        pt_scroll[PT_ESCMAX];           /* STRING,      Scroll bar background */
    char        pt_thumb[PT_ESCMAX];            /* STRING,      Scroll bar thumb. */

    /*
     *  specialise control sequences.
     */
    char        pt_icursor[PT_ESCMAX];          /* STRING,      Escape sequence, insert mode cursor. */
    char        pt_ocursor[PT_ESCMAX];          /* STRING,      Escape sequence, overstike mode cursor. */
    char        pt_vicursor[PT_ESCMAX];         /* STRING,      Escape sequence, insert mode cursor (virtual). */
    char        pt_vocursor[PT_ESCMAX];         /* STRING,      Escape sequence, overstike mode cursor (virtual). */

    char        pt_repeat_space[PT_ESCMAX];     /* STRING,      Escape sequence, erase multiple spaces. */
    char        pt_character[PT_ESCMAX];        /* STRING,      Escape sequence, Print special characters. */
    char        pt_escape[PT_ESCMAX];           /* STRING,      Escape sequence, print ESC. */
    char        pt_repeat_last[PT_ESCMAX];      /* STRING,      Escape sequence, repeating last character. */
    char        pt_cursor_right[PT_ESCMAX];     /* STRING,      Escape sequence, to move cursor on same line. */

    char        pt_winsetsize[PT_ESCMAX];       /* STRING,      Escape sequence, set window size (rows/cols). */
    char        pt_winsetpos[PT_ESCMAX];        /* STRING,      Escape sequence, set window position (row/col). */

    char        pt_colorsetfgbg[PT_ESCMAX];     /* STRING,      Escape eequence, set foreground and background. */
    char        pt_colorsetfg[PT_ESCMAX];       /* STRING,      Escape sequence, set foreground color. */
    char        pt_colorsetbg[PT_ESCMAX];       /* STRING,      Escape sequence, set background color. */

    char        pt_graphics_mode[PT_ESCMAX];    /* STRING,      Escape sequence, go into graphics mode. */
    char        pt_text_mode[PT_ESCMAX];        /* STRING,      Escape sequence, go into text mode. */

    char        pt_init[PT_ESCMAX];             /* STRING,      Escape sequence, init on startup. */
    char        pt_reset[PT_ESCMAX];            /* STRING,      Escape sequence, to reset on exit. */

    /*
     *  options/settings
     */
    char        pt_mouse[PT_ESCMAX];            /* STRING,      Mouse type (xterm, gmp or tty). */
    int         pt_mouseclkms;                  /* INT,         Mouse click timeout is milliseconds. */

    int         pt_0m;                          /* BOOL,        "\033[0m" resets color as well as character attributes. */
    int         pt_color;                       /* BOOL,INT     TRUE if terminal supports color (> 1 states default depth). */
    int         pt_colordepth;                  /* INT,         Color depth (active). */
    int         pt_clrisblack;                  /* BOOL,        Erasing line clears to a black space. */
    int         pt_scroll_disable;              /* BOOL,        Disable scroll regions. */
    int         pt_scroll_max;                  /* INT,         Max scroll region size. */
    int         pt_noinsdel;                    /* BOOL,        Do not use ins/del scrolling. */
    int         pt_8bit;                        /* BOOL,        Display eight bit characters. */
    int         pt_codepage;                    /* INT,         Codepage. */
    int         pt_lazyupdate;                  /* INT,         Lazy syntax hilite updates. */

    int         pt_schemedark;                  /* INT,         *true* if color scheme is "dark" */
    int         pt_defaultfg;                   /* INT,         Default foreground color. */
    int         pt_defaultbg;                   /* INT,         Default background color. */
    int         pt_attributes;                  /* INT,         Termcap attributes. */

    int         pt_noaltscreen;                 /* BOOL,        XTERM alt-screen not supported. */
    int         pt_xtcompat;                    /* BOOL,        XTERM compatible terminal. */
    int         pt_xtcursor;                    /* BOOL,        XTERM cursor color (command). */
    int         pt_xttitle;                     /* BOOL,        XTERM title (command). */
    int         pt_xtpalette;                   /* INT,         XTERM patette control. */

    int         pt_vtdatype;                    /* INT,         Device attribute type. */
    int         pt_vtdaversion;                 /* INT,         Device attribute version. */

    int         pt_tty_fast;                    /* INT,         TTY fast screen optimisations. */
    int         pt_tty_graphicsbox;             /* INT,         TTY box characters require graphics mode. */

    int         pt_lineno_columns;              /* INT,         Line-number display columns */
    int         pt_window_minrows;              /* INT,         Window rows limit. */
    int         pt_window_mincols;              /* INT,         Window column limit. */

    int         pt_screen_rows;                 /* INT,         Screen rows. */
    int         pt_screen_cols;                 /* INT,         Screen columns. */

    char        pt_unicode_version[PT_NAME];    /* STRING,      UNICODE version. */
    char        pt_encoding[PT_NAME];           /* STRING,      Encoding. */

    char        pt_colormap[512];               /* STRING,      XTERM color map. */
    char        pt_colorpalette[512];           /* STRING,      Driver color palette <index>=<color>. */
    char        pt_colorscheme[32];             /* STRING,      Current color scheme (dark or light). */

    uint32_t    pt_magic2;                      /* Structure magic. */
};

typedef struct _features features_t;

__CEND_DECLS

#endif /*GR_EDFEATURES_H_INCLUDED*/
/*end*/
