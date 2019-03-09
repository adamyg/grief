#ifndef GR_EDSTRUCT_H_INCLUDED
#define GR_EDSTRUCT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edstruct_h,"$Id: edstruct.h,v 1.65 2018/10/04 01:28:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edstruct.h,v 1.65 2018/10/04 01:28:00 cvsuser Exp $
 * Window, buffer, line and character-map definitions.
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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
#include <edheaders.h>

#include <libllist.h>
#include <libsplay.h>                           /* splay binary trees. */
#include <tailqueue.h>

#include <edopcode.h>

__CBEGIN_DECLS

typedef struct _cmapchr cmapchr_t;
typedef struct _cmap    cmap_t;
typedef struct _window  WINDOW_t;
typedef struct _buffer  BUFFER_t;
typedef struct _line    LINE_t;
typedef struct _display DISPLAY_t;
typedef struct _undo    UNDO_t;

typedef uint32_t        vbyte_t;

#define GR_SWAP(__a,__b,__c) \
                        __c=__a, __a=__b, __b=__c

enum _marks {
/*--export--enum--*/
/*
 *  Hilight types (anchors)
 */
    MK_NONE             =0,
    MK_NORMAL           =1,
    MK_COLUMN           =2,
    MK_LINE             =3,
    MK_NONINC           =4
/*--end--*/
};


/*
 *  Key table element
 */
struct k_tbl {
    uint16_t            key;
    const char *        name;
};


/*
 *  Character map.
 *
 *      The character map allows customisation of ASCII character appearance.
 *
 *      This feature allows buffer characters within a window to be represented in
 *      in any number of way, e.g. hex, or control-characters, or EDT mode, etc.
 */

#define CMAP_CHARMAX    256

#define CMAP_SPECIALMIN CMAP_TABSTART
#define CMAP_SPECIALMAX CMAP_EOF
#define CMAP_SPECIALS   ((CMAP_SPECIALMAX - CMAP_SPECIALMIN) + 1)

enum _cmapspecials {
/*--export--enum--*/
/*
 *  Character map, special characters
 *      Note, mapped outside valid ASCII/UNICODE to avoid confusion.
 */
    CMAP_TABSTART       =0x7fffff70,
    CMAP_TABVIRTUAL     =0x7fffff71,
    CMAP_TABEND         =0x7fffff72,
    CMAP_EOL            =0x7fffff73,
    CMAP_EOF            =0x7fffff74,
/*--end--*/
};


struct _cmapchr {
    unsigned char       mc_class;
    unsigned char       mc_literal;
    unsigned char       mc_length;
    unsigned char       mc_width;
    const char *        mc_str;
    vbyte_t             mc_chr;
};


struct _cmap {
    MAGIC_t             cm_magic;
#define CMAP_MAGIC              MKMAGIC('C','m','A','p')
    TAILQ_ENTRY(_cmap)
                        cm_node;                /* List node */
    int                 cm_ident;               /* Assigned identifier */
    char                cm_name[32];            /* "normal" etc */
    struct _cmapchr     cm_local;
    char                cm_working[64];         /* Working buffer during conversions */
    uint32_t            cm_flags;
    uint32_t            cm_lower;
    uint32_t            cm_upper;
    uint32_t            cm_range;
    const char *        cm_buffer;
    const char *        cm_cursor;
    unsigned            cm_size;
    uint32_t            cm_magic2;
    struct _cmapchr     cm_specials[CMAP_SPECIALS];
    struct _cmapchr     cm_chars[CMAP_CHARMAX];
};


enum _cmaps {
/*--export--enum--*/
/*
 *  Special character classes/flags.
 */
    CMAP_DEFAULT        =0,
    CMAP_TAB            =1,
    CMAP_BACKSPACE      =2,
    CMAP_ESCAPE         =3
/*--end--*/
};


/*
 *  Window object.
 */
#if defined(_MSC_VER) || defined(__WATCOMC__) || defined(WIN32)
#undef WB_LEFT                                  /* Conflicts */
#undef WB_RIGHT
#endif

enum _wside {
    WB_TOP              =0x01,                  /* Top border present */
    WB_BOTTOM           =0x02,                  /* Bottom border present */
    WB_LEFT             =0x04,                  /* Left border present */
    WB_RIGHT            =0x08                   /* Right border present */
};


enum _wpopup {
    W_TILED             =0x01,
    W_POPUP             =0x02,
    W_MENU              =0x03
};


enum _wcorner {
    /*
     *  Definitions of the corners of a window for the w_corner_hints array
     */
    TL_CORNER           =0,
    TR_CORNER           =1,
    BL_CORNER           =2,
    BR_CORNER           =3,

    /*
     *  Definitions of the corner settings. This is used to indicate how the
     *  corners of a window intersect so we can change the corner points so we
     *  have a solid line. The syntax of the names correspond to clock hands
     */
    CORNER_12           =0x01,
    CORNER_3            =0x02,
    CORNER_6            =0x04,
    CORNER_9            =0x08
};


/*
 *  Window modify actions.
 *      Use the lowest priority to avoid unneeded display updates.
 */
enum _wstatus {
    WFHARD              =0x0001,                /* Full display */
    WFMARGIN            =0x0002,                /* Margin change */
    WFTOP               =0x0010,                /* Top window */
    WFPAGE              =0x0020,                /* Page moveement (lines > 1) */
    WFMOVE              =0x0040,                /* Movement from line to line */
    WFEDIT              =0x0100,                /* Editing within a line */
    WFDELL              =0x0200,                /* Line been deleted */
    WFINSL              =0x0400,                /* Line inserted */
};


enum _wflags {
/*--export--enum--*/
/*
 *  Window flags
 */
    WF_HIDDEN           =0x00000001,            /* Hide the window from view, used to hide nested popups/boss mode etc */
    WF_NO_SHADOW        =0x00000002,            /* Turnoff the popups shadow */
    WF_NO_BORDER        =0x00000004,            /* Turnoff borders, regardless of the borders() setting */
    WF_SYSTEM           =0x00000008,            /* Window is a system window (eg menu) */

    WF_SHOWANCHOR       =0x00000010,            /* Show anchor regardless of selection status */
    WF_SELECTED         =0x00000020,            /* Hilite the title regardless of selection status */
    WF_LAZYUPDATE       =0x00000040,            /* Delay any updates until next refresh() */

    WF_LINE_NUMBERS     =0x00000100,            /* Line numbers */
    WF_LINE_STATUS      =0x00000200,            /* Line status */
    WF_EOF_DISPLAY      =0x00000400,            /* Show <EOF> marker */
    WF_TILDE_DISPLAY    =0x00000800,            /* Show <~> marker as EOF marker */

    WF_HIMODIFIED       =0x00001000,            /* Hilite modified lines using the 'modified' attribute */
    WF_HIADDITIONAL     =0x00002000,            /* Hilite additional lines */
    WF_HICHANGES        =0x00004000,            /* Hilite in-line changes */

    WF_EOL_HILITE       =0x00010000,            /* Limit hilites to the EOL and not screen width */
    WF_EOL_CURSOR       =0x00020000,            /* Limit cursor to EOL */

/*--end--*/

    WF_DIALOG           =0x00100000,            /* Dialog resource */
    WF_NO_TITLE         =0x00200000,            /* Hide top title line */
    WF_NO_MESSAGE       =0x00400000             /* Hide bottom message line */
};


enum {
/*--export--enum--*/
/*
 *  Window control objects
 */
    WCTRLO_CLOSE_BTN,
    WCTRLO_ZOOM_BTN,
    WCTRLO_VERT_SCROLL,
    WCTRLO_HORZ_SCROLL,
    WCTRLO_VERT_THUMB,
    WCTRLO_HORZ_THUMB,

/*--end--*/
    WCTRLO_MAX,                                 /* limit for user access */

    /*internal*/
    WCTRLO_USER_VSCROLL,
    WCTRLO_USER_HSCROLL,
    WCTRLO_USER_VTHUMB,
    WCTRLO_USER_HTHUMB
};


enum {
/*--export--enum--*/
/*
 *  Window control states
 */
    WCTRLS_DISABLE,
    WCTRLS_ENABLE,
    WCTRLS_HIDE,
    WCTRLS_SHOW,
    WCTRLS_ZOOMED
/*--end--*/
};

#define WFRST(w,f)      ((w)->w_flags = (f))
#define WFSET(w,f)      ((w)->w_flags |= (f))
#define WFCLR(w,f)      ((w)->w_flags &= ~(f))
#define WFTST(w,f)      (((w)->w_flags & (f)) ? 1 : 0)

struct _wthumbs {
    uint16_t            t_value;                /* Thumb value */
    uint16_t            t_active;               /* *true* if active */
    uint16_t            t_curr;                 /* Current display position */
    uint16_t            t_prev;                 /* Previous display position */
};

typedef TAILQ_HEAD(WindowList, _window)
                        WINDOWLIST_t;           /* Window list */

struct _window {
    MAGIC_t             w_magic;                /* Structure magic */
#define WINDOW_MAGIC            MKMAGIC('W','n','d','O')
    TAILQ_ENTRY(_window)
                        w_node;                 /* List node */
    IDENTIFIER_t        w_num;                  /* Window number */
    BUFFER_t *          w_bufp;                 /* Attached buffer displayed in window */
    void *              w_dialogp;              /* Attached dialog */
    uint16_t            w_priority;             /* Window priority */
    uint16_t            w_type;                 /* Window type (W_TILED, W_MENU or W_POPUP */
    uint16_t            w_tab;                  /* Assoicated tab */
    uint32_t            w_flags;                /* Flags (see above _wflags) */
    uint32_t            w_status;               /* Update status (se _wstatus) */
    LINENO              w_top_line;             /* Top line in window (>= 1) */
    LINENO              w_left_offset;          /* Offset of first left (window indentation, >= 0) */
    LINENO              w_line;                 /* Current line no */
    LINENO              w_col;                  /* Current column number */
    LINENO              w_eol_col;              /* Column prior to WF_EOL_CURSOR rules, otherwise 0 */
    LINENO              w_old_line;             /* Previous line, used to keep cursor stationary duing page moves */
    LINENO              w_old_col;              /* Previous column position */
    LINENO              w_mined;                /* Lowest line modified */
    LINENO              w_maxed;                /* Highest line modified */
    vbyte_t             w_attr;                 /* Current normal attrbute (if borderless) */
    uint16_t            w_x, w_y;               /* Current window frame */
    uint16_t            w_h, w_w;
    uint8_t             w_corner_hints[4];      /* Hints about how the corners should intersect */
    const char *        w_title;                /* Title line */
    const char *        w_message;              /* Message line */
    const cmap_t *      w_cmap;                 /* Assigned character map, NULL == default */
    uint16_t            w_linked;               /* Id of window we are scroll linked to */

    uint32_t            w_ctrl_stack[WCTRLO_MAX];
    uint32_t            w_ctrl_state;           /* Window control value */
    int                 w_ctrl_vthumb;
    int                 w_ctrl_hthumb;

    uint32_t            w_disp_flags;           /* Display flags */
    int                 w_disp_lmargin;         /* Left hand margin */
    int                 w_disp_rmargin;         /* Right hand margin */
    LINENO              w_disp_line;            /* Current line number */
    LINENO              w_disp_column;          /* Next column to be displayed */
    LINENO              w_disp_vtbase;          /* Base vt columnt adjustment */
    LINENO              w_disp_indent;          /* Indentation cursor */
    const cmap_t *      w_disp_cmap;            /* Active character map, buffer, window or base */
    void *              w_disp_anchor;          /* Associated anchor */
    vbyte_t             w_disp_nattr;           /* Normal attribute */
    vbyte_t             w_disp_ansicolor;       /* ANSI color cursor */
    unsigned            w_disp_ansiflags;

    struct _wthumbs     w_vthumb;               /* Vertical elevator positions */
    struct _wthumbs     w_hthumb;               /* Horizontal elevator positions */

    MAGIC_t             w_magic2;               /* Structure magic */
};


/*
 *  Following information defines the format of characters passed to the low level
 *  ttputc() function (ie. vbyte_t construction).
 *
 *    ------------------------------------------------------------------------
 *    |   colorattr    |                    character                        |
 *    ------------------------------------------------------------------------
 *    31               23                                                    0
 *
 *  Unicode defines a codespace for 1,114,112 code points in range 0 to 10FFFF
 *  leaving the top bits for 2048 attribute values.
 */

#define VBYTE_ATTR_MASK         0xffe00000      /* 0..2047 */
#define VBYTE_ATTR_MAX          2047
#define VBYTE_ATTR_SHIFT        21
#define VBYTE_CHAR_MASK         0x001fffff      /* 0..10FFFF */

#define VBYTE_CHAR(__x)         (__x)
#define VBYTE_CHAR_GET(__x)     ((__x) & VBYTE_CHAR_MASK)

#define VBYTE_ATTR(__x)         ((__x) << VBYTE_ATTR_SHIFT)
#define VBYTE_ATTR_GET(__x)     (((__x) & VBYTE_ATTR_MASK) >> VBYTE_ATTR_SHIFT)

enum _colordef {
/*--export--enum--*/
/*
 *  Color values
 *
 *      0..15           PC-STYLE/BRIEF
 *      16...           Extensions
 */
    BLACK               =0,
    BLUE                =1,
    GREEN               =2,
    CYAN                =3,
    RED                 =4,
    MAGENTA             =5,
    BROWN               =6,
    WHITE               =7,

    GREY                =8,
    LTBLUE              =9,
    LTGREEN             =10,
    LTCYAN              =11,
    LTRED               =12,
    LTMAGENTA           =13,
    YELLOW              =14,
    LTWHITE             =15,

    DKGREY              =16,
    DKBLUE              =17,
    DKGREEN             =18,
    DKCYAN              =19,
    DKRED               =20,
    DKMAGENTA           =21,
    DKYELLOW            =22,
    LTYELLOW            =23,

    COLOR_NONE          =24,
/*--end--*/

    COLOUR_FOREGROUND  =101,
    COLOUR_BACKGROUND  =102,
    COLOUR_DYNAMIC_FOREGROUND =103,
    COLOUR_DYNAMIC_BACKGROUND =104,
};


/*--export--defines--*/
/*
 *  get_color flags
 *
 *      [<id>,] [<flags>,] <name> = specification
 *
 *      flags =
 *          0x01        foreground only
 *          0x02        background only
 *          0x04        foreground + background
 *
 *      specification =
 *          foreground [,background] [:style] [sticky]link@[none|<attribute>]
 */
#define COLORGET_FNAME  0x02
#define COLORGET_FVALUE 0x04
#define COLORGET_FFLAGS 0x08
#define COLORGET_NAMES  0x10
/*--end--*/


/*--export--*/
/*
 *  Basic color names.
 */
#define COLORS_ALL      \
     "black",           \
     "blue",            \
     "green",           \
     "cyan",            \
     "red",             \
     "magenta",         \
     "brown",           \
     "white",           \
     "grey",            \
     "light-blue",      \
     "light-green",     \
     "light-cyan",      \
     "light-red",       \
     "light-magenta",   \
     "yellow",          \
     "light-white",     \
     "dark-grey",       \
     "dark-blue",       \
     "dark-green",      \
     "dark-cyan",       \
     "dark-red",        \
     "dark-magenta",    \
     "dark-yellow",     \
     "light-yellow",    \
     "none"
/*--end--*/


/*--export--*/
/*
 *  List of colors when we are only allowed to specify foreground.
 */
#define COLORS_FG       \
    "black",            \
    "blue",             \
    "green",            \
    "cyan",             \
    "red",              \
    "magenta",          \
    "brown",            \
    "white"
/*--end--*/


enum {
/*--export--enum--*/
/*
 *  Color enumations/
 *
 *      get_color/set_color basic colors
 */
    COL_BACKGROUND,
    COL_FOREGROUND,
    COL_SELECTED_WINDOW,
    COL_MESSAGES,
    COL_ERRORS,
    COL_HILITE_BACKGROUND,
    COL_HILITE_FOREGROUND,
    COL_BORDERS,
    COL_INSERT_CURSOR,
    COL_OVERTYPE_CURSOR,
    COL_SHADOW,
    COL_PROMPT,
    COL_COMPLETION,
    COL_QUESTION,
    COL_ECHOLINE,
    COL_STANDOUT,
/*--end--*/
    COL_MAX
};


enum _dflags {
/*--export--enum--*/
/*
 *  Display control flags/
 *
 *      display_mode/inq_display_mode
 */
    DC_WINDOW           =0x00000001,            /* Running under a windowing system (read-only) */
    DC_MOUSE            =0x00000002,            /* Mouse enabled/available (read-only) */
    DC_READONLY         =0x00000004,            /* Read-only mode (read-only) */
    DC_CHARMODE         =0x00000008,            /* Character-mode with primitive GUI features available (read-only) */

    DC_SHADOW           =0x00000010,            /* Display shadow around popups */
    DC_SHADOW_SHOWTHRU  =0x00000020,            /* Show-thru shadow around popups */
    DC_STATUSLINE       =0x00000040,            /* Utilise window status-lines, when borderless */

    DC_UNICODE          =0x00000100,            /* UNICODE character encoding available (read-only) */
    DC_ASCIIONLY        =0x00000200,            /* Only utilise ASCII characters within character-sets/dialogs */

    DC_ROSUFFIX         =0x00001000,            /* Read-only suffix on titles */
    DC_MODSUFFIX        =0x00002000,            /* Modified suffix */

    DC_EOF_DISPLAY      =0x00010000,            /* Show <EOF> marker */
    DC_TILDE_DISPLAY    =0x00020000,            /* Show <~> marker */
    DC_EOL_HILITE       =0x00040000,            /* Limit hilites to EOL */
    DC_EOL_CURSOR       =0x00080000,            /* Limit cursor to EOL */
    DC_HIMODIFIED       =0x00100000,            /* Hilite modified lines */
    DC_HIADDITIONAL     =0x00200000,            /* Hilite additional lines */
/*--end--*/

    DC_CMAPFRAME        =0x01000000,            /* cmap frame characters */
};


#define DISPLAY_ESCAPELEN       64              /* Size of escape sequences */

struct _display {
    MAGIC_t             d_magic;                /* Structure magic */
#define DISPLAY_MAGIC           MKMAGIC('P','r','O','c')

    int                 d_ident;                /* Identifier */
    int                 d_cols, d_rows;         /* Physical size */
    char                d_escbuf[DISPLAY_ESCAPELEN];   /* Buffer to assemble escape sequence */
    char *              d_escptr;               /* Pointer to next byte in d_escape */
    int                 d_escmax;               /* Upper length/max for d_escptr */
    int                 d_esclen;               /* Escape buffer length */
    int                 d_esctyp;               /* Encoding type */
    char                d_attrbuf[DISPLAY_ESCAPELEN];
    int                 d_attrlen;              /* Attribute buffer length */
    uint32_t            d_flags;                /* control flags */
    uint32_t            d_wlen;                 /* Number of characters in waitfor buffer */
    char *              d_waitfor;              /* Queue of characters inserted into buffer from pty */
    int                 d_curline;              /* Current cursor */
    int                 d_curcol;
    int                 d_attrline;             /* Last attribute update */
    int                 d_attrcol;
    int                 d_lastchar;             /* last character written (for repeats) */
    pid_t               d_pid;                  /* Process ID of child */
    int                 d_ipc_type;             /* IPC type */
    const char *        d_ptsname;
    int                 d_pipe_in;              /* Pipe to read from */
    int                 d_pipe_out;             /* Pipe to write to */
    void              (*d_cleanup)(struct _display *);
    vbyte_t             d_color;                /* Color mask to be OR'ed in */
    vbyte_t             d_attr;                 /* Mask telling us whether its reverse/bold */

    /*os specfic*/
#if defined(WIN32)
    int                 d_handle_in;
    int                 d_handle_out;
#endif
#if defined(__OS2__)
    unsigned long       d_sema;                 /* Semaphore to wait on */
    unsigned long       d_wait;                 /* Semaphore for thread to wait on */
    unsigned            d_err;                  /* Error return */
    unsigned            d_rcvlen;               /* Number of bytes read */
    short               d_dead;                 /* Thread died */
#if defined(__FLAT__)
#define PTY_BUFSIZ              41024
#define PTY_STACK               4096
#else
#define PTY_BUFSIZ              512
#define PTY_STACK               512
    char                d_stack[PTY_STACK];     /* Stack for PTY thread */
#endif
    char                d_buf[PTY_BUFSIZ];      /* Buffer for reading into */
#endif
#if defined(unix) || defined(__APPLE__)
    pid_t               d_pgrp;                 /* Process group of child */
#endif
};


/*
 *  Text is kept in buffers. A buffer header, described below, exists for every buffer
 *  in the system. The buffers are kept in a big list, so that commands that search for
 *  a buffer by name can find the buffer header. The text for the buffer is kept in a
 *  circularly linked list of lines, with a pointer to the header line in "b_linep".
 */
struct _undo {
    FSIZE_t             u_chain;                /* Buffer undo chain */
    FSIZE_t             u_last;                 /* ftell() position of undo */
};


enum _bflags {
/*--export--enum--*/
/*
 *  Buffer flags ---
 *      Basic buffer characteristics.
 */
    BF_CHANGED          =0x00000001,            /* Changed */
    BF_BACKUP           =0x00000002,            /* Backup required on next write */
    BF_RDONLY           =0x00000004,            /* Read-only */
    BF_READ             =0x00000008,            /* Buffer content still to be read */
    BF_EXEC             =0x00000010,            /* File is executable */
    BF_PROCESS          =0x00000020,            /* Buffer has process attached */
    BF_BINARY           =0x00000040,            /* Binary buffer */
    BF_ANSI             =0x00000080,            /* If TRUE, ANSI-fication is done */
    BF_TABS             =0x00000100,            /* Buffer inserts real-tabs */
    BF_SYSBUF           =0x00000200,            /* Buffer is a system buffer */
    BF_LOCK             =0x00000400,            /* File lock */
    BF_NO_UNDO          =0x00000800,            /* Dont keep undo info */
    BF_NEW_FILE         =0x00001000,            /* File is a new file, so write even if no changes */
    BF_CR_MODE          =0x00002000,            /* Append <CR> to end of each line on output */
    BF_SYNTAX           =0x00004000,            /* Enable syntax highlighting (unless ANSI) */
    FB_STATUSLINE       =0x00008000,            /* Status line */
    BF_MAN              =0x00010000,            /* If TRUE, man style \b is done */
    BF_SPELL            =0x00020000,            /* Enable spell */
    BF_FOLDING          =0x00040000,            /* Test folding/hiding */
    BF_RULER            =0x00080000,            /* Display ruler */
    BF_VOLATILE         =0x00100000,            /* Buffer is volatile */
    BF_EOF_DISPLAY      =0x00200000,            /* Show <EOF> markers */
    BF_HIDDEN           =0x00400000,            /* Hidden buffer, from buffer list */
//  BF_AUTOREAD         =0x01000000,            /* Automaticly re-read buffer if underlying changes */
    BF_AUTOWRITE        =0x02000000,            /* Automaticly write buffer, if modified */
    BF_SCRAPBUF         =0x04000000,            /* Scrap buffer */
//  BF_DELAYED          =0x10000000,            /* Content load delayed until first reference */

    /*
     *  BF2_XXXX values ---
     *      UI formatting control.
     */
    BF2_ATTRIBUTES      =0x00000001,            /* Character attributes (ie. charcell level coloring) */
    BF2_DIALOG          =0x00000002,            /* Dialog */

    BF2_CURSOR_ROW      =0x00000010,            /* Display cursor crosshair */
    BF2_CURSOR_COL      =0x00000020,
    BF2_TILDE_DISPLAY   =0x00000040,
    BF2_EOL_HILITE      =0x00000080,            /* Limit hilites to EOL */

    BF2_LINE_NUMBERS    =0x00000100,            /* Line numbers */
    BF2_LINE_OLDNUMBERS =0x00000200,            /* If has line numbers, display old lines */
    BF2_LINE_STATUS     =0x00000400,            /* Markup modified lines. */
    BF2_LINE_SYNTAX     =0x00000800,            /* Syntax pre-processor flags */

    BF2_TITLE_FULL      =0x00001000,            /* Label window using full path name */
    BF2_TITLE_SCROLL    =0x00002000,            /* Scroll title with window */
    BF2_TITLE_LEFT      =0x00004000,            /* Left justify title */
    BF2_TITLE_RIGHT     =0x00008000,            /* Right justify title */

    BF2_SUFFIX_RO       =0x00010000,            /* read-only suffix on title */
    BF2_SUFFIX_MOD      =0x00020000,            /* modified suffix on title */
    BF2_EOL_CURSOR      =0x00040000,            /* Limit cursor to EOL */
    BF2_EOF_CURSOR      =0x00080000,            /* Limit cursor to EOF */

    BF2_HILITERAL       =0x00100000,            /* Hilite literal characters */
    BF2_HIWHITESPACE    =0x00200000,            /* Hilite whitespace */
    BF2_HIMODIFIED      =0x00400000,            /* Hilite modified lines */
    BF2_HIADDITIONAL    =0x00800000,            /* Hilite added lines */

    /*
     *  BF3_XXXX values ---
     *      Indirect buffer functionality, generally implemented at a macro level.
     */
    BF3_AUTOSAVE        =0x00000001,            /* Auto-save */
    BF3_AUTOINDENT      =0x00000002,            /* Auto-indent */
    BF3_AUTOWRAP        =0x00000004,            /* Auto-wrap */

    BF3_PASTE_MODE      =0x00000010,            /* Paste mode, disables a number of auto functions */

    /*
     *  BF4_xxxx values ----
     *      File input/out conversion
     */
    BF4_OCVT_TRIMWHITE  =0x00010000,            /* Output conversion, trim trailing whitespace */
/*--end--*/

  /*0x80000000  --- dont use as flags are exported as 'int32_t' */
};


/*
 *  Buffer support macros
 *
 *      buf_isutf8()    UTF8 encoded buffer.
 *      buf_ismchar()   Multibyte character encoded buffer.
 */
#define BFTYP_IS8BIT(__t)                       /* 8bit encoding */ \
    (BFTYP_UNIX  == (__t) || BFTYP_DOS == (__t) || BFTYP_MAC == (__t) || BFTYP_SBCS == (__t) ? 1 : 0)

#define buf_isutf8(_bp)     \
            (_bp && (BFTYP_UTF8 == _bp->b_type))

typedef int16_t BUFTYPE_t;

enum _btypes {
/*--export--enum--*/
/*
 *  Buffer types
 */
    BFTYP_UNKNOWN       =0x0000,

    BFTYP_UNIX          =0x0001,                /* LF */
    BFTYP_DOS           =0x0002,                /* CR/LF */
    BFTYP_MAC           =0x0003,                /* CR */
    BFTYP_BINARY        =0x0004,                /* <none> */
    BFTYP_ANSI          =0x0005,                /* ANSI */
    BFTYP_EBCDIC        =0x0006,                /* EBCDIC */

    BFTYP_UTF8          =0x0010,                /* UTF8 */
    BFTYP_UTF16         =0x0011,                /* UTF16/USC2 */
    BFTYP_UTF32         =0x0012,                /* UTF32/USC4 */
    BFTYP_UTFEBCDIC     =0x0015,                /* UTF8/EBCDIC (rare) */
    BFTYP_BOCU1         =0x0016,                /* Binary Ordered Compression for Unicode */
    BFTYP_SCSU          =0x0017,                /* Standard Compression Scheme for Unicode */
    BFTYP_UTF7          =0x0018,                /* 7-bit Unicode Transformation Format */

    BFTYP_GB            =0x0020,                /* GB */
    BFTYP_BIG5          =0x0021,                /* BIG5 */

    BFTYP_ISO2022       =0x0030,                /* ISO-2022 */

    BFTYP_SBCS          =0x0090,                /* Single Byte */
    BFTYP_DBCS          =0x0091,                /* Double Byte */
    BFTYP_MBCS          =0x0092,                /* Multi Byte (non-unicode) */

    BFTYP_OTHER         =0x0099,                /* Other supported */
    BFTYP_UNSUPPORTED   =0x00ff,                /* Known file-type, yet no internal support */
/*--end--*/

    BFTYP_UNDEFINED     =9999
};


enum _lterms {
/*--export--enum--*/
/*
 *  Line terminator types
 */
    LTERM_UNDEFINED     =0x00,                  /* <unknown/default> */
    LTERM_NONE          =0x01,                  /* <none> (i.e. binary) */
    LTERM_UNIX          =0x02,                  /* CR/LF */
    LTERM_DOS           =0x03,                  /* LF */
    LTERM_MAC           =0x04,                  /* CR */
    LTERM_NEL           =0x05,                  /* NEL */
    LTERM_UCSNL         =0x06,                  /* Unicode next line */
    LTERM_USER          =0xff                   /* User defined */
/*--end--*/
};


#if defined(__APPLE__) || defined(MAC_OSX)
#define BFTYP_DEFAULT   BFTYP_MAC
#elif defined(DOSISH)
#define BFTYP_DEFAULT   BFTYP_DOS
#else
#define BFTYP_DEFAULT   BFTYP_UNIX
#endif
#define LTERM_DEFAULT   LTERM_UNDEFINED

/*
 *  Buffer flags access
 */
#define BFRST(b,f)      ((b)->b_flag1  =  (uint32_t)(f))
#define BFSET(b,f)      ((b)->b_flag1  |= (uint32_t)(f))
#define BFCLR(b,f)      ((b)->b_flag1  &= (uint32_t)(~(f)))
#define BFTST(b,f)      (((b)->b_flag1 &  (uint32_t)(f)) ? 1 : 0)

#define BF2RST(b,f)     ((b)->b_flag2  =  (uint32_t)(f))
#define BF2SET(b,f)     ((b)->b_flag2  |= (uint32_t)(f))
#define BF2CLR(b,f)     ((b)->b_flag2  &= (uint32_t)(~(f)))
#define BF2TST(b,f)     (((b)->b_flag2 &  (uint32_t)(f)) ? 1 : 0)

#define BF3RST(b,f)     ((b)->b_flag3  =  (uint32_t)(f))
#define BF3SET(b,f)     ((b)->b_flag3  |= (uint32_t)(f))
#define BF3CLR(b,f)     ((b)->b_flag3  &= (uint32_t)(~(f)))
#define BF3TST(b,f)     (((b)->b_flag3 &  (uint32_t)(f)) ? 1 : 0)

#define BF4RST(b,f)     ((b)->b_flag4  =  (uint32_t)(f))
#define BF4SET(b,f)     ((b)->b_flag4  |= (uint32_t)(f))
#define BF4CLR(b,f)     ((b)->b_flag4  &= (uint32_t)(~(f)))
#define BF4TST(b,f)     (((b)->b_flag4 &  (uint32_t)(f)) ? 1 : 0)

/*
 *  Text line objects are managed in a linked lists of "LINE" structures.
 *
 *  Each line object contains the number of bytes 'used' within the line buffer,
 *  the 'size' of the underlying line buffer 'size', plus flags and optional
 *  attributes buffer.
 *
 *  Unless binary, the end of line is not stored; it is implied.
 */
typedef uint32_t lineflags_t;

enum _liflags {
    LI_INFILE           =0x01,                  /* Line is within a file chunk (replace with with !L_INCORE) */
    LI_INCORE           =0x02,                  /* Line is in-memory. */
    LI_LOCKED           =0x04,                  /* Line has been locked */
    LI_MODIFIED         =0x08,                  /* Line has been modified, since last save. */
    LI_ATTRIBUTES       =0x10,                  /* Line has attributes. */
    LI_DIRTY            =0x20,                  /* On screen line image is dirty, result of lazyvt. */
    LI_MBSWIDE          =0x40,                  /* Line contains multibyte/wide characters. */
};

enum _luflags {
#define L_SYSRO_MASK     0x0000ffff             /* read-only mask */

    L_BREAK             =0x00000001,            /* Line was broken, no implied new-of-line. */

#define L_DIFF_MASK      0X000000f0

    L_DIFF_WHITESPACE   =0x00000010,            /* White space change only */
    L_DIFF_MODIFIED     =0x00000020,            /* Modified */
    L_DIFF_NEW_LINE     =0x00000030,            /* New line */
    L_DIFF_DEL_LINE     =0x00000040,            /* Deleted line */

#define L_SYNTAX_MASK    0x00000f00
#define L_SYNTAX_IN      0x00000700

    L_IN_COMMENT        =0x00000100,            /* Line within a comment */
    L_IN_COMMENT2       =0x00000200,            /* Line within a comment, style 2 */
    L_IN_STRING         =0x00000300,            /* Line within a string */
    L_IN_LITERAL        =0x00000400,            /* Line within a literal */
    L_IN_CHARACTER      =0x00000500,            /* Line within a character */
    L_IN_CONTINUE       =0x00000600,            /* Line within a continuation (eg pragma) */
    L_IN_PREPROC        =0x00000700,            /* Line within a continuation of an preprocessor statement */

    L_HAS_EOL_COMMENT   =0x00000800,            /* EOL contained within a comment */


    /*
     *  XXX - reserved for future use
     */
#define L_TERM_MASK      0x00007000

    L_TERM_NEL          =0x00001000,            /* Next-line */
    L_TERM_LINE         =0x00002000,            /* Unicode Line terminator */
    L_TERM_PARAGRAPH    =0x00003000,            /* Unicode Paragraph terminator */
    L_SPELL             =0x00008000,            /* Spelling errors detected */

/*--export--enum--*/
/*
 *  User line status flags/
 *      note, exclude hi-bit as macrp types are signed.
 */
#define L_USER_MASK      0xfff00000

    L_MARKED            =0x00100008,
    L_USER1             =0x00200000,
    L_USER2             =0x00400000,
    L_USER3             =0x00800000,
    L_USER4             =0x01000000,
    L_USER5             =0x02000000,
    L_USER6             =0x04000000,
    L_USER7             =0x08000000,
    L_USER8             =0x10000000,
    L_USER9             =0x20000000,
    L_USER10            =0x40000000
/*--end--*/
};


/*
 *  Buffer lines
 */
typedef TAILQ_HEAD(LineList, _line)
                        LINELIST_t;             /* Line list */

struct _line {
    TAILQ_ENTRY(_line)  l_node;                 /* Line list node */
    uint32_t            l_size   : 24;          /* Allocated length, in bytes (0 -- 16,777,215) */
    uint32_t            l_iflags : 8;           /* Internal flags, LI_INFILE etc */
    uint32_t            l_used   : 24;          /* Assigned length, in bytes */
    uint32_t            l_depth  : 8;           /* Folding/nesting depth (0 - 255) */
    LINENO              l_oldlineno;            /* Old line number */
    lineflags_t         l_uflags;               /* User flags, See above */
    void *              l_chunk;                /* assigned file chunk */
    union {
        LINECHAR *      text;                   /* Line buffer/address within chunk */
        FSIZE_t         tell;                   /* Offset in file when swapped out. */
    } u;
#define l_tell          u.tell
#define l_text          u.text
    LINEATTR *          l_attr;                 /* Attributes (optional) */
};

#define ltext(__lp)     ((__lp)->l_text)
#define lattr(__lp)     ((__lp)->l_attr)
#define llength(__lp)   ((__lp)->l_used)
#define ldepth(__lp)    ((__lp)->l_depth)

/* internal line flags */
#define liflags(__lp)           ((__lp)->l_iflags)
#define liflagclr(__lp,__f)     ( (__lp)->l_iflags &= (uint8_t)(~(__f)))
#define liflagset(__lp,__f)     ( (__lp)->l_iflags |= (uint8_t)(__f))
#define liflagtst(__lp,__f)     (((__lp)->l_iflags &  (__f)) ? 1 : 0)

#define linfile(__lp)           (LI_INFILE & liflags(__lp))
#define lincore(__lp)           (LI_INCORE & liflags(__lp))
#define lisdirty(__lp)          (LI_DIRTY  & liflags(__lp))
#define lismodified(__lp)       (LI_MODIFIED & liflags(__lp))

/* user line flags */
#define lflags(__lp)            ((__lp)->l_uflags)
#define lflagset(__lp,__f)      ( (__lp)->l_uflags |= (lineflags_t)(__f))
#define lflagclr(__lp,__f)      ( (__lp)->l_uflags &= (lineflags_t)(~(__f)))
#define lflagtst(__lp,__f)      (((__lp)->l_uflags & (__f)) ? 1 : 0)

#define lhead(__bp)     TAILQ_FIRST(&__bp->b_lineq)
#define ltail(__bp)     TAILQ_LAST(&__bp->b_lineq, LineList)
#define lforw(__lp)     TAILQ_NEXT(__lp, l_node)
#define lback(__lp)     TAILQ_PREV(__lp, LineList, l_node)


/*
 *  buffer file chunk
 */
typedef TAILQ_HEAD(ChunkList, _bufferchunk)
                        BUFFERCHUNKLIST_t;      /* Chunk list */

typedef struct _bufferchunk {
    MAGIC_t             c_magic;                /* Structure wmagic */
#define BUFFERCHUNK_MAGIC       MKMAGIC('B','u','f','C')

    uint32_t            c_ident;                /* Identifier */
    uint32_t            c_size;                 /* Size of the chunk, in bytes */
    TAILQ_ENTRY(_bufferchunk)
                        c_node;                 /* Chunk list node */
    uint32_t            c_refers;               /* Reference count */
    void *              c_vmaddr;               /* Virtual memory address */
    size_t              c_vmsize;               /* Virtual memory size, in bytes */
    void *              c_buffer;               /* Associated buffer */
    MAGIC_t             c_magic2;               /* Structure wmagic */
} BUFFERCHUNK_t;


/*
 *  buffer object
 */
typedef TAILQ_HEAD(_BufferList, _buffer)
                        BUFFERLIST_t;           /* Buffer list */

typedef TAILQ_HEAD(_AnchorList, _anchor)
                        ANCHORLIST_t;           /* ANCHOR list */

typedef TAILQ_HEAD(_HiliteList, _hilite)
                        HILITELIST_t;           /* HILITE list */

typedef TAILQ_HEAD(_RegisterList, _registration)
                        REGISTERLIST_t;         /* REGISTER list */

struct _buffer {
    MAGIC_t             b_magic;                /* Structure magic */
#define BUFFER_MAGIC            MKMAGIC('B','u','f','O')

    TAILQ_ENTRY(_buffer)
                        b_node;                 /* List node */
    IDENTIFIER_t        b_bufnum;               /* Buffer number */
    unsigned            b_refs;                 /* Open/create count */
    unsigned            b_lstat;                /* Lock status */
    LINENO              b_line;                 /* Current line number */
    LINENO              b_col;                  /* Current column number */
#define CURSOR_HUGE_COL         0x00ffffff

    LINENO              b_numlines;             /* Number of lines within the buffer */
    LINENO              b_top;                  /* Top line on screen */

    LINENO              b_vline;                /* Visual cursor position */
    LINENO              b_vcol;                 /* Visual cursor column number */
    int                 b_vstatus;              /* Character status under cursor */
#define BUFFERVSTATUS_NORMAL    0
#define BUFFERVSTATUS_MULTIBYTE 0x0001
#define BUFFERVSTATUS_ILLEGAL   0x0002
#define BUFFERVSTATUS_VIRTUAL   0x0004
#define BUFFERVSTATUS_EOL       0x0010          /* <EOL> */
#define BUFFERVSTATUS_XEOL      0x0020          /* <EOL>, yet due to EOL_CURSOR settings */
#define BUFFERVSTATUS_PEOL      0x0040          /* Past <EOL> */
#define BUFFERVSTATUS_EOF       0x0100          /* <EOF> */

    int                 b_vcombined;            /* Number of combined characters */
    int                 b_vwidth;               /* Character width (generally 1 or 2) */
    int                 b_vlength;              /* Length of character, in bytes */
    int32_t             b_vchar[4];             /* Character value under cursor + combined values */

    LINELIST_t          b_lineq;                /* Line queus/list */
    LINENO              b_cline;                /* Cached line number */
    LINE_t *            b_clinep;               /* Pointer to currently cached line */
    LINENO              b_maxlength;            /* Length of longest line */
    LINE_t *            b_maxlinep;             /* and its line pointer */
    int                 b_nwnd;                 /* Count of windows on buffer */
    int                 b_imode;                /* Buffer specific insert-mode (-1 == system) */
    uint32_t            b_flag1;                /* BF_XXX values */
    uint32_t            b_flag2;                /* BF2_XXX values */
    uint32_t            b_flag3;                /* BF3_XXX values */
    uint32_t            b_flag4;                /* BF4_XXX values */
    uint32_t            b_flagu;                /* User buffer flags */
    const char *        b_fname;                /* File name */
    const char *        b_title;                /* Title from create_buffer */
    const char *        b_encoding;             /* Buffer type encoding */
    BUFTYPE_t           b_type;                 /* BTYP_xxxx */
    int16_t             b_endian;               /* Endian UTF32 and UTF16 only */
    uint32_t            b_bin_chunk_size;       /* Chunk size (at load time) */
    void *              b_iconv;                /* Buffer iconv/character-map */
    uint8_t             b_bombuf[4];            /* Unicode Byte-Order-Marker (if any) */
    unsigned            b_bomlen;               /* Byte-Order-Marker length */
    unsigned            b_termtype;             /* Line terminator type */
    unsigned            b_termlen;              /* Line terminator length */
    unsigned char       b_termbuf[8];           /* Line terminator definition */
    UNDO_t              b_undo;                 /* UNDO information */
    UNDO_t              b_redo;                 /* REDO information */
    uint32_t            b_nummod;               /* No. of modifications to buffer; zero denotes unmodified */
    int32_t             b_uchar;                /* Last character selfinserted to do undo collapsing */
    mode_t              b_mode;                 /* Mode for chmod() */
    uid_t               b_uid;                  /* File owner */
    gid_t               b_gid;                  /* File group */

    int                 b_bkversions;           /* Backup versions */
    const char *        b_bkdir;                /* Backup directory */
    const char *        b_bksuffix;             /* Backup suffix */
    const char *        b_bkprefix;             /* Backup prefix */
    FSIZE_t             b_bkask;                /* Backup ask limit */
    FSIZE_t             b_bkdont;               /* Backup dont limit */

    time_t              b_mtime;                /* Time when buffer modified when we read it in. */
    time_t              b_ctime;                /* Last change/modication time */
    size_t              b_rsize;                /* Size at time of last read */

    int                 b_marginl;              /* Left margin */
    int                 b_marginr;              /* Right margin */
    int                 b_margins;              /* Style */
    LINENO              b_indent;               /* Indenting amount (as distinct from tabs) */
    LINENO              b_colorcolumn;          /* Right color column */
#define BUFFER_NTABS            16              /* Max. number of tab stops */
    LINENO              b_tabs[BUFFER_NTABS+1]; /* Tab columns (XXX - dynamic like b_ruler) */
    const LINENO *      b_ruler;                /* Indenting ruler */
    SPTREE *            b_syms;                 /* Local symbols */
    ANCHORLIST_t        b_anchors;              /* Anchor list */
    const struct _anchor *b_anchor;             /* Current anchor */
    HILITELIST_t        b_hilites;              /* Associated hilites */
    const struct _hilite *b_hilite;             /* Current hilite/cursor */
    DISPLAY_t *         b_display;              /* Pointer to screen buffer if this buffer is attached to a pty */
    int                 b_wstat;                /* Exit status of child process */
    struct _keyboard *  b_keyboard;             /* Local keyboard */
    REGISTERLIST_t      b_register;             /* Registered macros */
    const cmap_t *      b_cmap;                 /* Character map */
    struct SyntaxTable *b_syntax;               /* Syntax table */
    LINENO              b_syntax_min;           /* Syntax lower rescan region */
    LINENO              b_syntax_max;           /* Syntax upper rescan region */
    BUFFERCHUNKLIST_t   b_chunk_list;           /* Chunk list, memory managment */
    uint32_t            b_chunk_ident;          /* Plus allocation identifier */
    LINEATTR            b_attrcurrent;          /* Current attribute */
    LINEATTR            b_attrnormal;           /* Clear attribute */
    LINEATTR *          b_colorizer;            /* Color map */
    MAGIC_t             b_magic2;               /* Structure magic */
};


enum {
/*--export--enum--*/
/*
 *  Definitions for the edit_file primitive
 */
    EDIT_NORMAL         =0x0000000,             /* Guess file type */
    EDIT_BINARY         =0x0000001,             /* Force file to be read in binary mode */
    EDIT_ASCII          =0x0000002,             /* Force file to be read in ascii mode */
    EDIT_STRIP_CR       =0x0000004,             /* Force CR removal on input and write them on output */
    EDIT_STRIP_CTRLZ    =0x0000008,             /* Force Ctrl-Z removal */
    EDIT_MASK           =0x0000008,             /* Mask out internals */

    EDIT_SYSTEM         =0x0000010,             /* System buffer */
    EDIT_RC             =0x0000020,             /* Extended return codes */
    EDIT_QUICK          =0x0000040,             /* Quick detection, removes more costly character-encoding methods */
    EDIT_AGAIN          =0x0000080,             /* Edit_again() implementation */

    EDIT_LOCAL          =0x0000100,             /* Edit locally, do not rely on the disk being stable */
    EDIT_READONLY       =0x0000200,             /* Set as read-only */
    EDIT_LOCK           =0x0000400,             /* Kock on read (strict-locking) */
    EDIT_NOLOCK         =0x0000800,             /* Disable auto-locking */

    EDIT_PREVIEW        =0x0001000,             /* Limit the load to the window size */

/*--end--*/
    EDIT_STARTUP        =0x0100000              /* Internal startup flag */
};


enum {
/*--export--enum--*/
/*
 *  Definitions for the write_file primitive
 */
    WRITE_APPEND        =0x0001,                /* Append, otherwise overwrite */
    WRITE_NOTRIGGER     =0x0010,                /* Do not generate buffer triggers */
    WRITE_NOREGION      =0x0020,                /* Ignore any selected region */
    WRITE_FORCE         =0x0040,                /* Force write, even if no change */
    WRITE_BACKUP        =0x0080,                /* Generate a backup image regardless whether already performed for this edit session */
/*--end--*/

    WRITE_APPEND_CR     =0x0100,                /* Append a carriage-return to the end of each line */
    WRITE_APPEND_NL     =0x0200,                /* Append a newline to the end of each line */
    WRITE_CTRLZ         =0x0400,                /* Append Ctrl-Z at the end of the file buffer */
};


enum {
/*--export--enum--*/
/*
 *  Definitions for the set/inq_backup_option primitive
 */
    BK_MODE             =1,
    BK_AUTOSAVE         =2,
    BK_DIR              =3,
    BK_DIRMODE          =4,
    BK_VERSIONS         =5,
    BK_PREFIX           =6,
    BK_SUFFIX           =7,
    BK_ONESUFFIX        =8,
    BK_ASK              =9,
    BK_DONT             =10
/*--end--*/
};


/*
 *  Characters used to symbolically represent special characters,
 *  for example line/border and scroll-bars.
 */
enum {
    /*
     *  Unicode reserves sixty-six code points as noncharacters. These code points are
     *  guaranteed to never have a character assigned to them.
     *
     *      As such use the range U+FDD0..U+FDEF
     *      Also available        U+E000..U+F8FF        Private Use Area
     */
    CH_VSCROLL          =0xFDD0,
    CH_VTHUMB,

    CH_HSCROLL,
    CH_HTHUMB,

    CH_VERTICAL,
    CH_HORIZONTAL,
    CH_TOP_LEFT,
    CH_TOP_RIGHT,
    CH_TOP_JOIN,
    CH_BOT_LEFT,
    CH_BOT_RIGHT,
    CH_BOT_JOIN,
    CH_LEFT_JOIN,
    CH_RIGHT_JOIN,
    CH_CROSS,

    CH_TOP_LEFT2,       /* dialog usage */
    CH_TOP_RIGHT2,
    CH_BOT_LEFT2,
    CH_BOT_RIGHT2,
    CH_RADIO_OFF,
    CH_RADIO_ON,
    CH_CHECK_OFF,
    CH_CHECK_ON,
    CH_CHECK_TRI,

    CH_PADDING,         /* 0xFDDF - wide character, padding */

    CH_MIN              =CH_VSCROLL,
    CH_MAX              =CH_PADDING
};


extern const unsigned   sizeof_atoms[];
extern const char *     nameof_atoms[];

extern const int        x_major_version;
extern const int        x_minor_version;
extern const int        x_edit_version;

extern const char *     x_appname;
extern const char *     x_version;
extern const char *     x_copyright;
extern const char *     x_compiled;

__CEND_DECLS

#endif /*GR_EDSTRUCT_H_INCLUDED*/
